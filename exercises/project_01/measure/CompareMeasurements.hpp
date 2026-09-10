#pragma once
#include <array>
#include <filesystem>
#include <fstream>

// Click on the original image once; undistort these exact pixels for each model.
struct RectangleClicks {
    std::vector<cv::Point2f> points;
    static void mouse(int event, int x, int y, int, void* data) {
        auto& p = static_cast<RectangleClicks*>(data)->points;
        if (event == cv::EVENT_LBUTTONDOWN && p.size() < 8)
            p.emplace_back(float(x), float(y));
        if (event == cv::EVENT_RBUTTONDOWN && !p.empty()) p.pop_back();
    }
};

inline std::array<double, 2> rectangleDimensions(
    const std::vector<cv::Point2f>& raw, size_t offset,
    const CameraCalibration& camera, const MeasurementPlane& plane) {
    std::vector<cv::Point2f> pixels(raw.begin() + offset, raw.begin() + offset + 4), corrected;
    cv::undistortPoints(pixels, corrected, camera.getCameraMatrix(), camera.getDistCoeffs(),
                       cv::noArray(), camera.getCameraMatrix());
    std::array<cv::Point2d, 4> world;
    for (size_t i = 0; i < 4; ++i) world[i] = plane.imageToWorld(corrected[i]);
    // Width follows edge 1->2; height follows edge 2->3. Average opposite edges.
    std::array<double, 2> result = {
        (cv::norm(world[1]-world[0]) + cv::norm(world[2]-world[3])) / 2,
        (cv::norm(world[2]-world[1]) + cv::norm(world[3]-world[0])) / 2};
    for (double d : result)
        if (!std::isfinite(d) || d <= 0) throw std::runtime_error("Invalid rectangle dimensions.");
    return result;
}

inline int compareMeasurements(int argc, char** argv) {
    namespace fs = std::filesystem;
    if (argc < 3 || argc > 7 || argc == 6) {
        std::cout << "Usage: measure --compare CALIBRATION_DIR [NEW_OUTPUT_DIR] [CAMERA_ID]\n"
                     "       measure --compare CALIBRATION_DIR NEW_OUTPUT_DIR CAMERA_ID PLANE_IMAGE SCENE_IMAGE\n";
        return 1;
    }
    const fs::path directory = argv[2];
    const fs::path output = argc > 3 ? argv[3] : "measurement_results";
    const int cameraId = argc > 4 ? std::stoi(argv[4]) : 0;
    const bool offline = argc == 7;
    const std::array<std::string, 4> names = {
        "exp1_flexible_center", "exp2_flexible_full", "exp3_rigid_center", "exp4_rigid_full"};
    std::array<CameraCalibration, 4> cameras;
    std::array<MeasurementPlane, 4> planes;
    cv::Size resolution, board;
    double square = 0;
    for (size_t i = 0; i < names.size(); ++i) {
        const auto path = directory / (names[i] + "_parameters.xml");
        if (!cameras[i].loadCalibration(path.string())) return 1;
        cv::FileStorage metadata(path.string(), cv::FileStorage::READ);
        cv::Size size{int(metadata["image_width"]), int(metadata["image_height"])};
        cv::Size pattern{int(metadata["board_width"]), int(metadata["board_height"])};
        double spacing = double(metadata["square_size"]);
        if (size.width <= 0 || size.height <= 0 || pattern.width < 2 || pattern.height < 2 ||
            !std::isfinite(spacing) || spacing <= 0)
            throw std::runtime_error("Missing or invalid calibration image/board metadata.");
        if (i == 0) { resolution = size; board = pattern; square = spacing; }
        if (size != resolution || pattern != board || spacing != square)
            throw std::runtime_error("All calibrations must use the same resolution and checkerboard.");
    }
    if (fs::exists(output)) throw std::runtime_error("Output directory already exists. Choose a new session directory.");
    cv::VideoCapture capture;
    if (!offline) {
        capture.open(cameraId);
        if (!capture.isOpened()) throw std::runtime_error("Cannot open camera.");
        capture.set(cv::CAP_PROP_FRAME_WIDTH, resolution.width);
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, resolution.height);
    }
    auto checkImage = [&](const cv::Mat& frame) {
        if (frame.empty() || frame.size() != resolution)
            throw std::runtime_error("Image must match calibration resolution: " +
                std::to_string(resolution.width) + " x " + std::to_string(resolution.height));
    };
    auto snapshot = [&](const std::string& instruction, int key) {
        cv::namedWindow("Comparison capture", cv::WINDOW_NORMAL);
        cv::resizeWindow("Comparison capture", 1280, 720);
        std::cout << instruction << '\n';
        for (;;) {
            cv::Mat frame; capture >> frame; checkImage(frame);
            cv::Mat preview = frame.clone();
            cv::putText(preview, instruction, {20, 35}, cv::FONT_HERSHEY_SIMPLEX,
                        0.7, {0, 255, 255}, 2);
            cv::imshow("Comparison capture", preview);
            int pressed = cv::waitKey(20) & 255;
            if (pressed == 27 || cv::getWindowProperty("Comparison capture", cv::WND_PROP_VISIBLE) < 1)
                return cv::Mat();
            if (pressed == key || pressed == key - 32) return frame;
        }
    };
    cv::Mat planeImage;
    for (;;) {
        planeImage = offline ? cv::imread(argv[5]) : snapshot("Place flat checkerboard on measurement plane. P=capture, ESC=cancel", 'p');
        if (!offline && planeImage.empty()) return 0;
        checkImage(planeImage);
        bool success = true;
        for (size_t i = 0; i < planes.size(); ++i) {
            if (!planes[i].calibrateFromChessboard(planeImage, cameras[i].getCameraMatrix(),
                    cameras[i].getDistCoeffs(), board, square)) { success = false; break; }
        }
        if (success) break;
        if (offline) throw std::runtime_error("Checkerboard detection failed in plane image.");
        std::cerr << "Adjust checkerboard visibility and press P to retry.\n";
    }
    cv::destroyWindow("Plane Calibration");
    cv::Mat scene = offline ? cv::imread(argv[6]) : snapshot(
        "Keep camera fixed. Place both squares on same plane. C=capture, ESC=cancel", 'c');
    if (!offline && scene.empty()) return 0;
    checkImage(scene);
    if (!offline) cv::destroyWindow("Comparison capture");
    capture.release();
    RectangleClicks clicks;
    const std::string window = "Rectangle comparison";
    cv::namedWindow(window, cv::WINDOW_NORMAL);
    cv::resizeWindow(window, 1280, 720);
    cv::setMouseCallback(window, RectangleClicks::mouse, &clicks);
    std::cout << "Click four corners around the 75 mm square, then four around the 125 mm square.\n"
                 "Use clockwise or counterclockwise order. Right-click=undo, R=reset, S=save, ESC=cancel.\n";
    cv::Mat annotated;
    for (;;) {
        annotated = scene.clone();
        for (size_t i = 0; i < clicks.points.size(); ++i) {
            cv::circle(annotated, clicks.points[i], 5, {0, 0, 255}, cv::FILLED);
            cv::putText(annotated, std::to_string(i + 1), clicks.points[i] + cv::Point2f(8, -8),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, {0, 255, 255}, 2);
            if (i % 4 != 0) cv::line(annotated, clicks.points[i-1], clicks.points[i], {0,255,0}, 2);
            if (i % 4 == 3) cv::line(annotated, clicks.points[i], clicks.points[i-3], {0,255,0}, 2);
        }
        const auto n = clicks.points.size();
        std::string prompt = n < 8 ? "Click " + std::string(n < 4 ? "75 mm" : "125 mm") +
            " corner " + std::to_string(n % 4 + 1) + "/4" : "S=save all four calibration results";
        cv::putText(annotated, prompt + " | Right-click=undo R=reset ESC=cancel", {20,35},
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, {0,255,255}, 2);
        cv::imshow(window, annotated);
        int key = cv::waitKey(20) & 255;
        if (key == 27 || cv::getWindowProperty(window, cv::WND_PROP_VISIBLE) < 1) { cv::destroyAllWindows(); return 0; }
        if (key == 'r' || key == 'R') clicks.points.clear();
        if ((key == 's' || key == 'S') && clicks.points.size() == 8) {
            bool valid = true;
            for (size_t offset : {size_t(0), size_t(4)}) {
                std::vector<cv::Point2f> polygon(clicks.points.begin()+offset, clicks.points.begin()+offset+4);
                valid = valid && cv::isContourConvex(polygon) && std::abs(cv::contourArea(polygon)) > 1;
            }
            if (valid) break;
            std::cerr << "Select four distinct corners in perimeter order; use R to retry.\n";
        }
    }
    std::ostringstream csv;
    csv << "experiment,object,real_width_mm,real_height_mm,width_mm,height_mm,width_abs_error_mm,height_abs_error_mm,width_error_percent,height_error_percent\n";
    csv << std::fixed << std::setprecision(6);
    for (size_t i = 0; i < cameras.size(); ++i) {
        for (size_t object = 0; object < 2; ++object) {
            auto d = rectangleDimensions(clicks.points, object*4, cameras[i], planes[i]);
            double real = object == 0 ? 75 : 125;
            double ew = std::abs(d[0]-real), eh = std::abs(d[1]-real);
            csv << names[i] << ',' << (object == 0 ? "small" : "large") << ',' << real << ',' << real
                << ',' << d[0] << ',' << d[1] << ',' << ew << ',' << eh << ',' << ew/real*100 << ',' << eh/real*100 << '\n';
        }
    }
    fs::create_directories(output);
    if (!cv::imwrite((output/"plane.png").string(), planeImage) ||
        !cv::imwrite((output/"scene.png").string(), scene) ||
        !cv::imwrite((output/"selected_corners.png").string(), annotated))
        throw std::runtime_error("Could not save session images.");
    cv::FileStorage selection((output/"selected_points.yml").string(), cv::FileStorage::WRITE);
    if (!selection.isOpened()) throw std::runtime_error("Could not save selected points.");
    selection << "raw_image_points" << clicks.points;
    selection.release();
    for (size_t i = 0; i < planes.size(); ++i) {
        if (!planes[i].savePose((output/(names[i]+"_plane.yml")).string()))
            throw std::runtime_error("Could not save plane pose.");
        fs::copy_file(directory/(names[i]+"_parameters.xml"), output/(names[i]+"_parameters.xml"));
    }
    std::ofstream results(output/"measurements.csv");
    results << csv.str(); results.close();
    if (!results) throw std::runtime_error("Could not save measurement CSV.");
    std::cout << csv.str() << "\nSaved session to " << output << '\n';
    cv::destroyAllWindows();
    return 0;
}
