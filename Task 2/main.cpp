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

static const String kWinTitleRaw      = "left";
static const String kWinTitleFiltered = "left filtered";

static const int kMaxH = 360;
static const int kMaxSV = 1000;
static HSVConfig hsv{{0, 0, 0, 0, 0, 0}};

static void onlhThreshTrackbar(int, void *);
static void onhhThreshTrackbar(int, void *);
static void onlsThreshTrackbar(int, void *);
static void onhsThreshTrackbar(int, void *);
static void onlvThreshTrackbar(int, void *);
static void onhvThreshTrackbar(int, void *);

int main()
{
    hsv = loadConfig("test.txt");

    //connect with the owl and load calibration values
    robotOwl owl(1500, 1475, 1520, 1525, 1520);

    namedWindow(kWinTitleRaw);
    namedWindow(kWinTitleFiltered);

    createTrackbar("Low H",  kWinTitleRaw, &hsv.lh, kMaxH, onlhThreshTrackbar);
    createTrackbar("High H", kWinTitleRaw, &hsv.hh, kMaxH, onhhThreshTrackbar);
    createTrackbar("Low S",  kWinTitleRaw, &hsv.ls, kMaxSV, onlsThreshTrackbar);
    createTrackbar("High S", kWinTitleRaw, &hsv.hs, kMaxSV, onhsThreshTrackbar);
    createTrackbar("Low V",  kWinTitleRaw, &hsv.lv, kMaxSV, onlvThreshTrackbar);
    createTrackbar("High V", kWinTitleRaw, &hsv.hv, kMaxSV, onhvThreshTrackbar);

    bool running = true;
    while (running){
        //read the owls camera frames
        Mat left, right, hsvLeft, filteredLeft;
        owl.getCameraFrames(left, right);

        //your tracking code here
        //left *= 1./255;
        cvtColor(left, hsvLeft, COLOR_BGR2HSV);
        inRange(hsvLeft, Scalar(hsv.lh, hsv.ls, hsv.lv), Scalar(hsv.hh, hsv.hs, hsv.hv), filteredLeft);

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

    saveConfig("test.txt", hsv);
}

static void onlhThreshTrackbar(int, void *)
{
    hsv.lh = min(hsv.hh-1, hsv.lh);
    setTrackbarPos("Low H", kWinTitleRaw, hsv.lh);
}
static void onhhThreshTrackbar(int, void *)
{
    hsv.hh = max(hsv.hh, hsv.lh+1);
    setTrackbarPos("High H", kWinTitleRaw, hsv.hh);
}
static void onlsThreshTrackbar(int, void *)
{
    hsv.ls = min(hsv.hs-1, hsv.ls);
    setTrackbarPos("Low S", kWinTitleRaw, hsv.ls);
}
static void onhsThreshTrackbar(int, void *)
{
    hsv.hs = max(hsv.hs, hsv.ls+1);
    setTrackbarPos("High S", kWinTitleRaw, hsv.hs);
}
static void onlvThreshTrackbar(int, void *)
{
    hsv.lv = min(hsv.hv-1, hsv.lv);
    setTrackbarPos("Low V", kWinTitleRaw, hsv.lv);
}
static void onhvThreshTrackbar(int, void *)
{
    hsv.hv = max(hsv.hv, hsv.lv+1);
    setTrackbarPos("High V", kWinTitleRaw, hsv.hv);
}
