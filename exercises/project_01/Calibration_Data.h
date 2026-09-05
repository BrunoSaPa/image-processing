#pragma once

#include "GeneralOCV.h"

//////////////////////////////////////////////////////////
// Calibration_Data
//////////////////////////////////////////////////////////
class Calibration_Data {
public:
    // Camera parameters
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

    // Extrinsic parameters for every calibration image
    std::vector<cv::Mat> rvecs;
    std::vector<cv::Mat> tvecs;

    // Error statistics
    double rms;
    double rmsRecomputed;
    double meanError;
    double maxError;

    // Error for every image and every detected point
    std::vector<double> perImageRMS;
    std::vector<std::vector<double>> perPointErrors;

    // Experimental information
    cv::Size imageSize;
    int boardWidth;
    int boardHeight;
    float squareSize;

    Calibration_Data()
        : rms(0.0),
          rmsRecomputed(0.0),
          meanError(0.0),
          maxError(0.0),
          boardWidth(0),
          boardHeight(0),
          squareSize(0.0f)
    {
        cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
        distCoeffs = cv::Mat::zeros(5, 1, CV_64F);
    }

    //////////////////////////////////////////////////////
    // Save complete calibration
    //////////////////////////////////////////////////////
    void saveToXML(const std::string& file) const {
        cv::FileStorage fs(file, cv::FileStorage::WRITE);

        fs << "image_width" << imageSize.width;
        fs << "image_height" << imageSize.height;

        fs << "board_width" << boardWidth;
        fs << "board_height" << boardHeight;
        fs << "square_size" << squareSize;

        fs << "camera_matrix" << cameraMatrix;
        fs << "dist_coeffs" << distCoeffs;

        fs << "opencv_rms" << rms;
        fs << "recomputed_rms" << rmsRecomputed;
        fs << "mean_reprojection_error" << meanError;
        fs << "max_reprojection_error" << maxError;

        fs << "per_image_rms" << perImageRMS;

        fs << "rvecs" << "[";
        for (const auto& rvec : rvecs)
            fs << rvec;
        fs << "]";

        fs << "tvecs" << "[";
        for (const auto& tvec : tvecs)
            fs << tvec;
        fs << "]";

        fs.release();
    }

    //////////////////////////////////////////////////////
    // Save every point error
    //////////////////////////////////////////////////////
    void saveErrorsToCSV(const std::string& file) const {
        std::ofstream out(file);

        if (!out.is_open()) {
            std::cerr << "Could not create " << file << std::endl;
            return;
        }

        out << "image,point,error_px\n";

        for (size_t i = 0; i < perPointErrors.size(); i++) {
            for (size_t j = 0; j < perPointErrors[i].size(); j++) {
                out << (i + 1) << ","
                    << (j + 1) << ","
                    << perPointErrors[i][j]
                    << "\n";
            }
        }

        out.close();
    }

    //////////////////////////////////////////////////////
    // Save summary for report
    //////////////////////////////////////////////////////
    void saveSummaryToCSV(const std::string& file) const {
        std::ofstream out(file);

        if (!out.is_open()) {
            std::cerr << "Could not create " << file << std::endl;
            return;
        }

        out << "metric,value\n";
        out << "opencv_rms," << rms << "\n";
        out << "recomputed_rms," << rmsRecomputed << "\n";
        out << "mean_error," << meanError << "\n";
        out << "max_error," << maxError << "\n";

        out.close();
    }
};