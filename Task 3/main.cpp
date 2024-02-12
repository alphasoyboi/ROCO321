//James Rogers Nov 2023 (c) Plymouth University

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <iostream>
#include <string>

#include "../owl.h"

using namespace std;
using namespace cv;

#define targetSize 60 //adjust this to change the size of your target

int main()
{
    //connect with the owl and load calibration values
    robotOwl owl(1500, 1475, 1520, 1525, 1520);

    //===========================target selection loop==============================
    Mat target;
    Rect targetPos(320-targetSize/2, 240-targetSize/2, targetSize, targetSize);
    while (true){
        //read the owls camera frames
        Mat left, right;
        owl.getCameraFrames(left, right);

        //if SPACE is pressed, save the target image and break the loop
        if(waitKey(10)==' ')
        {
            left(targetPos).copyTo(target);
            cout<<"Target selected!"<<endl;
            break;
        }

        //display camera frame
        rectangle(left, targetPos, Scalar(255,255,255), 2);
        imshow("left",left);        
    }

    //===========================target tracking loop================================
    cout<<"Starting tracking code..."<<endl;
    while(1)
    {
        //read the owls camera frames
        Mat left, right;
        owl.getCameraFrames(left, right);

        //your tracking code goes here
















        //display camera frames
        imshow("left", left);
        imshow("right", right);
        imshow("target", target);
        waitKey(10);
    }


}


















