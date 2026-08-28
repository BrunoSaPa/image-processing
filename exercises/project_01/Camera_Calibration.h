#pragma once

#include "GeneralOCV.h"
#include "Calibration_Data.h"
#include "Image_Captured.h"

//////////////////////////////////////////////////////////
// camera_calibration
//////////////////////////////////////////////////////////
class Camera_Calibration {
private:
    std::vector<Image_Captured>& captures;

public:
    Camera_Calibration(std::vector<Image_Captured>& cap)
        : captures(cap) {
    }

    Calibration_Data performCalibration(cv::Size boardSize, float squareSize) {

        std::vector<std::vector<cv::Point3f>> objectPoints;
        std::vector<std::vector<cv::Point2f>> imagePoints;

        // Prepare object coordinates
        std::vector<cv::Point3f> obj;
        for (int i = 0; i < boardSize.height; i++)
            for (int j = 0; j < boardSize.width; j++)
                obj.push_back(cv::Point3f(j * squareSize, i * squareSize, 0));

        // For each captured image
        for (auto& cap : captures) {
            imagePoints.push_back(cap.points);
            objectPoints.push_back(obj);
        }

        Calibration_Data result;

        std::vector<cv::Mat> rvecs, tvecs;

        double rms = cv::calibrateCamera(objectPoints,
            imagePoints,
            captures[0].image.size(),
            result.cameraMatrix,
            result.distCoeffs,
            rvecs,
            tvecs);

        std::cout << "RMS error = " << rms << std::endl;

        return result;
    }
};

