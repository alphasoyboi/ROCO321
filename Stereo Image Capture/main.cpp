//James Rogers Oct 2023 (c) Plymouth University

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <iostream>
#include <string>

#include "../owl.h"

using namespace std;
using namespace cv;

string outputFolder="../Stereo Image Capture/CapturedImages";

int main()
{
    //connect with the owl and load calibration values
    robotOwl owl(1500, 1475, 1520, 1525, 1520);
    int imgNumber=0;

    while (true){
        //read the owls camera frames
        Mat left, right;
        owl.getCameraFrames(left, right);

        //stitch images
        Mat stereo(left.size().height, left.size().width*2, CV_8UC3, Scalar(0,0,0));
        left .copyTo(stereo(Rect(0,0,left.size().width,left.size().height)));
        right.copyTo(stereo(Rect(left.size().width,0,left.size().width,left.size().height)));

        //draw text and display
        putText(stereo, "Press SPACE to take a picture", Point(250, 460), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 2);
        putText(stereo, "Images Captured: " + to_string(imgNumber), Point(10,35), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255,255,255), 2);
        imshow("stereo",stereo);

        //handle keypress
        if(waitKey(10)==' ')
        {
            cout<<"Saving image pair "<<imgNumber<<" to \""<<outputFolder<<"\""<<endl;
            imwrite(outputFolder+"/right"+to_string(imgNumber)+".png", right);
            imwrite(outputFolder+"/left" +to_string(imgNumber)+".png", left);
            imgNumber++;
        }
    }
}











