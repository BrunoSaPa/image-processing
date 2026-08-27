#include <iostream>
#include <opencv2/opencv.hpp>

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



    //wait 
    //waitKey(0);

    return 0;

}