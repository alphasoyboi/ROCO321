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
#define FRAME_CENTER_X FRAME_WIDTH/2
#define FRAME_CENTER_Y FRAME_HEIGHT/2
#define MOVE_FACTOR_X 0.25f
#define MOVE_FACTOR_Y 0.25f
#define MOVE_FACTOR_NECK MOVE_FACTOR_X/2

static const String kWinTitleRaw      = "left";
static const String kWinTitleFiltered = "left filtered";

static HSVConfig hsv;

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

    hsv = loadConfig(HSV_CONFIG_FILEPATH);
    namedWindow(kWinTitleRaw);
    namedWindow(kWinTitleFiltered);
    createTrackbar("Low Hue",  kWinTitleRaw, &hsv.lh, MAX_H, onLowHueThreshTrackbar);
    createTrackbar("High Hue", kWinTitleRaw, &hsv.hh, MAX_H, onHighHueThreshTrackbar);
    createTrackbar("Low Sat",  kWinTitleRaw, &hsv.ls, MAX_SV, onLowSatThreshTrackbar);
    createTrackbar("High Sat", kWinTitleRaw, &hsv.hs, MAX_SV, onHighSatThreshTrackbar);
    createTrackbar("Low Val",  kWinTitleRaw, &hsv.lv, MAX_SV, onLowValThreshTrackbar);
    createTrackbar("High Val", kWinTitleRaw, &hsv.hv, MAX_SV, onHighValThreshTrackbar);

    Mat left, right, hsvLeft, filteredLeft;
    bool running = true;
    bool tracking = false;
    while (running) {
        //read the owls camera frames
        owl.getCameraFrames(left, right);

        string trackText = "t = toggle tracking";
        string saveText = "s = save hsv config";
        string quitText = "q = quit";
        putText(left, trackText, {5, 30}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA); //draw the string containing hsv components to the image
        putText(left, saveText, {5, 60}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA); //draw the string containing hsv components to the image
        putText(left, quitText, {5, 90}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA); //draw the string containing hsv components to the image

        //your tracking code here
        cvtColor(left, hsvLeft, COLOR_BGR2HSV);
        inRange(hsvLeft, Scalar(hsv.lh, hsv.ls, hsv.lv), Scalar(hsv.hh, hsv.hs, hsv.hv), filteredLeft);

        Moments m = moments(filteredLeft, true);
        Point center{int(m.m10/m.m00), int(m.m01/m.m00)};
        //center = (m.m00 < 10) ? Point{FRAME_CENTER_X, FRAME_CENTER_Y} : Point{int(m.m10/m.m00), int(m.m01/m.m00)};
        center.x = (center.x > FRAME_HEIGHT) || (center.x < -FRAME_HEIGHT) ? FRAME_CENTER_X : center.x;
        center.y = (center.y > FRAME_HEIGHT) || (center.y < -FRAME_HEIGHT) ? FRAME_CENTER_Y : center.y;
        circle(left, center, 5, Scalar(128), -1);
        circle(filteredLeft, center, 5, Scalar(128), -1);

        if (tracking) {
            string statusText = "head tracking enabled";
            putText(left, statusText, {5, FRAME_HEIGHT - 5}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA);

            int xr, yr, xl, yl, neck;
            owl.getRelativeServoPositions(xr, yr, xl, yl, neck);
            int xDiff = center.x - FRAME_CENTER_X;
            int xMove = int(xDiff * MOVE_FACTOR_X);
            int neckMove = (xl < 50) && (xl > -50) ? 0 : int(xl * MOVE_FACTOR_NECK);
            owl.setServoRelativePositions(0, 0, xMove, 0, neckMove);
            int yDiff = center.y - FRAME_CENTER_Y;
            int yMove = int(yDiff * MOVE_FACTOR_Y);
            owl.setServoRelativePositions(0, 0, 0, -yMove, 0);
        } else {
            string statusText = "head tracking disbaled";
            putText(left, statusText, {5, FRAME_HEIGHT - 5}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA);
        }

        //display camera frame
        imshow(kWinTitleRaw, left);
        imshow(kWinTitleFiltered, filteredLeft);
        switch(waitKey(10)) {
        case 'q':
        case 27: // ESC
            running = false;
            break;
        case 's':
            saveConfig(HSV_CONFIG_FILEPATH, hsv);
            break;
        case 't':
            tracking = !tracking;
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
