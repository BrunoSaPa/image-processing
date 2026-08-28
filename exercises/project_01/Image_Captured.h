#pragma once

#include "GeneralOCV.h"
//////////////////////////////////////////////////////////
// image_captured
//////////////////////////////////////////////////////////
class Image_Captured {
public:
    Mat image;                       // processed image with points drawn
    std::vector<cv::Point2f> points;     // detected keypoints (NOT saved to file)
    std::string filename;                // filename of the saved image

    Image_Captured(const cv::Mat& img,
        const std::vector<cv::Point2f>& pts,
        const std::string& file)
        : image(img.clone()), points(pts), filename(file)
    {
        cv::imwrite(filename, img);
    }
};

