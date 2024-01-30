//James Rogers Nov 2023 (c) Plymouth University

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <iostream>
#include <string>

#include "../owl.h"

using namespace std;
using namespace cv;

static const String kWinTitleRaw      = "left";
static const String kWinTitleFiltered = "left filtered";

static const int kMaxH = 360;
static const int kMaxSV = 1000;
static int lowHue = 0, lowSat = 0, lowVal = 0;
static int highHue = kMaxH, highSat = kMaxSV, highVal = kMaxSV;

static void onLowHueThreshTrackbar(int, void *);
static void onHighHueThreshTrackbar(int, void *);
static void onLowSatThreshTrackbar(int, void *);
static void onHighSatThreshTrackbar(int, void *);
static void onLowValThreshTrackbar(int, void *);
static void onHighValThreshTrackbar(int, void *);

int main()
{
    //connect with the owl and load calibration values
    robotOwl owl(1500, 1475, 1520, 1525, 1520);

    namedWindow(kWinTitleRaw);
    namedWindow(kWinTitleFiltered);

    createTrackbar("Low H",  kWinTitleRaw, &lowHue, kMaxH, onLowHueThreshTrackbar);
    createTrackbar("High H", kWinTitleRaw, &highHue, kMaxH, onHighHueThreshTrackbar);
    createTrackbar("Low S",  kWinTitleRaw, &lowSat, kMaxSV, onLowSatThreshTrackbar);
    createTrackbar("High S", kWinTitleRaw, &highSat, kMaxSV, onHighSatThreshTrackbar);
    createTrackbar("Low V",  kWinTitleRaw, &lowVal, kMaxSV, onLowValThreshTrackbar);
    createTrackbar("High V", kWinTitleRaw, &highVal, kMaxSV, onHighValThreshTrackbar);

    bool running = true;
    while (running){
        //read the owls camera frames
        Mat left, right, hsvLeft, filteredLeft;
        owl.getCameraFrames(left, right);

        //your tracking code here
        //left *= 1./255;
        cvtColor(left, hsvLeft, COLOR_BGR2HSV);
        inRange(hsvLeft, Scalar(lowHue, lowSat, lowVal), Scalar(highHue, highSat, highVal), filteredLeft);

        //display camera frame
        imshow(kWinTitleRaw, left);
        imshow(kWinTitleFiltered, filteredLeft);
        //imshow("left hsv", )
        switch(waitKey(10)) {
            case 'q':
            case 27:
                running = false;
                break;
        }
    }
}

static void onLowHueThreshTrackbar(int, void *)
{
    lowHue = min(highHue-1, lowHue);
    setTrackbarPos("Low H", kWinTitleRaw, lowHue);
}
static void onHighHueThreshTrackbar(int, void *)
{
    highHue = max(highHue, lowHue+1);
    setTrackbarPos("High H", kWinTitleRaw, highHue);
}
static void onLowSatThreshTrackbar(int, void *)
{
    lowSat = min(highSat-1, lowSat);
    setTrackbarPos("Low S", kWinTitleRaw, lowSat);
}
static void onHighSatThreshTrackbar(int, void *)
{
    highSat = max(highSat, lowSat+1);
    setTrackbarPos("High S", kWinTitleRaw, highSat);
}
static void onLowValThreshTrackbar(int, void *)
{
    lowVal = min(highVal-1, lowVal);
    setTrackbarPos("Low V", kWinTitleRaw, lowVal);
}
static void onHighValThreshTrackbar(int, void *)
{
    highVal = max(highVal, lowVal+1);
    setTrackbarPos("High V", kWinTitleRaw, highVal);
}
