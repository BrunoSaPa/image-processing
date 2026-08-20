#include <iostream>
#include <opencv2/opencv.hpp>

int main()
{

    // Open the input image from the repository root.
    cv::Mat inputImage = cv::imread("resources/greyscale.png", cv::IMREAD_GRAYSCALE);
    if (inputImage.empty())
    {
        std::cerr << "Could not open resources/greyscale.png\n";
        return 1;
    }


    //new image
    cv::Mat A(inputImage.rows, inputImage.cols, CV_8U);

    // Apply threshold on grayscale image (white or black only)
    for (int i = 0; i < inputImage.rows; i++)
    {
        for (int j = 0; j < inputImage.cols; j++)
        {
            if (inputImage.at<uchar>(i, j) > 127)
            {
                A.at<uchar>(i, j) = 255;
            }
            else
            {
                A.at<uchar>(i, j) = 0;
            }
        }
    }


    //Create a window and display the image
    cv::imshow("Modified image", A);
    cv::imshow("Original image", inputImage);

    // Wait for a keystroke in the window
    cv::waitKey(0);

    return 0;
}
