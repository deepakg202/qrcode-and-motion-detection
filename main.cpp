#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <zbar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
using namespace std;
using namespace cv;
using namespace zbar;

class CloudCam
{
private:
    // Create video Capture Object
    VideoCapture cap;
    // Create zbar scanner
    ImageScanner scanner;

public:
    Mat frame, reference_frame, gray_frame, frame_delta;
    Image zbar_image;
    CloudCam(int deviceID, int apiID)
    {
        // Initializing Program Here

        // set resolution of camera
        cap.set(CAP_PROP_FRAME_WIDTH, 640);
        cap.set(CAP_PROP_FRAME_HEIGHT, 480);
        // Configuring zbar scanner
        scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

        // Open the primary camera
        cap.open(deviceID, apiID);
        if (!cap.isOpened())
        {
            cerr << "Could not open camera." << endl;
            exit(EXIT_FAILURE);
        }
    }
    void captureFrame()
    {
        // read camera output in frame
        cap >> frame;
        // converts frame to grayscale and stores it in gray_frame
        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
    }

    int detectQR()
    {
        // Converting frame to grayscale for better output

        int width = gray_frame.cols;
        int height = gray_frame.rows;
        uchar *raw = (uchar *)(gray_frame.data);

        // Adjust the zbar_image properties for the captured frame
        zbar_image.set_size(width, height);
        zbar_image.set_data(raw, width * height);
        zbar_image.set_format("Y800");

        cout << scanner.scan(zbar_image);

        return scanner.scan(zbar_image);
    }

    void writeToFile(const string filename, const string data)
    {
        cout << "Writing QR code contents to file: " << filename << endl;
        ofstream myfile(filename);
        if (myfile.is_open())
        {
            myfile << data;
            myfile.close();
        }
        else
            cout << "Unable to write contents to file";
    }

    void captureReferenceFrame()
    {
        // Initializing reference frame to detect motion using backgroound subrtraction method
        cap >> reference_frame;
        cvtColor(reference_frame, reference_frame, COLOR_BGR2GRAY);
        // Used to smooth the edges to reduce noise
        GaussianBlur(reference_frame, reference_frame, Size(21, 21), 0);
    }

    void detectMotion()
    {
        vector<vector<Point>> cnts;

        GaussianBlur(gray_frame, gray_frame, Size(21, 21), 0);

        // compute difference between first frame and current frame
        absdiff(reference_frame, gray_frame, frame_delta);

        // converts pixels above 25 to white
        threshold(frame_delta, frame_delta, 25, 255, THRESH_BINARY);

        dilate(frame_delta, frame_delta, Mat(), Point(-1, -1), 2);

        findContours(frame_delta, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        for (int i = 0; i < cnts.size(); i++)
        {

            if (contourArea(cnts[i]) < 500)
            {
                continue;
            }
            drawContours(frame, cnts, i, Scalar(0, 255, 0), 2);

            cout << "Motion Detected" << endl;
            putText(frame, "Motion Detected", Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
        }
    }

    void stop()
    {
        destroyAllWindows();
        cap.release();
        exit(0);
    }
};

int main()
{
    // Initializing Object
    CloudCam cc{0, 0};

    // Initializing opencv window
    namedWindow("Output", WINDOW_NORMAL);
    resizeWindow("Output", 240, 480);
    while (true)
    {
        cc.captureFrame();

        // if qr code is detected
        if (cc.detectQR() == 1)
        {
            cout << "QR Code Detected with Data: " << endl;

            // getting the first qr code data
            Image::SymbolIterator symbol = cc.zbar_image.symbol_begin();
            string data = symbol->get_data();
            cout << data << endl;
            cc.writeToFile("wpa_supplicant.conf", data);

            // check validation of qr code. if true then break the loop
            if (true)
                break;
        }

        imshow("Output", cc.frame);
        if (waitKey(1) == 27)
        {
            cc.stop(); // Exit if ESC is pressed
        }
    }

    // Detect motion after qr code validation
    namedWindow("Bgr Difference", WINDOW_NORMAL);
    resizeWindow("Bgr Difference", 240, 480);
    while (true)
    {
        cc.captureReferenceFrame();
        cc.captureFrame();
        cc.detectMotion();

        // This should run after some seconds

        imshow("Output", cc.frame);
        imshow("Bgr Difference", cc.frame_delta);

        if (waitKey(1) == 27)
        {
            cc.stop(); // Exit if ESC is pressed
        }
    }
    // Release camera and destory all windows
    cc.stop();
}