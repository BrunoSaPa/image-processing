#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;

int main()
{


    //read image
    Mat image = imread("./resources/sobel.png", IMREAD_GRAYSCALE);
    if (image.empty())
    {
        cerr << "Could not read image" << endl;
        return 1;
    }

    //apply binary threshold to image

    for(int i = 0; i < image.rows; i++)
    {
        for(int j = 0; j < image.cols; j++)
        {
            if(image.at<uchar>(i, j) > 128)
            {
                image.at<uchar>(i, j) = 255;
            }
            else
            {
                image.at<uchar>(i, j) = 0;
            }
        }
    }

    int count = 0;

    //wait 
    waitKey(0);

    return 0;

}