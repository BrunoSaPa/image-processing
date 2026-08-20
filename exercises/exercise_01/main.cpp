#include <iostream>
#include <opencv2/opencv.hpp>

int main()
{
    cv::Mat A(500, 500, CV_8U);

    // Create a black image
    for (int i = 0; i < A.rows; i++)
    {
        for (int j = 0; j < A.cols; j++)
        {
            A.at<uchar>(i, j) = 0;
        }
    }
    

    cv::Mat B(500, 500, CV_8U);
    //checkerboard pattern
    for (int i = 0; i < B.rows; i++)
    {
        for (int j = 0; j < B.cols; j++)
        {
            if ((i / 50 + j / 50) % 2 == 0)
            {
                B.at<uchar>(i, j) = 0;
            }
            else
            {
                B.at<uchar>(i, j) = 255;
            }
        }
    }

    //gradient pattern
    cv::Mat C(500, 500, CV_8U);
    for (int i = 0; i < C.rows; i++)
    {
        for (int j = 0; j < C.cols; j++)
        {
            C.at<uchar>(j, i) = 255 - ((i + j) / 4);
        }
    }




    //save image to /images
    cv::imwrite("images/checkerboard.png", B);
    cv::imwrite("images/black_image.png", A);
    cv::imwrite("images/gradient.png", C);

    //Create a window and display the image
    cv::imshow("Black Image", A);
    cv::imshow("Checkerboard", B);
    cv::imshow("Gradient", C);

    // Wait for a keystroke in the window
    cv::waitKey(0);

    return 0;
}