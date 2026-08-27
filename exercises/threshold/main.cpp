#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main()
{

    // Open the input image from the repository root.
    Mat inputImage = cv::imread("resources/gradient_horizontal.png", IMREAD_GRAYSCALE);
    if (inputImage.empty())
    {
        cerr << "Could not open resources/gradient_horizontal.png\n";
        return 1;
    }

    //Display original
    imshow("Original image", inputImage);

    //new image
    Mat A(inputImage.rows, inputImage.cols, CV_8U);

    //Binary thresholding
    int threshold = 127;
    for (int i = 0; i < inputImage.rows; i++)
    {
        for (int j = 0; j < inputImage.cols; j++)
        {
            if (inputImage.at<uchar>(i, j) > threshold)
            {
                A.at<uchar>(i, j) = 255;
            }
            else
            {
                A.at<uchar>(i, j) = 0;
            }
        }
    }

    //Display modified
    imshow("Binary", A);

    // Binary inverse thresholding
    for (int i = 0; i < inputImage.rows; i++)
    {
        for (int j = 0; j < inputImage.cols; j++)
        {
            if (inputImage.at<uchar>(i, j) < threshold)
            {
                A.at<uchar>(i, j) = 255;
            }
            else
            {
                A.at<uchar>(i, j) = 0;
            }
        }
    }

    imshow("Binary Inverse", A);

    // Truncate thresholding
    for (int i = 0; i < inputImage.rows; i++)
    {
        for (int j = 0; j < inputImage.cols; j++)
        {
            if (inputImage.at<uchar>(i, j) > threshold)
            {
                A.at<uchar>(i, j) = threshold;
            }
            else
            {
                A.at<uchar>(i, j) = inputImage.at<uchar>(i, j);
            }
        }
    }

    imshow("Truncate",A);


    //To zero thresholding
    for (int i = 0; i < inputImage.rows; i++)
    {
        for (int j = 0; j < inputImage.cols; j++)
        {
            if (inputImage.at<uchar>(i, j) < threshold)
            {
                A.at<uchar>(i, j) = 0;
            }
            else
            {
                A.at<uchar>(i, j) = inputImage.at<uchar>(i, j);
            }
        }
    }

    imshow("To Zero", A);

    //To zero inverse thresholding
    for (int i = 0; i < inputImage.rows; i++)
    {
        for (int j = 0; j < inputImage.cols; j++)
        {
            if (inputImage.at<uchar>(i, j) > threshold)
            {
                A.at<uchar>(i, j) = 0;
            }
            else
            {
                A.at<uchar>(i, j) = inputImage.at<uchar>(i, j);
            }
        }
    }

    imshow("To Zero Inverse", A);


    // Wait for a keystroke in the window
    waitKey(0);

    return 0;
}
