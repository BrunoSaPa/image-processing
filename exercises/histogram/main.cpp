#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;

int main()
{
    Mat inputImage = imread("resources/pique.png", IMREAD_GRAYSCALE);
    
    if (inputImage.empty())
    {
        cerr << "Could not open input image\n";
        return 1; 
    }

    //imshow("Original image", inputImage);


    int count[256]={0};

    for (int i = 0; i < inputImage.rows; i++)
    {
        for (int j = 0; j < inputImage.cols; j++)
        {
            count[(int)inputImage.at<uchar>(i, j)]++;
        }
        cout << endl;
    }

    // Display the histogram
    int sum = 0;
    for (int i = 0; i < 256; i++)
    {
        cout << i << ": " << count[i] << endl;
        sum += count[i];
    }
    cout << "Total pixels: " << sum << endl;
    cout << "Image size: " << inputImage.rows << " x " << inputImage.cols << " total pixels: " << inputImage.rows * inputImage.cols << endl;



    //draw histogram with rectangles
    Mat histImage(400, 512, CV_8UC1, Scalar(255));


    //display histogram values in the image with rectangles
    for (int i = 0; i < 256; i++)
    {
        int height = count[i] / 1000; // Scale down for display purposes
        rectangle(histImage, Point(i * 2, 400), Point(i * 2 + 2, 400 - height), Scalar(0), FILLED);
    }


    imshow("Histogram", histImage);

    //wait 
    waitKey(0);

    return 0;

}