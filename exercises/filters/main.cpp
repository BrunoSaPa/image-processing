#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

int main()
{
    Mat inputImage = imread("resources/pepper.png", IMREAD_GRAYSCALE);

    if (inputImage.empty())
    {
        cerr << "Could not open input image\n";
        return 1;
    }

    imshow("Original image", inputImage);



    //mean filter 3 by 3
    Mat imageA = inputImage.clone();

    int maskSize = 3;
    int halfMaskSize = maskSize / 2;

    for (int j = halfMaskSize; j < inputImage.rows - halfMaskSize; j++)
    {
        for (int i = halfMaskSize; i < inputImage.cols - halfMaskSize; i++)
        {
            int sum = 0;

            for (int l = -halfMaskSize; l <= halfMaskSize; l++)
            {
                for (int k = -halfMaskSize; k <= halfMaskSize; k++)
                {
                    sum += inputImage.at<uchar>(j + l, i + k);
                }
            }

            imageA.at<uchar>(j, i) = sum / 9;
        }
    }

    imshow("A - Mean 3x3", imageA);


    //gaussian filter 3 by 3

    Mat imageB = inputImage.clone();
    int data3x3[] = {
            1, 2, 1,
            2, 4, 2,
            1, 2, 1
        };
    Mat gaussian3x3 = Mat(3, 3, CV_32S, data3x3).clone();

    for (int j = 1; j < inputImage.rows - 1; j++)
    {
        for (int i = 1; i < inputImage.cols - 1; i++)
        {
            int sum = 0;

            for (int l = -1; l <= 1; l++)
            {
                for (int k = -1; k <= 1; k++)
                {
                    sum += inputImage.at<uchar>(j + l, i + k)* gaussian3x3.at<int>(l + 1, k + 1);
                }
            }

            imageB.at<uchar>(j, i) = sum / 16;
        }
    }

    imshow("B - Gaussian 3x3", imageB);




    //difference between A and B

    Mat differenceAB = Mat::zeros(inputImage.size(), CV_8U);

    for (int j = 0; j < inputImage.rows; j++)
    {
        for (int i = 0; i < inputImage.cols; i++)
        {
            int valueA = imageA.at<uchar>(j, i);
            int valueB = imageB.at<uchar>(j, i);

            differenceAB.at<uchar>(j, i) =
                abs(valueA - valueB);
        }
    }

    imshow("Difference A-B", differenceAB);


    //mean filter 5 by 5

    Mat imageC = inputImage.clone();

    maskSize = 5;
    halfMaskSize = maskSize / 2;

    for (int j = halfMaskSize; j < inputImage.rows - halfMaskSize; j++)
    {
        for (int i = halfMaskSize; i < inputImage.cols - halfMaskSize; i++)
        {
            int sum = 0;

            for (int l = -halfMaskSize; l <= halfMaskSize; l++)
            {
                for (int k = -halfMaskSize; k <= halfMaskSize; k++)
                {
                    sum += inputImage.at<uchar>(j + l, i + k);
                }
            }

            imageC.at<uchar>(j, i) = sum / 25;
        }
    }

    imshow("C - Mean 5x5", imageC);



    //gaussian filter 5 by 5 option 1

    Mat imageD = inputImage.clone();

    int data5x5_op1[] = {
            1,  4,  6,  4, 1,
            4, 16, 24, 16, 4,
            6, 24, 36, 24, 6,
            4, 16, 24, 16, 4,
            1,  4,  6,  4, 1
        };
    Mat gaussian5x5_op1 = Mat(5, 5, CV_32S, data5x5_op1).clone();

    for (int j = 2; j < inputImage.rows - 2; j++)
    {
        for (int i = 2; i < inputImage.cols - 2; i++)
        {
            int sum = 0;

            for (int l = -2; l <= 2; l++)
            {
                for (int k = -2; k <= 2; k++)
                {
                    sum += inputImage.at<uchar>(j + l, i + k)
                         * gaussian5x5_op1.at<int>(l + 2, k + 2);
                }
            }

            imageD.at<uchar>(j, i) = sum / 256;
        }
    }

    imshow("D - Gaussian 5x5 Op1", imageD);



    //gaussian filter 5 by 5 option 2

    Mat imageE = inputImage.clone();

    int data5x5_op2[] = {
            2,  4,  5,  4, 2,
            4,  9, 12,  9, 4,
            5, 12, 15, 12, 5,
            4,  9, 12,  9, 4,
            2,  4,  5,  4, 2
        };
    Mat gaussian5x5_op2 = Mat(5, 5, CV_32S, data5x5_op2).clone();

    for (int j = 2; j < inputImage.rows - 2; j++)
    {
        for (int i = 2; i < inputImage.cols - 2; i++)
        {
            int sum = 0;

            for (int l = -2; l <= 2; l++)
            {
                for (int k = -2; k <= 2; k++)
                {
                    sum += inputImage.at<uchar>(j + l, i + k)
                         * gaussian5x5_op2.at<int>(l + 2, k + 2);
                }
            }

            imageE.at<uchar>(j, i) = sum / 159;
        }
    }

    imshow("E - Gaussian 5x5 Op2", imageE);


    //difference between C and D

    Mat differenceCD = Mat::zeros(inputImage.size(), CV_8U);

    for (int j = 0; j < inputImage.rows; j++)
    {
        for (int i = 0; i < inputImage.cols; i++)
        {
            int valueC = imageC.at<uchar>(j, i);
            int valueD = imageD.at<uchar>(j, i);

            differenceCD.at<uchar>(j, i) =
                abs(valueC - valueD);
        }
    }

    imshow("Difference C-D", differenceCD);



    //difference between D and E

    Mat differenceDE = Mat::zeros(inputImage.size(), CV_8U);

    for (int j = 0; j < inputImage.rows; j++)
    {
        for (int i = 0; i < inputImage.cols; i++)
        {
            int valueD = imageD.at<uchar>(j, i);
            int valueE = imageE.at<uchar>(j, i);

            differenceDE.at<uchar>(j, i) =
                abs(valueD - valueE);
        }
    }

    imshow("Difference D-E", differenceDE);


    waitKey(0);

    return 0;
}