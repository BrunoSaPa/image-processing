#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main()
{

    Mat inputImage1 = cv::imread("resources/pique.png", IMREAD_GRAYSCALE);
    Mat inputImage2 = cv::imread("resources/ronaldo_sad.png", IMREAD_GRAYSCALE);

    if (inputImage1.empty() || inputImage2.empty())
    {
        cerr << "Could not open images\n";
        return 1;
    }

    //Display original
    imshow("Original image 1", inputImage1);
    imshow("Original image 2", inputImage2);

    //new image
    Mat mergedImage(inputImage1.rows, inputImage1.cols, CV_8U);

    // Sum the pixels after resizing the second image to match the first.
    resize(inputImage2, inputImage2, inputImage1.size());

    for (int i = 0; i < inputImage1.rows; i++)
    {
        for (int j = 0; j < inputImage1.cols; j++)
        {
            mergedImage.at<uchar>(i, j) = (inputImage1.at<uchar>(i, j) + inputImage2.at<uchar>(i, j))/2;
        }
    }
    imshow("Merged image", mergedImage);


    Mat addCte(inputImage1.rows, inputImage1.cols, CV_8U);
    int value = 50,tmp;

    for (int i = 0; i < inputImage1.rows; i++)
    {
        for (int j = 0; j < inputImage1.cols; j++)
        {
            tmp = inputImage1.at<uchar>(i, j) + value;
            if(tmp > 255)
                tmp = 255;
            addCte.at<uchar>(i, j) = tmp;
        }
    }

    imshow("Add constant", addCte);


    //multipy by collumn from left to right
    Mat multCol(inputImage1.rows, inputImage1.cols, CV_8U);
    for (int i = 0; i < inputImage1.rows; i++)
    {
        for (int j = 0; j < inputImage1.cols; j++)
        {
            tmp = inputImage1.at<uchar>(i, j) + (j+255)/multCol.cols;
            if(tmp < 0)
                tmp = 0;
            multCol.at<uchar>(i, j) = tmp;
        }
    }
    imshow("Multiply by column", multCol);

    //add two images with different contributions
    Mat addImages(inputImage1.rows, inputImage1.cols, CV_8U);
    double alpha = 0.5, beta = 0.5;
    for (int i = 0; i < inputImage1.rows; i++)
    {
        for (int j = 0; j < inputImage1.cols; j++)
        {
            tmp = alpha * inputImage1.at<uchar>(i, j) + beta * inputImage2.at<uchar>(i, j);
            if(tmp < 0)
                tmp = 0;
            if(tmp > 255)
                tmp = 255;
            addImages.at<uchar>(i, j) = tmp;
        }
    }
    imshow("Add images", addImages);


    //change weight from left to right, far left is 100% image1, far right is 100% image2
    Mat addImages2(inputImage1.rows, inputImage1.cols, CV_8U);
    for (int i = 0; i < inputImage1.rows; i++)
    {
        for (int j = 0; j < inputImage1.cols; j++)
        {
            alpha = (double)j/(addImages2.cols-1);
            beta = 1 - alpha;
            tmp = alpha * inputImage1.at<uchar>(i, j) + beta * inputImage2.at<uchar>(i, j);
            if(tmp < 0)
                tmp = 0;
            if(tmp > 255)
                tmp = 255;
            addImages2.at<uchar>(i, j) = tmp;
        }
    }
    imshow("Add images with changing weight", addImages2);

    //wait for a keystroke in the window
    waitKey(0);

}