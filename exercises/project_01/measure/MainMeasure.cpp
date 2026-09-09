#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <sstream>

// ============================================================
// CLASS: CameraCalibration
// Loads the intrinsic camera matrix and distortion coefficients.
// ============================================================

class CameraCalibration
{
private:
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

public:

    bool loadCalibration(const std::string& filename)
    {
        cv::FileStorage fs(filename, cv::FileStorage::READ);

        if (!fs.isOpened())
        {
            std::cerr << "ERROR: Cannot open calibration file: "
                << filename << std::endl;
            return false;
        }

        fs["camera_matrix"] >> cameraMatrix;
        fs["dist_coeffs"] >> distCoeffs;

        fs.release();

        if (cameraMatrix.empty())
        {
            std::cerr << "ERROR: Camera matrix was not found."
                << std::endl;
            return false;
        }

        if (distCoeffs.empty())
        {
            std::cerr << "ERROR: Distortion coefficients were not found."
                << std::endl;
            return false;
        }

        cameraMatrix.convertTo(cameraMatrix, CV_64F);
        distCoeffs.convertTo(distCoeffs, CV_64F);

        std::cout << "\nCamera calibration loaded successfully.\n";

        std::cout << "\nCamera matrix K:\n"
            << cameraMatrix << std::endl;

        std::cout << "\nDistortion coefficients:\n"
            << distCoeffs << std::endl;

        return true;
    }

    cv::Mat undistortImage(const cv::Mat& image) const
    {
        cv::Mat corrected;

        cv::undistort(
            image,
            corrected,
            cameraMatrix,
            distCoeffs,
            cameraMatrix
        );

        return corrected;
    }

    const cv::Mat& getCameraMatrix() const
    {
        return cameraMatrix;
    }

    const cv::Mat& getDistCoeffs() const
    {
        return distCoeffs;
    }
};


// ============================================================
// CLASS: MeasurementPlane
//
// Determines the transformation between the checkerboard plane
// and the camera.
//
// World coordinate system:
//
//           Y
//           |
//           |
//           +------ X
//
// Checkerboard plane: Z = 0
//
// ============================================================

class MeasurementPlane
{
private:
    cv::Mat rvec;
    cv::Mat tvec;

    cv::Mat rotation;
    cv::Mat homography;
    cv::Mat inverseHomography;

    bool calibrated = false;

public:

    bool calibrateFromChessboard(
        const cv::Mat& image,
        const cv::Mat& cameraMatrix,
        const cv::Mat& distCoeffs,
        const cv::Size& boardSize,
        double squareSize)
    {
        cv::Mat gray;

        if (image.channels() == 3)
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        else
            gray = image.clone();

        std::vector<cv::Point2f> imagePoints;

        // --------------------------------------------------------
        // Detect checkerboard internal corners
        // --------------------------------------------------------

        bool found = cv::findChessboardCorners(
            gray,
            boardSize,
            imagePoints,
            cv::CALIB_CB_ADAPTIVE_THRESH |
            cv::CALIB_CB_NORMALIZE_IMAGE
        );

        if (!found)
        {
            std::cerr << "\nERROR: Checkerboard was not detected.\n";
            return false;
        }

        // --------------------------------------------------------
        // Refine corner coordinates to subpixel accuracy
        // --------------------------------------------------------

        cv::cornerSubPix(
            gray,
            imagePoints,
            cv::Size(11, 11),
            cv::Size(-1, -1),
            cv::TermCriteria(
                cv::TermCriteria::EPS +
                cv::TermCriteria::COUNT,
                30,
                0.001
            )
        );

        // --------------------------------------------------------
        // Generate real checkerboard coordinates
        //
        // P = (X,Y,0)
        //
        // squareSize determines the measurement unit.
        // If squareSize = 25 mm, all resulting measurements
        // will be in millimeters.
        // --------------------------------------------------------

        std::vector<cv::Point3f> worldPoints;

        for (int row = 0; row < boardSize.height; row++)
        {
            for (int col = 0; col < boardSize.width; col++)
            {
                worldPoints.push_back(
                    cv::Point3f(
                        static_cast<float>(col * squareSize),
                        static_cast<float>(row * squareSize),
                        0.0f
                    )
                );
            }
        }

        // --------------------------------------------------------
        // Calculate checkerboard/measurement-plane pose
        // --------------------------------------------------------

        bool success = cv::solvePnP(
            worldPoints,
            imagePoints,
            cameraMatrix,
            distCoeffs,
            rvec,
            tvec,
            false,
            cv::SOLVEPNP_ITERATIVE
        );

        if (!success)
        {
            std::cerr << "ERROR: solvePnP failed.\n";
            return false;
        }

        // Convert rotation vector to rotation matrix.
        cv::Rodrigues(rvec, rotation);

        createHomography(cameraMatrix);

        calibrated = true;

        std::cout << "\nPlane calibration successful.\n";

        std::cout << "\nRotation vector:\n"
            << rvec << std::endl;

        std::cout << "\nTranslation vector:\n"
            << tvec << std::endl;

        std::cout << "\nRotation matrix:\n"
            << rotation << std::endl;

        std::cout << "\nPlane homography:\n"
            << homography << std::endl;

        // --------------------------------------------------------
        // Display detected checkerboard
        // --------------------------------------------------------

        cv::Mat preview = image.clone();

        cv::drawChessboardCorners(
            preview,
            boardSize,
            imagePoints,
            found
        );

        cv::putText(
            preview,
            "Measurement plane detected",
            cv::Point(20, 35),
            cv::FONT_HERSHEY_SIMPLEX,
            0.8,
            cv::Scalar(0, 255, 0),
            2
        );

        cv::imshow("Plane Calibration", preview);

        return true;
    }


    // ============================================================
    // Construct homography
    //
    // H = K [r1 r2 t]
    //
    // because Z = 0 on the measurement plane.
    // ============================================================

    void createHomography(const cv::Mat& cameraMatrix)
    {
        cv::Rodrigues(rvec, rotation);

        cv::Mat planeTransformation(3, 3, CV_64F);

        // First column of rotation matrix = r1
        rotation.col(0).copyTo(
            planeTransformation.col(0)
        );

        // Second column = r2
        rotation.col(1).copyTo(
            planeTransformation.col(1)
        );

        // Third column = translation t
        tvec.copyTo(
            planeTransformation.col(2)
        );

        homography =
            cameraMatrix * planeTransformation;

        inverseHomography =
            homography.inv();
    }


    // ============================================================
    // Convert image pixel coordinates into real-world coordinates
    // on the measurement plane.
    // ============================================================

    cv::Point2d imageToWorld(
        const cv::Point2f& imagePoint) const
    {
        cv::Mat pixel =
            (cv::Mat_<double>(3, 1)
                << imagePoint.x,
                imagePoint.y,
                1.0);

        cv::Mat world =
            inverseHomography * pixel;

        double w =
            world.at<double>(2, 0);

        if (std::abs(w) < 1e-12)
        {
            throw std::runtime_error(
                "Invalid homogeneous coordinate."
            );
        }

        double X =
            world.at<double>(0, 0) / w;

        double Y =
            world.at<double>(1, 0) / w;

        return cv::Point2d(X, Y);
    }


    // ============================================================
    // Save plane pose
    // ============================================================

    bool savePose(const std::string& filename) const
    {
        if (!calibrated)
            return false;

        cv::FileStorage fs(
            filename,
            cv::FileStorage::WRITE
        );

        if (!fs.isOpened())
        {
            std::cerr
                << "ERROR: Cannot create plane pose file.\n";

            return false;
        }

        fs << "rvec" << rvec;
        fs << "tvec" << tvec;

        fs.release();

        std::cout
            << "\nMeasurement-plane calibration saved to:\n"
            << filename << std::endl;

        return true;
    }


    // ============================================================
    // Load previously calculated plane pose
    // ============================================================

    bool loadPose(
        const std::string& filename,
        const cv::Mat& cameraMatrix)
    {
        cv::FileStorage fs(
            filename,
            cv::FileStorage::READ
        );

        if (!fs.isOpened())
        {
            return false;
        }

        fs["rvec"] >> rvec;
        fs["tvec"] >> tvec;

        fs.release();

        if (rvec.empty() || tvec.empty())
        {
            return false;
        }

        rvec.convertTo(rvec, CV_64F);
        tvec.convertTo(tvec, CV_64F);

        createHomography(cameraMatrix);

        calibrated = true;

        std::cout
            << "\nMeasurement-plane calibration loaded.\n";

        return true;
    }


    bool isCalibrated() const
    {
        return calibrated;
    }
};


// ============================================================
// CLASS: PointSelector
//
// Handles mouse clicks.
// ============================================================

class PointSelector
{
private:
    std::vector<cv::Point2f> points;

public:

    static void mouseCallback(
        int event,
        int x,
        int y,
        int flags,
        void* userdata)
    {
        PointSelector* selector =
            static_cast<PointSelector*>(userdata);

        if (selector == nullptr)
            return;

        // Left button: select a point.
        if (event == cv::EVENT_LBUTTONDOWN)
        {
            if (selector->points.size() < 2)
            {
                selector->points.push_back(
                    cv::Point2f(
                        static_cast<float>(x),
                        static_cast<float>(y)
                    )
                );

                std::cout
                    << "Selected image point "
                    << selector->points.size()
                    << ": ("
                    << x << ", "
                    << y << ")\n";
            }
        }

        // Right button: reset.
        if (event == cv::EVENT_RBUTTONDOWN)
        {
            selector->resetPoints();

            std::cout
                << "Selected points were reset.\n";
        }
    }


    void resetPoints()
    {
        points.clear();
    }


    int numberOfPoints() const
    {
        return static_cast<int>(
            points.size()
            );
    }


    cv::Point2f getPoint(int index) const
    {
        return points.at(index);
    }


    void drawPoints(cv::Mat& image) const
    {
        if (points.size() >= 1)
        {
            cv::circle(
                image,
                points[0],
                6,
                cv::Scalar(0, 0, 255),
                cv::FILLED
            );

            cv::putText(
                image,
                "P1",
                points[0] + cv::Point2f(10, -10),
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                cv::Scalar(0, 0, 255),
                2
            );
        }

        if (points.size() >= 2)
        {
            cv::circle(
                image,
                points[1],
                6,
                cv::Scalar(0, 255, 0),
                cv::FILLED
            );

            cv::putText(
                image,
                "P2",
                points[1] + cv::Point2f(10, -10),
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                cv::Scalar(0, 255, 0),
                2
            );

            cv::line(
                image,
                points[0],
                points[1],
                cv::Scalar(255, 0, 0),
                2
            );
        }
    }
};


// ============================================================
// CLASS: ObjectMeasurement
// ============================================================

class ObjectMeasurement
{
public:

    static double calculateDistance(
        const cv::Point2d& p1,
        const cv::Point2d& p2)
    {
        double dx =
            p2.x - p1.x;

        double dy =
            p2.y - p1.y;

        return std::sqrt(
            dx * dx +
            dy * dy
        );
    }


    static cv::Point calculateMidpoint(
        const cv::Point2f& p1,
        const cv::Point2f& p2)
    {
        return cv::Point(
            static_cast<int>((p1.x + p2.x) / 2.0),
            static_cast<int>((p1.y + p2.y) / 2.0)
        );
    }


    static std::string distanceText(
        double distance,
        const std::string& unit)
    {
        std::ostringstream stream;

        stream
            << std::fixed
            << std::setprecision(2)
            << distance
            << " "
            << unit;

        return stream.str();
    }
};


// ============================================================
// FUNCTION: measurementMode
//
// Allows user to select two points on a static undistorted image.
// ============================================================

void measurementMode(
    const cv::Mat& image,
    const MeasurementPlane& plane,
    const std::string& unit)
{
    const std::string windowName =
        "Object Measurement";

    PointSelector selector;

    cv::namedWindow(
        windowName,
        cv::WINDOW_NORMAL
    );

    cv::setMouseCallback(
        windowName,
        PointSelector::mouseCallback,
        &selector
    );

    std::cout
        << "\n=======================================\n"
        << "MEASUREMENT MODE\n"
        << "=======================================\n"
        << "Left mouse button : select P1 and P2\n"
        << "Right mouse button: reset points\n"
        << "R                 : reset points\n"
        << "ESC               : return to camera\n"
        << "=======================================\n";


    while (true)
    {
        cv::Mat display =
            image.clone();

        selector.drawPoints(display);

        // --------------------------------------------------------
        // Calculate distance when two points are available.
        // --------------------------------------------------------

        if (selector.numberOfPoints() == 2)
        {
            cv::Point2f imageP1 =
                selector.getPoint(0);

            cv::Point2f imageP2 =
                selector.getPoint(1);

            try
            {
                cv::Point2d worldP1 =
                    plane.imageToWorld(imageP1);

                cv::Point2d worldP2 =
                    plane.imageToWorld(imageP2);

                double distance =
                    ObjectMeasurement::calculateDistance(
                        worldP1,
                        worldP2
                    );

                std::string text =
                    ObjectMeasurement::distanceText(
                        distance,
                        unit
                    );

                cv::Point midpoint =
                    ObjectMeasurement::calculateMidpoint(
                        imageP1,
                        imageP2
                    );

                // Background for distance text.
                cv::rectangle(
                    display,
                    cv::Point(midpoint.x - 10,
                        midpoint.y - 35),
                    cv::Point(midpoint.x + 180,
                        midpoint.y + 5),
                    cv::Scalar(0, 0, 0),
                    cv::FILLED
                );

                cv::putText(
                    display,
                    text,
                    midpoint,
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.8,
                    cv::Scalar(255, 255, 255),
                    2
                );


                // World coordinates.
                std::ostringstream coordinates;

                coordinates
                    << std::fixed
                    << std::setprecision(2)
                    << "P1=("
                    << worldP1.x << ", "
                    << worldP1.y
                    << ")  P2=("
                    << worldP2.x << ", "
                    << worldP2.y
                    << ") "
                    << unit;

                cv::putText(
                    display,
                    coordinates.str(),
                    cv::Point(20, display.rows - 30),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.55,
                    cv::Scalar(0, 255, 255),
                    2
                );
            }
            catch (const std::exception& e)
            {
                std::cerr
                    << "Measurement error: "
                    << e.what()
                    << std::endl;
            }
        }


        cv::putText(
            display,
            "Select two points | R=reset | ESC=return",
            cv::Point(20, 35),
            cv::FONT_HERSHEY_SIMPLEX,
            0.65,
            cv::Scalar(0, 255, 255),
            2
        );

        cv::imshow(
            windowName,
            display
        );

        int key =
            cv::waitKey(20);

        if (key == 27)
        {
            break;
        }

        if (key == 'r' ||
            key == 'R')
        {
            selector.resetPoints();
        }
    }

    cv::destroyWindow(windowName);
}


// ============================================================
// MAIN
// ============================================================

int main(int argc, char** argv)
{
    // ------------------------------------------------------------
    // Default configuration
    // ------------------------------------------------------------

    std::string calibrationFile =
        "camera_parameters.xml";

    std::string planeFile =
        "measurement_plane.yml";

    int cameraID = 0;

    // Number of INTERNAL checkerboard corners.
    int boardColumns = 9;
    int boardRows = 6;

    // Real checkerboard-square dimension.
    double squareSize = 25.0;

    std::string measurementUnit =
        "mm";


    // ------------------------------------------------------------
    // Optional command-line arguments
    //
    // program calibration.yml plane.yml cameraID
    //         boardColumns boardRows squareSize
    // ------------------------------------------------------------

    if (argc >= 2)
        calibrationFile = argv[1];

    if (argc >= 3)
        planeFile = argv[2];

    if (argc >= 4)
        cameraID = std::stoi(argv[3]);

    if (argc >= 5)
        boardColumns = std::stoi(argv[4]);

    if (argc >= 6)
        boardRows = std::stoi(argv[5]);

    if (argc >= 7)
        squareSize = std::stod(argv[6]);


    std::cout
        << "============================================\n"
        << " REAL-WORLD OBJECT MEASUREMENT\n"
        << " C++ / OpenCV\n"
        << "============================================\n";

    std::cout
        << "Camera calibration file: "
        << calibrationFile << "\n";

    std::cout
        << "Plane calibration file: "
        << planeFile << "\n";

    std::cout
        << "Checkerboard internal corners: "
        << boardColumns
        << " x "
        << boardRows << "\n";

    std::cout
        << "Checkerboard square size: "
        << squareSize
        << " "
        << measurementUnit << "\n";


    // ============================================================
    // Load camera calibration
    // ============================================================

    CameraCalibration cameraCalibration;

    if (!cameraCalibration.loadCalibration(
        calibrationFile))
    {
        return -1;
    }


    // ============================================================
    // Measurement plane
    // ============================================================

    MeasurementPlane plane;

    if (plane.loadPose(
        planeFile,
        cameraCalibration.getCameraMatrix()))
    {
        std::cout
            << "Existing plane pose will be used.\n";
    }
    else
    {
        std::cout
            << "\nNo measurement-plane calibration found.\n"
            << "Press P while the checkerboard is visible "
            << "to calibrate the plane.\n";
    }


    // ============================================================
    // Open camera
    // ============================================================

    cv::VideoCapture camera(cameraID);

    if (!camera.isOpened())
    {
        std::cerr
            << "ERROR: Camera could not be opened.\n";

        return -1;
    }


    const std::string cameraWindow =
        "Live Camera";

    cv::namedWindow(
        cameraWindow,
        cv::WINDOW_NORMAL
    );


    std::cout
        << "\n============================================\n"
        << "CONTROLS\n"
        << "============================================\n"
        << "P   Calibrate measurement plane\n"
        << "C   Capture static image and measure\n"
        << "ESC Exit program\n"
        << "============================================\n";


    // ============================================================
    // Main camera loop
    // ============================================================

    while (true)
    {
        cv::Mat frame;

        camera >> frame;

        if (frame.empty())
        {
            std::cerr
                << "ERROR: Empty camera frame.\n";

            break;
        }


        cv::Mat display =
            frame.clone();


        // --------------------------------------------------------
        // Display instructions
        // --------------------------------------------------------

        cv::putText(
            display,
            "P = calibrate plane",
            cv::Point(20, 30),
            cv::FONT_HERSHEY_SIMPLEX,
            0.65,
            cv::Scalar(0, 255, 255),
            2
        );

        cv::putText(
            display,
            "C = capture and measure",
            cv::Point(20, 60),
            cv::FONT_HERSHEY_SIMPLEX,
            0.65,
            cv::Scalar(0, 255, 255),
            2
        );


        if (plane.isCalibrated())
        {
            cv::putText(
                display,
                "Measurement plane: CALIBRATED",
                cv::Point(20, 90),
                cv::FONT_HERSHEY_SIMPLEX,
                0.65,
                cv::Scalar(0, 255, 0),
                2
            );
        }
        else
        {
            cv::putText(
                display,
                "Measurement plane: NOT CALIBRATED",
                cv::Point(20, 90),
                cv::FONT_HERSHEY_SIMPLEX,
                0.65,
                cv::Scalar(0, 0, 255),
                2
            );
        }


        cv::imshow(
            cameraWindow,
            display
        );


        int key =
            cv::waitKey(20);


        // ========================================================
        // ESC = exit
        // ========================================================

        if (key == 27)
        {
            break;
        }


        // ========================================================
        // P = measurement-plane calibration
        // ========================================================

        if (key == 'p' ||
            key == 'P')
        {
            std::cout
                << "\nAttempting measurement-plane calibration...\n";

            cv::Size boardSize(
                boardColumns,
                boardRows
            );

            bool success =
                plane.calibrateFromChessboard(
                    frame,
                    cameraCalibration.getCameraMatrix(),
                    cameraCalibration.getDistCoeffs(),
                    boardSize,
                    squareSize
                );

            if (success)
            {
                plane.savePose(
                    planeFile
                );

                std::cout
                    << "\nIMPORTANT:\n"
                    << "Do not move the camera or measurement "
                    << "surface after this calibration.\n";

                std::cout
                    << "The checkerboard can now be removed.\n";

                cv::waitKey(1000);

                cv::destroyWindow(
                    "Plane Calibration"
                );
            }
        }


        // ========================================================
        // C = capture static image and measure
        // ========================================================

        if (key == 'c' ||
            key == 'C')
        {
            if (!plane.isCalibrated())
            {
                std::cerr
                    << "\nERROR: The measurement plane "
                    << "has not been calibrated.\n"
                    << "Place the checkerboard on the "
                    << "measurement surface and press P first.\n";

                continue;
            }


            // ----------------------------------------------------
            // Correct lens distortion
            // ----------------------------------------------------

            cv::Mat corrected =
                cameraCalibration.undistortImage(
                    frame
                );


            // ----------------------------------------------------
            // Enter static measurement mode
            // ----------------------------------------------------

            measurementMode(
                corrected,
                plane,
                measurementUnit
            );
        }
    }


    camera.release();

    cv::destroyAllWindows();

    std::cout
        << "\nProgram finished.\n";

    return 0;
}