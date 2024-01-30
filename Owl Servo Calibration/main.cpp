//James Rogers Oct 2023 (c) Plymouth University

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <iostream>
#include <string>

#include "../owl.h"

using namespace std;
using namespace cv;

void drawUI(Mat& left, Mat& right, char key);

int main()
{
    //connect with the owl and load calibration values
    robotOwl owl(1500, 1475, 1520, 1525, 1520);

    while (true){
        //read the owls camera frames and record the users keypress
        Mat left, right;
        owl.getCameraFrames(left, right);
        char key = waitKey(30);
        drawUI(left, right, key);
        imshow("left",left);
        imshow("right",right);

        //based on the key press, move a servo a set ammount
        switch (key){
            case 'i': owl.setServoRelativePositions( 0, 5, 0, 0, 0); break;
            case 'k': owl.setServoRelativePositions( 0,-5, 0, 0, 0); break;
            case 'j': owl.setServoRelativePositions(-5, 0, 0, 0, 0); break;
            case 'l': owl.setServoRelativePositions( 5, 0, 0, 0, 0); break;
            case 'w': owl.setServoRelativePositions( 0, 0, 0, 5, 0); break;
            case 's': owl.setServoRelativePositions( 0, 0, 0,-5, 0); break;
            case 'a': owl.setServoRelativePositions( 0, 0,-5, 0, 0); break;
            case 'd': owl.setServoRelativePositions( 0, 0, 5, 0, 0); break;
            case 'q': owl.setServoRelativePositions( 0, 0, 0, 0,-5); break;
            case 'e': owl.setServoRelativePositions( 0, 0, 0, 0, 5); break;
        }

        //if a key has been pressed, print the new servo positions
        if(key!=-1)
        {
            int Rx, Ry, Lx, Ly, Neck;
            owl.getRawServoPositions(Rx, Ry, Lx, Ly, Neck);
            cout<<"Raw servo positions=("<<Rx<<", "<<Ry<<", "<<Lx<<", "<<Ly<<", "<<Neck<<")"<<endl;
        }
    }
}

void drawUI(Mat& left, Mat& right, char key)
{
    Point cameraCentre = Point(left.size().width, left.size().height)/2;
    //draw left marker
    Mat leftMask(left.size(), CV_8U, Scalar(0));
    circle(leftMask, cameraCentre, 7, Scalar(255), 4);
    line(leftMask,cameraCentre+Point(7,0), cameraCentre+Point(17,0), Scalar(255), 4);
    line(leftMask,cameraCentre-Point(7,0), cameraCentre-Point(17,0), Scalar(255), 4);
    line(leftMask,cameraCentre+Point(0,7), cameraCentre+Point(0,17), Scalar(255), 4);
    line(leftMask,cameraCentre-Point(0,7), cameraCentre-Point(0,17), Scalar(255), 4);

    //draw left key boxes
    rectangle(leftMask, Rect(70 ,360,50,50), Scalar(255), (key=='w')?6:4);//border width changes if they key matches the button
    rectangle(leftMask, Rect(10 ,420,50,50), Scalar(255), (key=='a')?6:4);
    rectangle(leftMask, Rect(70 ,420,50,50), Scalar(255), (key=='s')?6:4);
    rectangle(leftMask, Rect(130,420,50,50), Scalar(255), (key=='d')?6:4);
    rectangle(leftMask, Rect(520,420,50,50), Scalar(255), (key=='q')?6:4);
    rectangle(leftMask, Rect(580,420,50,50), Scalar(255), (key=='e')?6:4);

    //draw left letters
    putText(leftMask, "W", Point(77 ,400),  FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='w')?6:4);
    putText(leftMask, "A", Point(21 ,460),  FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='a')?6:4);
    putText(leftMask, "S", Point(79 ,460),  FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='s')?6:4);
    putText(leftMask, "D", Point(138,460),  FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='d')?6:4);
    putText(leftMask, "Q", Point(527 ,460), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='q')?6:4);
    putText(leftMask, "E", Point(589,460),  FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='e')?6:4);
    putText(leftMask, "Neck", Point(535, 410), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255), 4);

    //draw right marker
    Mat rightMask(left.size(), CV_8U, Scalar(0));
    circle(rightMask, cameraCentre, 7, Scalar(255), 4);
    line(rightMask,cameraCentre+Point(7,0), cameraCentre+Point(17,0), Scalar(255), 4);
    line(rightMask,cameraCentre-Point(7,0), cameraCentre-Point(17,0), Scalar(255), 4);
    line(rightMask,cameraCentre+Point(0,7), cameraCentre+Point(0,17), Scalar(255), 4);
    line(rightMask,cameraCentre-Point(0,7), cameraCentre-Point(0,17), Scalar(255), 4);

    //draw right key boxes
    rectangle(rightMask, Rect(70 ,360,50,50), Scalar(255), (key=='i')?6:4);//border width changes if they key matches the button
    rectangle(rightMask, Rect(10 ,420,50,50), Scalar(255), (key=='j')?6:4);
    rectangle(rightMask, Rect(70 ,420,50,50), Scalar(255), (key=='k')?6:4);
    rectangle(rightMask, Rect(130,420,50,50), Scalar(255), (key=='l')?6:4);
    rectangle(rightMask, Rect(520,420,50,50), Scalar(255), (key=='q')?6:4);
    rectangle(rightMask, Rect(580,420,50,50), Scalar(255), (key=='e')?6:4);

    //draw right letters
    putText(rightMask, "I", Point(88 ,400), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='i')?6:4);
    putText(rightMask, "J", Point(23 ,460), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='j')?6:4);
    putText(rightMask, "K", Point(79 ,460), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='k')?6:4);
    putText(rightMask, "L", Point(140,460), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='l')?6:4);
    putText(rightMask, "Q", Point(527 ,460), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='q')?6:4);
    putText(rightMask, "E", Point(589,460),  FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255), (key=='e')?6:4);
    putText(rightMask, "Neck", Point(535, 410), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255), 4);


    //draw in black, then erode, then draw in white. gives everything a black outline
    left.setTo(Scalar(0,0,0), leftMask);
    cv::erode(leftMask, leftMask, cv::Mat::ones(3, 3, CV_8U));
    left.setTo(Scalar(255,255,255), leftMask);

    right.setTo(Scalar(0,0,0), rightMask);
    cv::erode(rightMask, rightMask, cv::Mat::ones(3, 3, CV_8U));
    right.setTo(Scalar(255,255,255), rightMask);

}



















