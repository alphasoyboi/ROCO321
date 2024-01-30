//James Rogers Nov 2023 (c) Plymouth University

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <iostream>
#include <string>

#include "../owl.h"

using namespace std;
using namespace cv;

int main()
{

    //connect with the owl and load calibration values
    robotOwl owl(1500, 1475, 1520, 1525, 1520);

    while (true){
        //read the owls camera frames
        Mat left, right;
        owl.getCameraFrames(left, right);

        //your tracking code here












        //display camera frame
        imshow("left",left);
        waitKey(10);
    }
}


















