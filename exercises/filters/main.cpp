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

  

    //ask for mask size 
    int maskSize;
    cout << "Enter the mask size (odd number): ";
    cin >> maskSize;
    int halfMaskSize = maskSize / 2;

    //mean filter

    Mat meanImage = inputImage.clone();
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
            meanImage.at<uchar>(j, i) = sum / (maskSize * maskSize);
        }
    }

    imshow("Mean filtered image", meanImage);
   


    //median filter
    Mat medianImage = inputImage.clone();
    vector<int> myvec;

    for (int j = halfMaskSize; j < inputImage.rows - halfMaskSize; j++)
    {
        for (int i = halfMaskSize; i < inputImage.cols - halfMaskSize; i++)
        {
            myvec.clear();
            for (int l = -halfMaskSize; l <= halfMaskSize; l++)
            {
                for (int k = -halfMaskSize; k <= halfMaskSize; k++)
                {
                    myvec.push_back(inputImage.at<uchar>(j + l, i + k));
                }
            }
            sort(myvec.begin(), myvec.end());
            medianImage.at<uchar>(j, i) = myvec[myvec.size() / 2];
        }
    }

    imshow("Median filtered image", medianImage);

    //wait 
    waitKey(0);

    return 0;

}


