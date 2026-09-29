#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;

int main()
{
    Mat inputImage = imread("resources/dices.png", IMREAD_GRAYSCALE);
    Mat inputImage2 = imread("resources/falcon.png", IMREAD_GRAYSCALE);
    
    if (inputImage.empty() || inputImage2.empty())
    {
        cerr << "Could not open input images\n";
        return 1; 
    }
    //clamp input image to same size as 1
    resize(inputImage2, inputImage2, inputImage.size());


    imshow("Original image", inputImage);

    imshow("Original image 2", inputImage2);

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
    Mat B(inputImage.rows, inputImage.cols, CV_8U);

    //paste image 2 onto the "mask" A of the first image 
    for (int i = 0; i < A.rows; i++)
    {
        for (int j = 0; j < A.cols; j++)
        {
            if (A.at<uchar>(i, j) == 255)
            {
                B.at<uchar>(i, j) = inputImage2.at<uchar>(i, j);
            }
            else
            {
                B.at<uchar>(i, j) = inputImage.at<uchar>(i, j);
            }
        }
    }

    imshow("Masked", B);

    Mat C(inputImage.rows, inputImage.cols, CV_8U);

    for(int i = 0; i < A.rows; i++)
    {
        for (int j = 0; j < A.cols; j++)
        {
            if (A.at<uchar>(i, j) == 255)
            {
                C.at<uchar>(i, j) = inputImage.at<uchar>(i, j);
            }
            else
            {
                C.at<uchar>(i, j) = inputImage2.at<uchar>(i, j);
            }
        }
    }

    imshow("Masked 2", C);

    

    //wait 
    waitKey(0);

    return 0;

}