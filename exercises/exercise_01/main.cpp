#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <opencv2/opencv.hpp>

namespace
{
constexpr int imageSize = 500;

cv::Mat createCheckerboard(int numberOfSquares)
{
    cv::Mat image(imageSize, imageSize, CV_8U);
    const int squareSize = imageSize / numberOfSquares;

    for (int row = 0; row < image.rows; ++row)
    {
        for (int column = 0; column < image.cols; ++column)
        {
            const bool isBlack = ((row / squareSize) + (column / squareSize)) % 2 == 0;
            image.at<uchar>(row, column) = isBlack ? 0 : 255;
        }
    }

    return image;
}

cv::Mat createDiagonalGradient()
{
    cv::Mat image(imageSize, imageSize, CV_8U);

    for (int row = 0; row < image.rows; ++row)
    {
        for (int column = 0; column < image.cols; ++column)
        {
            image.at<uchar>(row, column) = static_cast<uchar>(
                255 - ((row + column) * 255 / (2 * (imageSize - 1))));
        }
    }

    return image;
}

cv::Mat createHorizontalGradient()
{
    cv::Mat image(imageSize, imageSize, CV_8U);

    for (int row = 0; row < image.rows; ++row)
    {
        for (int column = 0; column < image.cols; ++column)
        {
            // White on the left, black on the right.
            image.at<uchar>(row, column) = static_cast<uchar>(
                255 - (column * 255 / (imageSize - 1)));
        }
    }

    return image;
}

cv::Mat createHorizontalStripes()
{
    cv::Mat image(imageSize, imageSize, CV_8U);
    constexpr int stripeWidth = 5;

    for (int row = 0; row < image.rows; ++row)
    {
        const bool isBlack = (row / stripeWidth) % 2 == 0;
        image.row(row).setTo(isBlack ? 0 : 255);
    }

    return image;
}

cv::Mat createVerticalStripes()
{
    cv::Mat image(imageSize, imageSize, CV_8U);
    constexpr int stripeWidth = 5;

    for (int column = 0; column < image.cols; ++column)
    {
        const bool isBlack = (column / stripeWidth) % 2 == 0;
        image.col(column).setTo(isBlack ? 0 : 255);
    }

    return image;
}

cv::Mat createBullseye()
{
    cv::Mat image(imageSize, imageSize, CV_8U, cv::Scalar(255));
    const int outerMargin = 40;
    const int innerMargin = 120;

    cv::rectangle(
        image,
        cv::Point(outerMargin, outerMargin),
        cv::Point(imageSize - outerMargin - 1, imageSize - outerMargin - 1),
        cv::Scalar(0),
        cv::FILLED);
    cv::rectangle(
        image,
        cv::Point(innerMargin, innerMargin),
        cv::Point(imageSize - innerMargin - 1, imageSize - innerMargin - 1),
        cv::Scalar(255),
        cv::FILLED);

    return image;
}

void saveImage(const std::string& filename, const cv::Mat& image)
{
    if (!cv::imwrite("images/" + filename, image))
    {
        throw std::runtime_error("Could not write images/" + filename);
    }
}
}

int main()
{
    try
    {
        std::filesystem::create_directories("images");

        const cv::Mat blackImage(imageSize, imageSize, CV_8U, cv::Scalar(0));
        const cv::Mat checkerboard2x2 = createCheckerboard(2);
        const cv::Mat checkerboard4x4 = createCheckerboard(4);
        const cv::Mat diagonalGradient = createDiagonalGradient();
        const cv::Mat horizontalGradient = createHorizontalGradient();
        const cv::Mat horizontalStripes = createHorizontalStripes();
        const cv::Mat verticalStripes = createVerticalStripes();
        const cv::Mat bullseye = createBullseye();

        saveImage("black_image.png", blackImage);
        saveImage("checkerboard_2x2.png", checkerboard2x2);
        saveImage("checkerboard_4x4.png", checkerboard4x4);
        saveImage("gradient.png", diagonalGradient);
        saveImage("gradient_horizontal.png", horizontalGradient);
        saveImage("stripes_horizontal.png", horizontalStripes);
        saveImage("stripes_vertical.png", verticalStripes);
        saveImage("bullseye.png", bullseye);

        cv::imshow("Black image", blackImage);
        cv::imshow("Checkerboard 2x2", checkerboard2x2);
        cv::imshow("Checkerboard 4x4", checkerboard4x4);
        cv::imshow("Diagonal gradient", diagonalGradient);
        cv::imshow("Horizontal gradient", horizontalGradient);
        cv::imshow("Horizontal stripes", horizontalStripes);
        cv::imshow("Vertical stripes", verticalStripes);
        cv::imshow("Bullseye", bullseye);
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }

    // Wait for a keystroke in the window
    cv::waitKey(0);

    return 0;
}
