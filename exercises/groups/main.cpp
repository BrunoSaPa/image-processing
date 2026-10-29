#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;



bool growRQ(const Mat &imgori, Mat &res, int x0, int y0, int val, std::vector<Point2d> &regionPoints)
{
	if (x0 <0 || x0 >= (imgori.cols) || y0<0 || y0 >= (imgori.rows))
	{
		return true;
	}
	std::vector<Point2d> colaPts;
	Point2d pt1;
	colaPts.clear();
    if (regionPoints.size() > 0)
    {
        regionPoints.clear();
    }
	if (((int)imgori.at<uchar>(y0, x0)) == 255)
	{
		if (((int)res.at<uchar>(y0, x0)) == 0)
		{
			res.at<uchar >(y0, x0) = (uchar)val;

			for (int k = -1; k < 2; k++)
			{
				for (int l = -1; l <2; l++)
				{
					if (!(k==0 && l==0))
					if ((x0 + l) >= 0 && (x0 + l) < (imgori.cols) && (y0 + k) >= 0 && (y0 + k)  < (imgori.rows))
					{
						colaPts.push_back(Point2d(x0 + l, y0 + k));
					}
				}
			}
			while (colaPts.size()>0)
			{
				pt1 = colaPts[0];
				colaPts.erase( colaPts.begin());

				if (((int)imgori.at<uchar>(pt1.y, pt1.x)) == 255)
					if (((int)res.at<uchar>(pt1.y, pt1.x)) == 0)
					{
						res.at<uchar >(pt1.y, pt1.x) = (uchar)val;
						regionPoints.push_back(pt1);
						//introuduce coordenadas en vector

						namedWindow("avance", WINDOW_NORMAL);
						imshow("avance", res);
						//waitKey(10);

						for (int k = -1; k < 2; k++)
						{
							for (int l = -1; l < 2; l++)
							{
								if (!(k == 0 && l == 0))
								if ((pt1.x + l) >= 0 && (pt1.x + l) < (imgori.cols) && (pt1.y + k) >= 0 && (pt1.y + k) < (imgori.rows))
								{
								  if (((int)res.at<uchar>(pt1.y + k, pt1.x + l)) == 0)
										colaPts.push_back(Point2d(pt1.x + l, pt1.y + k));
								}
							}
						}
					}
				
				//sigue sacando de la cola
			}
			
			
	/*		namedWindow("avance", WINDOW_NORMAL);
			imshow("avance", res);
			waitKey(10);*/


		

		}

	}
	//printf("%d %d  \n",x0,y0);

	return false;

}

int main()
{
    // Load the input image in grayscale
    Mat inputImage = imread("resources/beans.png", IMREAD_GRAYSCALE);
    
    if (inputImage.empty())
    {
        cerr << "Could not open input image\n";
        return 1; 
    }

    imshow("Original image", inputImage);

    // Create a 3-channel color image for drawing boxes and centroids
    Mat colorOutput;
    cvtColor(inputImage, colorOutput, COLOR_GRAY2BGR);

    //apply inverse binary thresholding
    Mat binaryImage;
    double thresholdValue = 180;
    threshold(inputImage, binaryImage, thresholdValue, 255, THRESH_BINARY_INV);

    imshow("Binary image", binaryImage);

    // image to mark regions - FIXED to CV_8UC1 so .at<uchar> works safely
    Mat imageMarked(inputImage.size(), CV_8UC1, Scalar(0));

    //regions
    std::vector<std::vector<Point2d>> allRegions;
    int label1 = 100;
    
    //draw regions and bounding boxes
    for (int y = 0; y < binaryImage.rows; y++)
    {
        for (int x = 0; x < binaryImage.cols; x++)
        {
            if (binaryImage.at<uchar>(y, x) == 255 && imageMarked.at<uchar>(y, x) == 0)
            {
                std::cout << "Starting region growing at (" << x << ", " << y << ") with label " << label1 << std::endl;
                std::vector<Point2d> regionPoints;
                growRQ(binaryImage, imageMarked, x, y, label1, regionPoints);
                
                if (!regionPoints.empty())
                {
                    allRegions.push_back(regionPoints);
                    
                    double sumX = 0, sumY = 0;
                    double minX = regionPoints[0].x, maxX = regionPoints[0].x;
                    double minY = regionPoints[0].y, maxY = regionPoints[0].y;

                    for (const auto& point : regionPoints)
                    {
                        sumX += point.x;
                        sumY += point.y;
                        
                        // Find bounding box coordinates
                        if (point.x < minX) minX = point.x;
                        if (point.x > maxX) maxX = point.x;
                        if (point.y < minY) minY = point.y;
                        if (point.y > maxY) maxY = point.y;
                    }
                    
                    double centroidX = sumX / regionPoints.size();
                    double centroidY = sumY / regionPoints.size();
                    std::cout << "Centroid of the region: (" << centroidX << ", " << centroidY << ")" << std::endl;
                    
                    // Draw bounding box in Green (B=0, G=255, R=0)
                    rectangle(colorOutput, Point(minX, minY), Point(maxX, maxY), Scalar(0, 255, 0), 2);
                    
                    // Paint centroid in Red (B=0, G=0, R=255) using cv::circle
                    circle(colorOutput, Point(centroidX, centroidY), 4, Scalar(0, 0, 255), -1);
                }
                label1 += 10; // Increment label for the next region
            }
        }
    }

    imshow("Marked image regions", imageMarked);
    imshow("Bounding Boxes & Centroids", colorOutput);

    waitKey(0);
    return 0;
}