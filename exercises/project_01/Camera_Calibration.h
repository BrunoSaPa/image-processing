#pragma once

#include "GeneralOCV.h"
#include "Calibration_Data.h"
#include "Image_Captured.h"

//////////////////////////////////////////////////////////
// Camera_Calibration
//////////////////////////////////////////////////////////
class Camera_Calibration {
private:
    std::vector<Image_Captured>& captures;

public:
    Camera_Calibration(std::vector<Image_Captured>& cap)
        : captures(cap) {
    }

    Calibration_Data performCalibration(
        cv::Size boardSize,
        float squareSize
    ) {
        Calibration_Data result;

        if (captures.empty()) {
            std::cerr << "ERROR: No calibration images." << std::endl;
            return result;
        }

        std::vector<std::vector<cv::Point3f>> objectPoints;
        std::vector<std::vector<cv::Point2f>> imagePoints;

        //////////////////////////////////////////////////
        // Create known 3D checkerboard coordinates
        //////////////////////////////////////////////////
        std::vector<cv::Point3f> obj;

        for (int i = 0; i < boardSize.height; i++) {
            for (int j = 0; j < boardSize.width; j++) {
                obj.push_back(
                    cv::Point3f(
                        j * squareSize,
                        i * squareSize,
                        0.0f
                    )
                );
            }
        }

        //////////////////////////////////////////////////
        // Load detected image points
        //////////////////////////////////////////////////
        for (auto& capture : captures) {
            imagePoints.push_back(capture.points);
            objectPoints.push_back(obj);
        }

        result.imageSize = captures[0].image.size();
        result.boardWidth = boardSize.width;
        result.boardHeight = boardSize.height;
        result.squareSize = squareSize;

        //////////////////////////////////////////////////
        // Camera calibration
        //////////////////////////////////////////////////
        result.rms = cv::calibrateCamera(
            objectPoints,
            imagePoints,
            result.imageSize,
            result.cameraMatrix,
            result.distCoeffs,
            result.rvecs,
            result.tvecs
        );

        //////////////////////////////////////////////////
        // Reprojection error calculation
        //////////////////////////////////////////////////
        result.perPointErrors.resize(imagePoints.size());

        double totalError = 0.0;
        double totalSquaredError = 0.0;
        double maximumError = 0.0;

        size_t totalPoints = 0;

        for (size_t i = 0; i < objectPoints.size(); i++) {

            std::vector<cv::Point2f> projectedPoints;

            cv::projectPoints(
                objectPoints[i],
                result.rvecs[i],
                result.tvecs[i],
                result.cameraMatrix,
                result.distCoeffs,
                projectedPoints
            );

            double imageSquaredError = 0.0;

            for (size_t j = 0; j < imagePoints[i].size(); j++) {

                double dx =
                    imagePoints[i][j].x -
                    projectedPoints[j].x;

                double dy =
                    imagePoints[i][j].y -
                    projectedPoints[j].y;

                double error =
                    std::sqrt(dx * dx + dy * dy);

                result.perPointErrors[i].push_back(error);

                totalError += error;
                totalSquaredError += error * error;
                imageSquaredError += error * error;

                if (error > maximumError)
                    maximumError = error;

                totalPoints++;
            }

            double imageRMS =
                std::sqrt(
                    imageSquaredError /
                    imagePoints[i].size()
                );

            result.perImageRMS.push_back(imageRMS);
        }

        //////////////////////////////////////////////////
        // Final statistics
        //////////////////////////////////////////////////
        result.meanError =
            totalError / static_cast<double>(totalPoints);

        result.rmsRecomputed =
            std::sqrt(
                totalSquaredError /
                static_cast<double>(totalPoints)
            );

        result.maxError = maximumError;

        //////////////////////////////////////////////////
        // Print results
        //////////////////////////////////////////////////
        std::cout << std::endl;
        std::cout << "========== CALIBRATION ==========" << std::endl;

        std::cout << "OpenCV RMS: "
                  << result.rms
                  << " px" << std::endl;

        std::cout << "Recomputed RMS: "
                  << result.rmsRecomputed
                  << " px" << std::endl;

        std::cout << "Mean reprojection error: "
                  << result.meanError
                  << " px" << std::endl;

        std::cout << "Maximum reprojection error: "
                  << result.maxError
                  << " px" << std::endl;

        std::cout << std::endl;
        std::cout << "Per-image RMS:" << std::endl;

        for (size_t i = 0; i < result.perImageRMS.size(); i++) {
            std::cout
                << "Image "
                << i + 1
                << ": "
                << result.perImageRMS[i]
                << " px"
                << std::endl;
        }

        std::cout << "=================================" << std::endl;

        return result;
    }
};