//James Rogers Nov 2023 (c) Plymouth University

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <iostream>
#include <string>

#include "../owl.h"
#include "hsv_config.h"

using namespace std;
using namespace cv;

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

static const String kWinTitleRaw      = "left";
static const String kWinTitleFiltered = "left filtered";

static const int kMaxH = 360;
static const int kMaxSV = 1000;
static HSVConfig hsv{{0, kMaxH, 0, kMaxSV, 0, kMaxSV}};
static const char* kConfigFilepath = "hsv_conf.txt";

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

    hsv = loadConfig(kConfigFilepath);
    namedWindow(kWinTitleRaw);
    namedWindow(kWinTitleFiltered);
    createTrackbar("Low Hue",  kWinTitleRaw, &hsv.lh, kMaxH, onLowHueThreshTrackbar);
    createTrackbar("High Hue", kWinTitleRaw, &hsv.hh, kMaxH, onHighHueThreshTrackbar);
    createTrackbar("Low Sat",  kWinTitleRaw, &hsv.ls, kMaxSV, onLowSatThreshTrackbar);
    createTrackbar("High Sat", kWinTitleRaw, &hsv.hs, kMaxSV, onHighSatThreshTrackbar);
    createTrackbar("Low Val",  kWinTitleRaw, &hsv.lv, kMaxSV, onLowValThreshTrackbar);
    createTrackbar("High Val", kWinTitleRaw, &hsv.hv, kMaxSV, onHighValThreshTrackbar);

    bool running = true;
    while (running){
        //read the owls camera frames
        Mat left, right, hsvLeft, filteredLeft;
        owl.getCameraFrames(left, right);

        //your tracking code here
        cvtColor(left, hsvLeft, COLOR_BGR2HSV);
        inRange(hsvLeft, Scalar(hsv.lh, hsv.ls, hsv.lv), Scalar(hsv.hh, hsv.hs, hsv.hv), filteredLeft);

        Moments m = moments(filteredLeft, true);
        Point center(int(m.m10/m.m00), int(m.m01/m.m00));
        circle(left, center, 5, Scalar(128), -1);
        circle(filteredLeft, center, 5, Scalar(128), -1);

        if ((center.x - FRAME_WIDTH/2) > 10) {
            owl.
        }

        //display camera frame
        imshow(kWinTitleRaw, left);
        imshow(kWinTitleFiltered, filteredLeft);
        switch(waitKey(10)) {
        case 'q':
        case 27:
            running = false;
            break;
        case 's':
            saveConfig(kConfigFilepath, hsv);
            break;
        }
    }

}

static void onLowHueThreshTrackbar(int, void *)
{
    hsv.lh = min(hsv.hh-1, hsv.lh);
    setTrackbarPos("Low Hue", kWinTitleRaw, hsv.lh);
}
static void onHighHueThreshTrackbar(int, void *)
{
    hsv.hh = max(hsv.hh, hsv.lh+1);
    setTrackbarPos("High Hue", kWinTitleRaw, hsv.hh);
}
static void onLowSatThreshTrackbar(int, void *)
{
    hsv.ls = min(hsv.hs-1, hsv.ls);
    setTrackbarPos("Low Sat", kWinTitleRaw, hsv.ls);
}
static void onHighSatThreshTrackbar(int, void *)
{
    hsv.hs = max(hsv.hs, hsv.ls+1);
    setTrackbarPos("High Sat", kWinTitleRaw, hsv.hs);
}
static void onLowValThreshTrackbar(int, void *)
{
    hsv.lv = min(hsv.hv-1, hsv.lv);
    setTrackbarPos("Low Val", kWinTitleRaw, hsv.lv);
}
static void onHighValThreshTrackbar(int, void *)
{
    hsv.hv = max(hsv.hv, hsv.lv+1);
    setTrackbarPos("High Val", kWinTitleRaw, hsv.hv);
}
