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


    //histogram
    int count[256]={0};

    for (int i = 0; i < inputImage.rows; i++)
    {
        for (int j = 0; j < inputImage.cols; j++)
        {
            count[(int)inputImage.at<uchar>(i, j)]++;
        }
    }

    // Display the histogram values
    int sum = 0;
    for (int i = 0; i < 256; i++)
    {
        cout << i << ": " << count[i] << endl;
        sum += count[i];
    }
    cout << "Total pixels: " << sum << endl;
    cout << "Image size: " << inputImage.rows << " x " << inputImage.cols << " total pixels: " << inputImage.rows * inputImage.cols << endl;


    const int histWidth = 512;
    const int histHeight = 400;
    const int bins = 256;

    Mat histImage(histHeight, histWidth, CV_8U, Scalar(0));

    int maxCount = *std::max_element(count, count + bins);
    float binWidth = static_cast<float>(histWidth) / bins;

    for (int i = 0; i < bins; ++i)
    {
        int barHeight = (maxCount > 0)
            ? cvRound(static_cast<float>(count[i]) / maxCount * (histHeight - 1))
            : 0;

        int x1 = cvRound(i * binWidth);
        int x2 = cvRound((i + 1) * binWidth) - 1;
        int y1 = histHeight - 1;
        int y2 = histHeight - 1 - barHeight;

        rectangle(
            histImage,
            Point(x1, y1),
            Point(x2, y2),
            Scalar(255),
            FILLED
        );
    }


    imshow("Histogram", histImage);

    //calculate PMF from histogram
    float PMF[256] = {0.0};
    for (int i = 0; i < bins; ++i)
    {
        PMF[i] = count[i] / (float)(inputImage.rows * inputImage.cols);
    }

    //calculate CDF from PMF
    float CDF[256] = {0.0};
    CDF[0] = PMF[0];
    for (int i = 1; i < bins; ++i)
    {
        CDF[i] = CDF[i - 1] + PMF[i];
    }

    Mat newImage = inputImage.clone();

    //apply histogram equalization
    for (int i = 0; i < newImage.rows; ++i)
    {
        for (int j = 0; j < newImage.cols; ++j)
        {
            int pixelValue = (int)newImage.at<uchar>(i, j);
            newImage.at<uchar>(i, j) = (uchar)(CDF[pixelValue] * 255);
        }
    }

    imshow("Equalized image", newImage);


    //wait 
    waitKey(0);

    return 0;

}