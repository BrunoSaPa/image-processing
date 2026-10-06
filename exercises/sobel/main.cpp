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

    cout << "Image size: " << image.cols << "x" << image.rows << endl;

    //apply sobel operator to image
    //matrix for horizontal sobel
    float horizontalSobel[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    Mat SobX = Mat(3,3, CV_32F, horizontalSobel);
    //matrix for vertical sobel
    float verticalSobel[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    Mat SobY = Mat(3,3, CV_32F, verticalSobel);

    //apply the sobel operators
    Mat grad_x = Mat::zeros(image.size(), CV_32F);
    Mat grad_y = Mat::zeros(image.size(), CV_32F);

    //magnitude
    Mat imagefxi = Mat::zeros(image.size(), CV_8U);
    Mat imagefyi = Mat::zeros(image.size(), CV_8U);


    Mat imageSobel = Mat::zeros(image.size(), CV_32F);

    for(int i = 0; i < image.rows; i++)
    {
        for(int j = 0; j < image.cols; j++)
        {
            float sumX = 0.0f;
            float sumY = 0.0f;
            for(int m = -1; m <= 1; m++)
            {
                for(int n = -1; n <= 1; n++)
                {
                    const int row = i + m;
                    const int col = j + n;
                    if (row < 0 || row >= image.rows || col < 0 || col >= image.cols)
                    {
                        continue;
                    }

                    sumX += image.at<uchar>(row, col) * SobX.at<float>(m + 1, n + 1);
                    sumY += image.at<uchar>(row, col) * SobY.at<float>(m + 1, n + 1);


                }
            }
            grad_x.at<float>(i, j) = sumX;
            grad_y.at<float>(i, j) = sumY;

            imagefxi.at<uchar>(i, j) = abs(sumX);
            imagefyi.at<uchar>(i, j) = abs(sumY);

            float magnitude = sqrt(sumX * sumX + sumY * sumY);
            imageSobel.at<float>(i, j) = magnitude;
        }
    }

    cout << "Ended" << endl;

    //show the results
    imshow("Original Image", image);
    imshow("Sobel X", grad_x);
    imshow("Sobel Y", grad_y);
    imshow("Magnitude X", imagefxi);
    imshow("Magnitude Y", imagefyi);
    imshow("Sobel Magnitude", imageSobel);

    //wait 
    waitKey(0);

    return 0;

}