
#include "GeneralOCV.h"
#include "Calibration_Data.h"
#include "Image_Captured.h"
#include "Camera_Calibration.h"
#include "Image_Corrector.h"

using namespace std;

//////////////////////////////////////////////////////////
// Helper: timestamp-based filename
//////////////////////////////////////////////////////////
std::string generateFilename(int counter) {
    auto t = std::time(nullptr);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream ss;
    ss << "capture_"
        << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S")
        << "_" << std::setw(2) << std::setfill('0') << counter
        << ".png";

    return ss.str();
}

//////////////////////////////////////////////////////////
// MAIN PROGRAM (ACQUISITION LOOP LIVES ONLY HERE)
//////////////////////////////////////////////////////////
int main() {

    cv::FileStorage fs("exercises/project_01/config.xml", cv::FileStorage::READ);

    if (!fs.isOpened()) {
        std::cerr << "ERROR: Could not open config.xml" << std::endl;
        std::cerr << "Make sure config.xml is in the program's working directory."
                  << std::endl;
        //return -1;
    }
    int board_w, board_h;
    float squareSize;
    int numberOfFrames = 20;
    int baseLine = 0;
    Size textSize;
    Point textOrigin;
    Point textnext;

    fs["BoardSize_Width"] >> board_w;
    fs["BoardSize_Height"] >> board_h;
    fs["Square_Size"] >> squareSize;
    fs["Calibrate_NrOfFrameToUse"] >> numberOfFrames;

    if (numberOfFrames <= 0)
        numberOfFrames = 20;
    fs.release();


    cv::Size boardSize(board_w, board_h);

    int experiment;

std::cout << "Select experiment:" << std::endl;
std::cout << "1 - Flexible / Center" << std::endl;
std::cout << "2 - Flexible / Full" << std::endl;
std::cout << "3 - Rigid / Center" << std::endl;
std::cout << "4 - Rigid / Full" << std::endl;
std::cout << "> ";

std::cin >> experiment;

std::string experimentName;

switch (experiment) {
    case 1:
        experimentName = "exp1_flexible_center";
        break;

    case 2:
        experimentName = "exp2_flexible_full";
        break;

    case 3:
        experimentName = "exp3_rigid_center";
        break;

    case 4:
        experimentName = "exp4_rigid_full";
        break;

    default:
        std::cerr << "Invalid experiment." << std::endl;
        return -1;
}

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Camera not found!" << std::endl;
        return -1;
    }

    std::vector<Image_Captured> capturedImages;
    int successful = 0;
    int counter = 1;

    string msg0 = "selec R for begining ";

    string msg = "capturing";


    cv::Mat frame;
    bool fready = false;
    char cready;

    textSize = getTextSize(msg0, 1, 2, 3, &baseLine);
    cap >> frame;
    if (!frame.empty()) {
        textOrigin.x = frame.cols - 2 * textSize.width - 10;
        textOrigin.y = frame.rows - 2 * baseLine - 10;

    }
    else {
        std::cout << "error in the camera";
    }
   
    while (!fready) {
        cap >> frame;
        putText(frame, msg0, textOrigin, 1, 2, GREEN, 3);
        cv::imshow("Camera", frame);
        cready= waitKey(20);
        if (cready == 'r' || cready == 'R')
            fready = true;
    }

    while (successful < numberOfFrames) {

        //////////////////////////////
        // 1. Show live frame (1 sec)
        //////////////////////////////
        //cv::Mat frame;
        fready = false;
        
        for (int i=0;i<numberOfFrames;i++) {
            cap >> frame;     
            cv::imshow("Camera", frame);
            cv::waitKey(10);
        }
        //////////////////////////////
        // 2. Next frame: detect keypoints
        //////////////////////////////
        cap >> frame;
        if (frame.empty()) continue;
         cv::imshow("Camera", frame);
         cv::waitKey(5);

        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(frame, boardSize, corners);

        if (found) {
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
            cv::Mat rawFrame = frame.clone();
            cv::Mat displayFrame = rawFrame.clone();

            cv::drawChessboardCorners(
                displayFrame,
                boardSize,
                corners,
                true
            );

            std::string file =
                "exercises/project_01/" +
                experimentName +
                "_" +
                generateFilename(counter++);

            Image_Captured img(
                rawFrame,
                corners,
                file
            );
            capturedImages.push_back(img);

            successful++;
           
           
            textSize = getTextSize(msg, 1, 2, 3, &baseLine);
            textOrigin.x = frame.cols - 2 * textSize.width - 10;
            textOrigin.y= frame.rows - 2 * baseLine - 10;

            textnext.x = frame.cols - 2 * textSize.width - 10;
            textnext.y=frame.rows - 3 * baseLine - 10;
            msg = format(
                "%d/%d Undist",
                successful,
                numberOfFrames
            );
            putText(frame, msg, textOrigin, 1, 2, GREEN, 3);

            cv::imshow("Camera", frame);
            cv::waitKey(750);
            putText(frame, "Next frame", textnext, 1, 2, GREEN, 3);
            cv::imshow("Camera", frame);
            cv::waitKey(250);

            std::cout << "Captured "
            << successful
            << " / "
            << numberOfFrames
            << std::endl;
        }

        else {
            cv::putText(frame, "Pattern not found", cv::Point(50, 50),
                cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
            cv::imshow("Camera", frame);
            cv::waitKey(500);
        }
    }

    ////////////////////////////////////////////////////////
    // Perform calibration
    ////////////////////////////////////////////////////////
    cout << "calibrating....";
    Camera_Calibration calibManager(capturedImages);
    Calibration_Data data = calibManager.performCalibration(boardSize, squareSize);

    std::string base =
        "exercises/project_01/" +
        experimentName;

    data.saveToXML(
        base + "_parameters.xml"
    );

    data.saveErrorsToCSV(
        base + "_errors.csv"
    );

    data.saveSummaryToCSV(
        base + "_summary.csv"
    );


    ////////////////////////////////////////////////////////
    // Show menu
    ////////////////////////////////////////////////////////
    Image_Corrector corrector(data);
    //cv::Mat frame;
    char key = 'N';
    bool undistor = false;
    while (true) {
        cap >> frame;
        // cv::Mat menu(400, 600, CV_8UC3, cv::Scalar(50, 50, 50));
        cv::putText(frame, "Calibration Successful", cv::Point(50, 80),
            cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        cv::putText(frame, "Apply correction? (Y/N)", cv::Point(50, 160),
            cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);


        if (undistor)
            corrector.undistortAndShow(frame);
        else
            cv::imshow("Camera", frame);
        key = cv::waitKey(10);
        if (key == 'Y' || key == 'y') {
            undistor = true;
        }
        if (key == 'N' || key == 'n')
            undistor = false;
        if (key == 27) break;

    }

    return 0;
}
