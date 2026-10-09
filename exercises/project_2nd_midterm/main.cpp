#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

using namespace cv;
using namespace std;

static const string RESULT_WINDOW = "Detected features";
static const string CONTROL_WINDOW = "Detector controls";
static const Rect RECALC_BTN(42, 125, 270, 62);
static const Rect BACK_BTN(350, 125, 270, 62);

struct Application {
    Mat original, gray, controls;
    int detector = 0;
    bool recalculate = false;
    bool returnToMenu = false;
    string status = "Adjust parameters, then click Recalculate.";
    // Trackbar positions (integers). Physical values are converted in process().
    int cannyLow = 50, cannyHigh = 150, cannyAperture = 0, cannyL2 = 0;
    int harrisBlock = 2, harrisAperture = 0, harrisK = 4, harrisThreshold = 1;
    int lineRho = 10, lineTheta = 10, lineVotes = 60, lineMinLength = 40, lineMaxGap = 10;
    int circleDp = 12, circleMinDist = 30, circleParam1 = 120, circleParam2 = 30;
    int circleMinRadius = 0, circleMaxRadius = 0;
    int siftNFeatures = 0, siftLayers = 3, siftContrast = 40, siftEdge = 100, siftSigma = 16;
};

static void drawControls(Application& app) {
    app.controls = Mat(250, 665, CV_8UC3, Scalar(244, 244, 244));
    const vector<string> titles = {"", "CANNY EDGES", "HARRIS CORNERS", "HOUGH LINES",
                                   "HOUGH CIRCLES", "SIFT KEYPOINTS"};
    putText(app.controls, titles.at(app.detector), Point(30, 42), FONT_HERSHEY_SIMPLEX,
            0.92, Scalar(45, 45, 45), 2, LINE_AA);
    putText(app.controls, "Move sliders, then click Recalculate to apply changes.",
            Point(30, 86), FONT_HERSHEY_SIMPLEX, 0.50, Scalar(75, 75, 75), 1, LINE_AA);
    rectangle(app.controls, RECALC_BTN, Scalar(40, 135, 65), FILLED);
    rectangle(app.controls, BACK_BTN, Scalar(130, 85, 40), FILLED);
    putText(app.controls, "RECALCULATE", Point(69, 165), FONT_HERSHEY_SIMPLEX,
            0.67, Scalar(255, 255, 255), 2, LINE_AA);
    putText(app.controls, "MAIN MENU", Point(409, 165), FONT_HERSHEY_SIMPLEX,
            0.67, Scalar(255, 255, 255), 2, LINE_AA);
    putText(app.controls, app.status.substr(0, 85), Point(30, 224), FONT_HERSHEY_SIMPLEX,
            0.48, Scalar(40, 40, 40), 1, LINE_AA);
    imshow(CONTROL_WINDOW, app.controls);
}

static void onControlMouse(int event, int x, int y, int, void* userdata) {
    if (event != EVENT_LBUTTONDOWN || userdata == nullptr) return;
    auto& app = *static_cast<Application*>(userdata);
    if (RECALC_BTN.contains(Point(x, y))) app.recalculate = true;
    if (BACK_BTN.contains(Point(x, y))) app.returnToMenu = true;
}

static void track(const string& name, int* value, int maximum) {
    createTrackbar(name, CONTROL_WINDOW, value, maximum);
}

static void setupTrackbars(Application& a) {
    switch (a.detector) {
        case 1:
            track("Lower threshold", &a.cannyLow, 500);
            track("Upper threshold", &a.cannyHigh, 500);
            track("Aperture (0=3,1=5,2=7)", &a.cannyAperture, 2);
            track("L2 gradient (0/1)", &a.cannyL2, 1);
            break;
        case 2:
            track("Block size (2 + value)", &a.harrisBlock, 18);
            track("Aperture (0=3,1=5,2=7)", &a.harrisAperture, 2);
            track("k (value / 100)", &a.harrisK, 15);
            track("Threshold (% of max)", &a.harrisThreshold, 100);
            break;
        case 3:
            track("rho (value / 10 px)", &a.lineRho, 50);
            track("theta (value / 10 deg)", &a.lineTheta, 900);
            track("Vote threshold", &a.lineVotes, 300);
            track("Min line length (px)", &a.lineMinLength, 500);
            track("Max line gap (px)", &a.lineMaxGap, 300);
            break;
        case 4:
            track("dp (value / 10)", &a.circleDp, 50);
            track("Minimum center distance", &a.circleMinDist, 500);
            track("Canny high threshold", &a.circleParam1, 500);
            track("Center accumulator threshold", &a.circleParam2, 200);
            track("Minimum radius", &a.circleMinRadius, 500);
            track("Maximum radius (0=any)", &a.circleMaxRadius, 500);
            break;
        case 5:
            track("Number features (0=all)", &a.siftNFeatures, 2000);
            track("Layers per octave", &a.siftLayers, 10);
            track("Contrast (value / 1000)", &a.siftContrast, 200);
            track("Edge threshold (/ 10)", &a.siftEdge, 300);
            track("Sigma (value / 10)", &a.siftSigma, 50);
            break;
    }
}

static void process(Application& a) {
    Mat output = a.original.clone();
    try {
        if (a.detector == 1) {
            Mat edge;
            int aperture = 3 + 2 * a.cannyAperture;
            int low = min(a.cannyLow, a.cannyHigh);
            int high = max(a.cannyLow, a.cannyHigh);
            Canny(a.gray, edge, low, high, aperture, a.cannyL2 != 0);
            output.setTo(Scalar(0, 0, 255), edge);
            a.status = "Canny: " + to_string(countNonZero(edge)) + " edge pixels";
            cout << "Canny: low=" << low << ", high=" << high
                 << ", aperture=" << aperture << ", L2=" << a.cannyL2 << '\n';
        } else if (a.detector == 2) {
            Mat response, dilated;
            int block = max(2, a.harrisBlock + 2);
            int aperture = 3 + 2 * a.harrisAperture;
            double k = max(0.01, a.harrisK / 100.0);
            cornerHarris(a.gray, response, block, aperture, k);
            double maxValue = 0;
            minMaxLoc(response, nullptr, &maxValue);
            dilate(response, dilated, Mat());
            int count = 0;
            float threshold = static_cast<float>(maxValue * max(1, a.harrisThreshold) / 100.0);
            for (int y = 1; y < response.rows - 1; ++y)
                for (int x = 1; x < response.cols - 1; ++x) {
                    float r = response.at<float>(y, x);
                    if (maxValue > 0 && r > threshold && r == dilated.at<float>(y, x)) {
                        circle(output, Point(x, y), 3, Scalar(0, 255, 0), 1, LINE_AA);
                        ++count;
                    }
                }
            a.status = "Harris: " + to_string(count) + " corner candidates";
            cout << "Harris: block=" << block << ", aperture=" << aperture
                 << ", k=" << k << ", cutoff=" << max(1,a.harrisThreshold) << "%\n";
        } else if (a.detector == 3) {
            Mat edges;
            Canny(a.gray, edges, 50, 150);
            vector<Vec4i> lines;
            double rho = max(1, a.lineRho) / 10.0;
            double theta = max(1, a.lineTheta) * CV_PI / 1800.0;
            HoughLinesP(edges, lines, rho, theta, max(1, a.lineVotes),
                        a.lineMinLength, a.lineMaxGap);
            for (auto& l : lines)
                line(output, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 255, 0), 2, LINE_AA);
            a.status = "Hough lines: " + to_string(lines.size()) + " segments";
            cout << "Hough lines: rho=" << rho << ", theta=" << theta * 180 / CV_PI
                 << " deg, votes=" << max(1,a.lineVotes) << ", minLength="
                 << a.lineMinLength << ", maxGap=" << a.lineMaxGap << '\n';
        } else if (a.detector == 4) {
            Mat smooth;
            GaussianBlur(a.gray, smooth, Size(9, 9), 2);
            vector<Vec3f> circles;
            int minR = a.circleMinRadius;
            int maxR = a.circleMaxRadius;
            if (maxR != 0 && maxR < minR) maxR = minR;
            HoughCircles(smooth, circles, HOUGH_GRADIENT, max(10, a.circleDp) / 10.0,
                         max(1, a.circleMinDist), max(1, a.circleParam1),
                         max(1, a.circleParam2), minR, maxR);
            for (const auto& c : circles) {
                Point p(cvRound(c[0]), cvRound(c[1]));
                circle(output, p, cvRound(c[2]), Scalar(0, 255, 0), 2, LINE_AA);
                circle(output, p, 2, Scalar(0, 0, 255), FILLED, LINE_AA);
            }
            a.status = "Hough circles: " + to_string(circles.size()) + " circles";
            cout << "Hough circles: dp=" << max(10,a.circleDp)/10.0
                 << ", minDist=" << max(1,a.circleMinDist) << ", param1="
                 << max(1,a.circleParam1) << ", param2=" << max(1,a.circleParam2)
                 << ", minR=" << minR << ", maxR=" << maxR << '\n';
        } else if (a.detector == 5) {
            vector<KeyPoint> keypoints;
            Mat descriptors;
            auto sift = SIFT::create(a.siftNFeatures, max(1, a.siftLayers),
                                     max(1, a.siftContrast) / 1000.0,
                                     max(1, a.siftEdge) / 10.0,
                                     max(1, a.siftSigma) / 10.0);
            sift->detectAndCompute(a.gray, noArray(), keypoints, descriptors);
            drawKeypoints(a.original, keypoints, output, Scalar(0, 255, 255),
                          DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
            a.status = "SIFT: " + to_string(keypoints.size()) + " keypoints";
            cout << "SIFT: maxFeatures=" << a.siftNFeatures << ", layers="
                 << max(1,a.siftLayers) << ", contrast=" << max(1,a.siftContrast)/1000.0
                 << ", edge=" << max(1,a.siftEdge)/10.0
                 << ", sigma=" << max(1,a.siftSigma)/10.0 << '\n';
        }
        cout << a.status << "\n\n";
        imshow(RESULT_WINDOW, output);
    } catch (const cv::Exception& e) {
        a.status = "Invalid settings: check terminal.";
        cerr << "OpenCV error: " << e.what() << '\n';
    }
    drawControls(a);
}

int main(int argc, char** argv) {
    Application app;
    string path;
    if (argc > 1) path = argv[1];
    else {
        cout << "Image path: ";
        getline(cin, path);
        if (path.size() >= 2 && path.front() == '"' && path.back() == '"')
            path = path.substr(1, path.size() - 2);
    }
    app.original = imread(path, IMREAD_COLOR);
    if (app.original.empty()) {
        cerr << "Cannot read image: " << path << '\n';
        return 1;
    }
    cvtColor(app.original, app.gray, COLOR_BGR2GRAY);
    while (true) {
        cout << "\n========== SELECT DETECTOR ==========\n"
             << "1. Canny edges\n2. Harris corners\n3. Hough lines\n"
             << "4. Hough circles\n5. SIFT keypoints\n0. Exit\nChoice: ";
        int choice;
        if (!(cin >> choice)) {
            if (cin.eof()) break;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Enter a number from 0 to 5.\n";
            continue;
        }
        if (choice == 0) break;
        if (choice < 1 || choice > 5) {
            cout << "Invalid option.\n";
            continue;
        }
        app.detector = choice;
        app.recalculate = false;
        app.returnToMenu = false;
        app.status = "Adjust parameters, then click Recalculate.";
        namedWindow(RESULT_WINDOW, WINDOW_NORMAL);
        namedWindow(CONTROL_WINDOW, WINDOW_NORMAL);
        resizeWindow(CONTROL_WINDOW, 665, 450);
        setMouseCallback(CONTROL_WINDOW, onControlMouse, &app);
        setupTrackbars(app);
        process(app);
        while (!app.returnToMenu) {
            if (app.recalculate) {
                app.recalculate = false;
                process(app);
            }
            int key = waitKey(30);
            if (key == 27 || key == 'm' || key == 'M') app.returnToMenu = true;
            // If either window was manually closed, return to the menu.
            if (getWindowProperty(RESULT_WINDOW, WND_PROP_VISIBLE) < 1 ||
                getWindowProperty(CONTROL_WINDOW, WND_PROP_VISIBLE) < 1)
                app.returnToMenu = true;
        }
        destroyWindow(RESULT_WINDOW);
        destroyWindow(CONTROL_WINDOW);
        waitKey(1);
    }
    destroyAllWindows();
    return 0;
}
