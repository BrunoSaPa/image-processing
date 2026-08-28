#pragma once

#include "GeneralOCV.h"
#include "Calibration_Data.h"
//////////////////////////////////////////////////////////
// image_corrector
//////////////////////////////////////////////////////////
class Image_Corrector {
private:
    Calibration_Data calib;

public:
    Image_Corrector(const Calibration_Data& c) : calib(c) {}

    void undistortAndShow(const cv::Mat& frame) {
        cv::Mat undist;
        cv::undistort(frame, undist, calib.cameraMatrix, calib.distCoeffs);
        cv::imshow("Camera", undist);
    }
};

