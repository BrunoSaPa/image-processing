#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
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



    //horizontal shearing
    Mat horizontalSheeredImage = Mat::zeros(inputImage.size(), inputImage.type());
    float shx;
    cout << "Enter shearing factor in x direction: ";
    cin >> shx;
    for(int y = 0; y < inputImage.rows; y++)
    {
        for(int x = 0; x < inputImage.cols; x++)
        {
            //center coordinates
            int x_centered = x - inputImage.cols / 2;
            int y_centered = y - inputImage.rows / 2;

            //apply shearing
            int u = x_centered + shx * y_centered + inputImage.cols / 2;
            int v = y_centered + inputImage.rows / 2;

            //decenter coordinates
            u += inputImage.cols / 2;
            v += inputImage.rows / 2;

            if(u >= 0 && u < inputImage.cols && v >= 0 && v < inputImage.rows)
            {
                horizontalSheeredImage.at<uchar>(v, u) = inputImage.at<uchar>(y, x);
            }
        }
    }

    //linear transformation
    Mat linearTransformedImage = Mat::zeros(inputImage.size(), inputImage.type());
    float a, b, c, d;
    cout << "Enter linear transformation matrix (a, b, c, d): ";
    cin >> a >> b >> c >> d;
    for(int y = 0; y < inputImage.rows; y++)
    {
        for(int x = 0; x < inputImage.cols; x++)
        {
            u = a * x + b * y;
            v = c * x + d * y;

            if(u >= 0 && u < inputImage.cols && v >= 0 && v < inputImage.rows)
            {
                linearTransformedImage.at<uchar>(v, u) = inputImage.at<uchar>(y, x);
            }
        }
    }

    // Horizontal coordinate coefficients before conversion to integer pixels.
    const double centerX = inputImage.cols / 2;
    const double centerY = inputImage.rows / 2;
    const double radians = angle * CV_PI / 180;
    const double cosine = cos(radians);
    const double sine = sin(radians);

    const char* transformationNames[] = {
        "Translation", "Rotation", "Horizontal shear", "Linear transformation"
    };
    const double coefficients[4][4] = {
        {Tx, 1.0, 0.0, 0.0},
        {centerX * (1.0 - cosine) + centerY * sine, cosine, -sine, 0.0},
        // Includes the extra center offset in the current shearing code.
        {centerX - shx * centerY, 1.0, shx, 0.0},
        {0.0, a, b, 0.0}
    };

    cout << "\nHorizontal coefficients: u = a0 + a1*x + a2*y + a3*x*y\n";
    cout << left << setw(24) << "Transformation"
         << right << setw(14) << "a0" << setw(14) << "a1"
         << setw(14) << "a2" << setw(14) << "a3" << '\n';
    cout << string(80, '-') << '\n';
    cout << fixed << setprecision(4);
    for(int row = 0; row < 4; row++)
    {
        cout << left << setw(24) << transformationNames[row];
        for(int column = 0; column < 4; column++)
        {
            cout << right << setw(14) << coefficients[row][column];
        }
        cout << '\n';
    }
    cout << endl;

    imshow("Original Image", inputImage);
    imshow("Transformed Image", imageTransformed);
    imshow("Interpolated Image", imageInterpolated);
    imshow("Rotated Image", imageRotated);
    imshow("Horizontal Sheered Image", horizontalSheeredImage);
    imshow("Linear Transformed Image", linearTransformedImage);


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
