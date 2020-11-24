#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <zbar.h>
#include <iostream>
#include <fstream>
#include <string>

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
    Mat frame;
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
    }

    int detectQR()
    {
        // Converting frame to grayscale for better output
        Mat gray_frame;

        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
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

    void stop()
    {
        destroyAllWindows();
        cap.release();
    }
};

int main()
{
    // Initializing Object
    CloudCam cc{0, 0};

    // Initializing opencv window
    namedWindow("Output", WINDOW_NORMAL);
    resizeWindow("Output", 640, 480);
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

            break;
        }

        imshow("Output", cc.frame);
        waitKey(30);
    }
    // Release camera
    cc.stop();
}