#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;

int main()
{

    int maskSize = 9;
    Mat GassMask(maskSize,maskSize, CV_32F);
    int x0, y0, x, y;
    x0 = GassMask.cols / 2;
    y0 = GassMask.rows / 2;

    float Texp, tconst;
    float sigma = 2.0f;
    float totalM = 0.0f;

   

    tconst = 1.0f / (2 * 3.14159 * sigma * sigma);

    for (int i = 0; i < GassMask.rows; i++)
    {
        for (int j = 0; j < GassMask.cols; j++)
        {
            x = j - x0;
            y = y0 - i;
            Texp = -(x * x + y * y) / (2 * sigma * sigma);
            GassMask.at<float>(i, j) = tconst * exp(Texp);
            totalM += GassMask.at<float>(i, j);
        }
    }

    //print mask 
    cout << GassMask << endl;

    //apply mask to image
    Mat image = imread("./resources/pepper.png", IMREAD_GRAYSCALE);

    Mat result = Mat::zeros(image.size(), image.type());

    //use maskSize to apply mask to image

    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            float sum = 0.0f;
            for (int m = -maskSize/2; m <= maskSize/2; m++)
            {
                for (int n = -maskSize/2; n <= maskSize/2; n++)
                {
                    if (i + m >= 0 && i + m < image.rows && j + n >= 0 && j + n < image.cols)
                    {
                        sum += image.at<uchar>(i + m, j + n) * GassMask.at<float>(m + maskSize/2, n + maskSize/2);
                    }
                }
            }
            result.at<uchar>(i, j) = static_cast<uchar>(sum);
        }
    }

    imshow("Original Image", image);
    imshow("Gaussian Mask", result);

    //wait 
    waitKey(0);

    return 0;

}