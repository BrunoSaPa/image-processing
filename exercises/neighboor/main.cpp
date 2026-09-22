#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

int main()
{
    Mat inputImage = imread("resources/tepito.png", IMREAD_GRAYSCALE);
    
    if (inputImage.empty())
    {
        cerr << "Could not open input image\n";
        return 1; 
    }

    imshow("Original image", inputImage);

    waitKey(0);

    Mat medianImage = inputImage.clone();
    for (int j = 1; j < inputImage.rows - 1; j++)
    {
        for (int i = 1; i < inputImage.cols - 1; i++)
        {
            uchar pixels[9];

            int pos = 0;
            for (int l = -1; l <= 1; l++)
            {
                for (int k = -1; k <= 1; k++)
                {
                    pixels[pos] =
                        inputImage.at<uchar>(
                            j + l,
                            i + k
                        );

                    pos++;
                }
            }


            // Sort the 9 values
            sort(pixels, pixels + 9);

            medianImage.at<uchar>(j, i) =
                pixels[4];
        }
    }

    imshow("Median Filter", medianImage);




    int j = inputImage.rows / 2;
    int i = inputImage.cols / 2;

    cout << endl;
    cout << "Center pixel: (" << j << "," << i << ")" << endl;
    cout << "Pixels behind the mask:" << endl;

    for (int l = -1; l <= 1; l++)
    {
        for (int k = -1; k <= 1; k++)
        {
            cout
                << "("
                << j + l
                << ","
                << i + k
                << ") = "
                << (int)inputImage.at<uchar>(
                    j + l,
                    i + k
                )
                << endl;
        }
    }


    //wait 
    waitKey(0);

    return 0;

}


