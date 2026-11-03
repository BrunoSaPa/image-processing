#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

//erode function
void erode(const Mat& input, Mat& output, const Mat& se)
{
        for (int y = 1; y < input.rows - 1; y++)
    {
        for (int x = 1; x < input.cols - 1; x++)
        {
            bool shouldErode = true;
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    if (se.at<uchar>(j + 1, i + 1) == 0)
                    {
                        continue;
                    }
                    {
                        if (input.at<uchar>(y + j, x + i) == 0)
                        {
                            shouldErode = false;
                            break;
                        }
                    }
                }
                if (!shouldErode)
                {
                    break;
                }
            }
            if (shouldErode)
            {
                output.at<uchar>(y, x) = 255;
            }
        }
    }
}


//dilate function 
void dilate(const Mat& input, Mat& output, const Mat& se)
{
    for (int y = 1; y < input.rows - 1; y++)
    {
        for (int x = 1; x < input.cols - 1; x++)
        {
            bool shouldDilatate = false;
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    if (se.at<uchar>(j + 1, i + 1) == 0)
                    {
                        continue;
                    }
                    {
                        if (input.at<uchar>(y + j, x + i) == 255)
                        {
                            shouldDilatate = true;
                            break;
                        }
                    }
                }
                if (shouldDilatate)
                {
                    break;
                }
            }
            if (shouldDilatate)
            {
                output.at<uchar>(y, x) = 255;
            }
        }
    }
}


int main()
{
    Mat inputImage = imread("resources/beans.png", IMREAD_GRAYSCALE);

    if (inputImage.empty())
    {
        cerr << "Could not open input image\n";
        return 1; 
    }

    imshow("Original image", inputImage);

    Mat binaryImage(inputImage.rows, inputImage.cols, CV_8UC1);
    double thresholdValue = 110;
    for (int y = 0; y < inputImage.rows; y++)
    {
        for (int x = 0; x < inputImage.cols; x++)
        {
            if (inputImage.at<uchar>(y, x) > thresholdValue)
            {
                binaryImage.at<uchar>(y, x) = 0; // Set to black
            }
            else
            {
                binaryImage.at<uchar>(y, x) = 255; // Set to white
            }
        }
    }

    imshow("Binary image", binaryImage);

    Mat SE = (Mat_<uchar>({3, 3}, {0, 255, 0, 
                                   255, 255, 255, 
                                   0, 255, 0}));

    Mat imageEroded(inputImage.rows, inputImage.cols, CV_8UC1, Scalar(0));

    erode(binaryImage, imageEroded, SE);

    imshow("Eroded image", imageEroded);

    Mat imageDilatated(inputImage.rows, inputImage.cols, CV_8UC1, Scalar(0));

    dilate(binaryImage, imageDilatated, SE);

    imshow("Dilatated image", imageDilatated);


    //open 
    Mat imageOpened(inputImage.rows, inputImage.cols, CV_8UC1, Scalar(0));
    erode(binaryImage, imageOpened, SE);
    Mat imageOpenedTemp(inputImage.rows, inputImage.cols, CV_8UC1, Scalar(0));
    dilate(imageOpened, imageOpenedTemp, SE);      
    imshow("Opened image", imageOpenedTemp);

    //close
    Mat imageClosed(inputImage.rows, inputImage.cols, CV_8UC1, Scalar(0));
    dilate(binaryImage, imageClosed, SE);
    Mat imageClosedTemp(inputImage.rows, inputImage.cols, CV_8UC1, Scalar(0));
    erode(imageClosed, imageClosedTemp, SE);
    imshow("Closed image", imageClosedTemp);

    waitKey(0);

    return 0;

}