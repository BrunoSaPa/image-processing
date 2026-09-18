#include <opencv2/opencv.hpp>
#include <iostream>
 
using namespace cv;
using namespace std;
 
// ---------------------------------------------------------
// Function to normalize a matrix for visualization
// Converts the values to the range 0 - 255
// ---------------------------------------------------------
Mat normalizeForDisplay(const Mat& input)
{
    Mat output;
 
    normalize(input, output, 0, 255, NORM_MINMAX);
 
    output.convertTo(output, CV_8U);
 
    return output;
}
 
 
// ---------------------------------------------------------
// MAIN PROGRAM
// ---------------------------------------------------------
int main()
{
    // -----------------------------------------------------
    // 1. Read image
    // -----------------------------------------------------
 
    string filename;
 
    cout << "Enter image filename: ";
    cin >> filename;
 
    Mat image = imread("../../resources/" + filename);
 
    if (image.empty())
    {
        cout << "Error: Image could not be loaded." << endl;
        return -1;
    }
 
    imshow("Original Image", image);
 
 
    // -----------------------------------------------------
    // 2. Convert image to grayscale
    // -----------------------------------------------------
 
    Mat gray;
 
    cvtColor(image, gray, COLOR_BGR2GRAY);
 
    imshow("Grayscale Image", gray);
 
 
    // -----------------------------------------------------
    // 3. Convert grayscale image to floating point
    // -----------------------------------------------------
 
    Mat grayFloat;
 
    gray.convertTo(grayFloat, CV_32F);
 
 
    // -----------------------------------------------------
    // 4. Calculate image gradients
    //
    // Ix = horizontal derivative
    // Iy = vertical derivative
    // -----------------------------------------------------
 
    Mat Ix;
    Mat Iy;
 
    Sobel(grayFloat, Ix, CV_32F, 1, 0, 3);
    Sobel(grayFloat, Iy, CV_32F, 0, 1, 3);
 
    Mat IxDisplay = normalizeForDisplay(Ix);
    Mat IyDisplay = normalizeForDisplay(Iy);
 
    imshow("Ix - Horizontal Gradient", IxDisplay);
    imshow("Iy - Vertical Gradient", IyDisplay);
 
 
    // -----------------------------------------------------
    // 5. Calculate gradient products
    //
    // Ix^2
    // Iy^2
    // IxIy
    // -----------------------------------------------------
 
    Mat Ix2;
    Mat Iy2;
    Mat IxIy;
 
    multiply(Ix, Ix, Ix2);
    multiply(Iy, Iy, Iy2);
    multiply(Ix, Iy, IxIy);
 
    imshow("Ix squared", normalizeForDisplay(Ix2));
    imshow("Iy squared", normalizeForDisplay(Iy2));
    imshow("Ix Iy", normalizeForDisplay(IxIy));
 
 
    // -----------------------------------------------------
    // 6. Apply Gaussian smoothing
    //
    // These matrices represent the sums of gradient
    // information in a local neighborhood.
    // -----------------------------------------------------
 
    Mat Sxx;
    Mat Syy;
    Mat Sxy;
 
    GaussianBlur(Ix2, Sxx, Size(5, 5), 1.0);
    GaussianBlur(Iy2, Syy, Size(5, 5), 1.0);
    GaussianBlur(IxIy, Sxy, Size(5, 5), 1.0);
 
    imshow("Smoothed Ix squared", normalizeForDisplay(Sxx));
    imshow("Smoothed Iy squared", normalizeForDisplay(Syy));
    imshow("Smoothed Ix Iy", normalizeForDisplay(Sxy));
 
 
    // -----------------------------------------------------
    // 7. Calculate Harris response
    //
    // For every pixel:
    //
    //       | Sxx  Sxy |
    // M  =  |          |
    //       | Sxy  Syy |
    //
    // Harris response:
    //
    // R = det(M) - k * trace(M)^2
    //
    // det(M)   = Sxx*Syy - Sxy^2
    //
    // trace(M) = Sxx + Syy
    // -----------------------------------------------------
 
    Mat R = Mat::zeros(gray.size(), CV_32F);
 
    float k = 0.04f;
 
    for (int y = 0; y < gray.rows; y++)
    {
        for (int x = 0; x < gray.cols; x++)
        {
            float a = Sxx.at<float>(y, x);
            float b = Sxy.at<float>(y, x);
            float c = Syy.at<float>(y, x);
 
            // Determinant of M
            float determinant = (a * c) - (b * b);
 
            // Trace of M
            float trace = a + c;
 
            // Harris response
            R.at<float>(y, x) =
                determinant - k * trace * trace;
        }
    }
 
 
    // -----------------------------------------------------
    // 8. Display Harris response
    // -----------------------------------------------------
 
    Mat RDisplay = normalizeForDisplay(R);
 
    imshow("Harris Response", RDisplay);
 
 
    // -----------------------------------------------------
    // 9. Find maximum Harris response
    //
    // The maximum value is used to define the threshold.
    // -----------------------------------------------------
 
    double minR;
    double maxR;
 
    minMaxLoc(R, &minR, &maxR);
 
    cout << endl;
    cout << "Minimum Harris response: " << minR << endl;
    cout << "Maximum Harris response: " << maxR << endl;
 
 
    // -----------------------------------------------------
    // 10. Define threshold
    //
    // A pixel is considered a corner if:
    //
    // R > threshold * maximum response
    // -----------------------------------------------------
 
    float threshold = 0.01f;
 
    float cornerThreshold =
        threshold * static_cast<float>(maxR);
 
 
    // -----------------------------------------------------
    // 11. Detect corners
    // -----------------------------------------------------
 
    Mat result = image.clone();
 
    int numberCorners = 0;
 
    for (int y = 0; y < R.rows; y++)
    {
        for (int x = 0; x < R.cols; x++)
        {
            if (R.at<float>(y, x) > cornerThreshold)
            {
                circle(
                    result,
                    Point(x, y),
                    3,
                    Scalar(0, 0, 255),
                    1
                );
 
                numberCorners++;
            }
        }
    }
 
 
    // -----------------------------------------------------
    // 12. Display results
    // -----------------------------------------------------
 
    cout << "Threshold: " << cornerThreshold << endl;
    cout << "Detected corner pixels: "
        << numberCorners << endl;
 
    imshow("Detected Harris Corners", result);
 
 
    // -----------------------------------------------------
    // 13. Save important results
    // -----------------------------------------------------
 
    imwrite("../../images/01_grayscale.jpg", gray);
    imwrite("../../images/02_Ix.jpg", IxDisplay);
    imwrite("../../images/03_Iy.jpg", IyDisplay);
    imwrite("../../images/04_Ix_squared.jpg",
        normalizeForDisplay(Ix2));
    imwrite("../../images/05_Iy_squared.jpg",
        normalizeForDisplay(Iy2));
    imwrite("../../images/06_IxIy.jpg",
        normalizeForDisplay(IxIy));
    imwrite("../../images/07_Harris_Response.jpg",
        RDisplay);
    imwrite("../../images/08_Harris_Corners.jpg",
        result);
 
 
    cout << endl;
    cout << "Images saved." << endl;
    cout << "Press any key to finish." << endl;
 
    waitKey(0);
 
    destroyAllWindows();
 
    return 0;
}