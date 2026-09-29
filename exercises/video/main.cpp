#include <opencv2/videoio/videoio.hpp>

using namespace std;
using namespace cv;

int main()
{
    // VideoCapture cap(0); // open the default camera
    // if (!cap.isOpened()) // check if we succeeded
    // {
    //     cerr << "Could not open camera\n";
    //     return 1;
    // }

    // Mat frame;
    // while (true)
    // {
    //     cap >> frame; // get a new frame from camera
    //     if (frame.empty())
    //     {
    //         cerr << "Could not read frame\n";
    //         break;
    //     }

    //     imshow("Camera Feed", frame);
    //     if (waitKey(30) >= 0) break; // exit on any key press
    // }

    // // cap.release();
    // // destroyAllWindows();
    // return 0;
}