#pragma once

#include "GeneralOCV.h"
/////////////////////////////////////////////////////////
// calibration_data
//////////////////////////////////////////////////////////
class Calibration_Data {
public:
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

    Calibration_Data() {
        cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
        distCoeffs = cv::Mat::zeros(5, 1, CV_64F);
    }

    void saveToXML(const std::string& file) {
        cv::FileStorage fs(file, cv::FileStorage::WRITE);
        fs << "camera_matrix" << cameraMatrix;
        fs << "dist_coeffs" << distCoeffs;
        fs.release();
    }
};

