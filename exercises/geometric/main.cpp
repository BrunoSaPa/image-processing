#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;

int interpolation(int x, int y, Mat inputImage);

int main()
{

    Mat inputImage = imread("resources/mona_lisa.png", IMREAD_GRAYSCALE);

    if (!inputImage.data) {
        cout << "Could not open or find the image" << endl;
        return -1;
    }

    imshow("Original Image", inputImage);

    Mat imageTransformed = Mat::zeros(inputImage.size(), inputImage.type());

    float Tx, Ty;
    int u, v;

    cout << "Enter translation in x direction: ";
    cin >> Tx;
    cout << "Enter translation in y direction: ";
    cin >> Ty;  

    for(int y = 0; y < inputImage.rows; y++)
    {
        for(int x = 0; x < inputImage.cols; x++)
        {
            u = x + Tx;
            v = y + Ty;

            if(u >= 0 && u < inputImage.cols && v >= 0 && v < inputImage.rows)
            {
                imageTransformed.at<uchar>(v, u) = inputImage.at<uchar>(y, x);
            }
        }
    }

    imshow("Transformed Image", imageTransformed);

    //rotation
    float angle;
    cout << "Enter rotation angle (in degrees): ";
    cin >> angle;

    Mat imageRotated = Mat::zeros(inputImage.size(), inputImage.type());
    for(int y = 0; y < inputImage.rows; y++)
    {
        for(int x = 0; x < inputImage.cols; x++)
        {
            u = (x - inputImage.cols / 2) * cos(angle * CV_PI / 180) - (y - inputImage.rows / 2) * sin(angle * CV_PI / 180) + inputImage.cols / 2;
            v = (x - inputImage.cols / 2) * sin(angle * CV_PI / 180) + (y - inputImage.rows / 2) * cos(angle * CV_PI / 180) + inputImage.rows / 2;

            if(u >= 0 && u < inputImage.cols && v >= 0 && v < inputImage.rows)
            {
                imageRotated.at<uchar>(v, u) = inputImage.at<uchar>(y, x);
            }
        }
    }

    imshow("Rotated Image", imageRotated);

    //interpolation
    Mat imageInterpolated = Mat::zeros(inputImage.size(), inputImage.type());
    for(int y = 0; y < inputImage.rows - 1; y++)
    {
        for(int x = 0; x < inputImage.cols - 1; x++)
        {
            int interpolatedValue = interpolation(x, y, inputImage);
            imageInterpolated.at<uchar>(y, x) = interpolatedValue;
        }
    }

    imshow("Interpolated Image", imageInterpolated);

    waitKey(0);

    return 0;

}

int interpolation(int x, int y, Mat inputImage)
{
    float a = inputImage.at<uchar>(y, x);
    float b = inputImage.at<uchar>(y, x + 1);
    float c = inputImage.at<uchar>(y + 1, x);
    float d = inputImage.at<uchar>(y + 1, x + 1);

    // Perform bilinear interpolation
    float fx = x + 0.5;
    float fy = y + 0.5;
    float wx = fx - (int)fx;
    float wy = fy - (int)fy;

    float interpolatedValue = a * (1 - wx) * (1 - wy) + b * wx * (1 - wy) + c * (1 - wx) * wy + d * wx * wy;

    return (int)interpolatedValue;
}