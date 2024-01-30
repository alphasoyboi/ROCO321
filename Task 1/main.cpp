//James Rogers Oct 2023 (c) Plymouth University

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <iostream>
#include <string>

#include "../owl.h"

using namespace std;
using namespace cv;

Vec3f RGBtoHSV(const Vec3b& rgb)
{
    // create matrix from rgb pixel
    Mat3f rgbMat(static_cast<Vec3f>(rgb));

    // normalize values
    rgbMat *= 1.0/255.0;

    // convert image to hsv color space
    Mat3f hsvMat;
    cvtColor(rgbMat, hsvMat, COLOR_BGR2HSV);

    // pull out pixel value to return
    Vec3f hsv = hsvMat(0,0);
    return hsv;
}

string getColorString(const Vec3f& hsv)
{
    // this is fairly naive, but works well for the categorized colors, even if slightly desaturated.
    // make sure color isn't black by checking value (hsv[2]),
    //  then compare upper and lower bounds of hue (hsv[0]), and finally saturation (hsv[1]).
    if (hsv[2] >= 0.2f) {
        if ((hsv[0] <= 20.0f || hsv[0] >= 340.0f) && hsv[1] >= 0.5f) {// && hsv[2] >= 0.5f) {
            return "red";
        }
        if ((hsv[0] <= 55.0f && hsv[0] >= 35.0f) && hsv[1] >= 0.45f) {// && hsv[2] >= 0.45f) {
            return "yellow";
        }
        if ((hsv[0] <= 175.0f && hsv[0] >= 100.0f) && hsv[1] >= 0.3f) {// && hsv[2] >= 0.3f) {
           return "green";
        }
        if ((hsv[0] <= 245.0f && hsv[0] >= 185.0f) && hsv[1] >= 0.35f) {// && hsv[2] >= 0.5f) {
           return "blue";
        }
        if ((hsv[0] <= 335.0f && hsv[0] >= 295.0f) && hsv[1] >= 0.3f) {// && hsv[2] >= 0.45f) {
           return "magenta";
        }
    }
    return "n/a";
}

int main()
{
    //connect with the owl and load calibration values
    robotOwl owl(1500, 1475, 1520, 1525, 1520, true); //starts in "quiet mode" which switches off the servos.

    while (true) {
        //read the owls camera frames
        Mat left, right;
        owl.getCameraFrames(left, right);

        //get pixel colour values
        Point centrePoint(left.size().width/2, left.size().height/2);
        Vec3b pixelValue = left.at<Vec3b>(centrePoint);

        //drawing functions
        circle(left, centrePoint, 15, Scalar(255,255,255), 2); //draw a circle to show the pixel being measured
        Vec3f hsv = RGBtoHSV(pixelValue); // convert the pixel to hsv
        string hsvText = "(" + to_string(hsv[0]) + ", " + to_string(hsv[1]) + ", " + to_string(hsv[2]) + ")";
        putText(left, hsvText, centrePoint+Point(-250,100), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255), 2); //draw the string containing hsv components to the image
        string colorText = getColorString(hsv);
        putText(left, colorText, centrePoint+Point(-50,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,255,255), 2); //draw the string denoting the estimated color to the image

        //display image
        imshow("left", left);
        waitKey(30);
    }
}











