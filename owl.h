//James Rogers Oct 2023 (c) Plymouth University

#ifndef OWL_H
#define OWL_H

#include <iostream> // for standard I/O
#include <string>   // for strings
#include <winsock2.h>
#include <windows.h>
#include <sys/types.h>
#include <sstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

#define SERVO_PWM2RAD 0.00174532925

//A class to manage the TCP and IP camera streams between the owl and the PC
class robotOwl
{
public:

    //connect to the owl on initilisation
    robotOwl(int RxC, int RyC, int LxC, int LyC, int NeckC, bool quietMode = false)
    {
        //quiet mode doesnt activate the motors, you can use this if you only need the camera feed
        this->quietMode=quietMode;

        if(!quietMode)
        {
            //check winSock version
            WSAData version;
            WORD mkword=MAKEWORD(2,2);
            if(WSAStartup(mkword,&version)!=0)
                std::cout<<"WinSock version is not supported! - \n"<<WSAGetLastError()<<std::endl;
            else
                std::cout<<"winSock initialised"<<std::endl;

            //create socket
            u_sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            if(u_sock==INVALID_SOCKET)
                std::cout<<"Failed to create socket\n";
            else
                std::cout<<"Socket created\n";

            //Socket address information
            sockaddr_in addr;
            addr.sin_family=AF_INET;
            addr.sin_addr.s_addr=inet_addr(PiADDR.c_str());
            addr.sin_port=htons(PORT); // HAS to match that defined in Pi server

            //Now to connect
            int conn=connect(u_sock,(SOCKADDR*)&addr,sizeof(addr));
            if(conn==SOCKET_ERROR){
                std::cout<<"Unable to open socket "<<WSAGetLastError()<<std::endl;
                closesocket(u_sock);
                WSACleanup();

                cout<<"Make sure to run ./OWLsocket on the owls terminal"<<endl;
                while(1);
            }
            else
                cout<<"Owl TCP connection established"<<endl;

            //save calibration values
            this->RxC=RxC;
            this->RyC=RyC;
            this->LxC=LxC;
            this->LyC=LyC;
            this->NeckC=NeckC;
            setServoRawPositions(RxC, RyC, LxC, LyC, NeckC);
        }

        //connect camera stream
        cap.open(source);
        if(cap.isOpened())
            cout<<"Owl cameras connected"<<endl;
        else
            cout<<"Failed to open camera steam"<<endl;

    }

    //close stream when owl leaves scope
    ~robotOwl()
    {
        closesocket(u_sock);
    }

    //set all servos to raw PWM positions
    void setServoRawPositions(int Rx, int Ry, int Lx, int Ly, int Neck)
    {
        if(quietMode)//skip this if in quite mode
            return;

        //update servo variables
        this->Rx=Rx;
        this->Ry=Ry;
        this->Lx=Lx;
        this->Ly=Ly;
        this->Neck=Neck;

        sendServoPos();
    }

    //set all servos to relative positions
    void setServoRelativePositions(int Rx, int Ry, int Lx, int Ly, int Neck)
    {
        if(quietMode)//skip this if in quite mode
            return;

        //update servo variables
        this->Rx+=Rx;
        this->Ry+=Ry;
        this->Lx+=Lx;
        this->Ly-=Ly;
        this->Neck-=Neck;

        sendServoPos();
    }

    //set all servos to absolute positions with 0,0 looking straight forward
    void setServoAbsolutePositions(int Rx, int Ry, int Lx, int Ly, int Neck)
    {
        if(quietMode)//skip this if in quite mode
            return;

        //update servo variables
        this->Rx=RxC+Rx;
        this->Ry=RyC+Ry;
        this->Lx=LxC+Lx;
        this->Ly=LyC-Ly;
        this->Neck=NeckC-Neck;

        sendServoPos();
    }

    //read camera frames
    void getCameraFrames(Mat& left, Mat& right)
    {
        //if the cameras dont return a frame, set frame to black
        Mat Frame;
        if (!cap.read(Frame))
        {
            cout  << "Could not open the input video: " << source << endl;
            Frame = Mat(Size(640*2,480), CV_8UC3, Scalar(0,0,0));
        }

        //flip and split the frame into left and right images
        flip(Frame,Frame,1);
        left= Frame( Rect(0, 0, 640, 480));
        right=Frame( Rect(640, 0, 640, 480));
    }

    //return the raw servo positions
    void getRawServoPositions(int& Rx, int& Ry, int& Lx, int& Ly, int& Neck)
    {
        Rx=this->Rx;
        Ry=this->Ry;
        Lx=this->Lx;
        Ly=this->Ly;
        Neck=this->Neck;
    }

    //return servo positions relative to the origin
    void getRelativeServoPositions(int& Rx, int& Ry, int& Lx, int& Ly, int& Neck)
    {
        Rx=this->Rx-RxC;
        Ry=this->Ry-RyC;
        Lx=this->Lx-LxC;
        Ly=LyC-this->Ly;
        Neck=NeckC-this->Neck;
    }
	
	//get servo angles in radians for both the left and right eyes. a value of zero is looking straight forward, and positive is clockwise.
    void getServoAngles(float& left, float& right)
    {
        left=static_cast<float>(Lx-LxC)*SERVO_PWM2RAD;
        right=static_cast<float>(RxC-Rx)*SERVO_PWM2RAD;
    }
	
	

private:
    SOCKET u_sock;
    string source ="http://10.0.0.10:8080/stream/video.mjpeg"; // was argv[1];           // the source file name
    string PiADDR = "10.0.0.10";
    int PORT=12345;
    VideoCapture cap;
    bool quietMode;

    int Rx, Ry, Lx, Ly, Neck;
    int RxC=1530, RyC=1455, LxC=1530, LyC=1540, NeckC=1520; //default calib values
    //int vRange = 800;
    //int hRange = 600;
    //int nRange = 700;

    //Send data over the TCP connection
    void sendPacket (string CMD){
        char receivedCHARS[3] = {0,0,0}; // send 'ok' back

        int smsg=send(u_sock,CMD.c_str(),strlen(CMD.c_str()),0);
        if(smsg==SOCKET_ERROR){
            std::cout<<"Socket Sending Error "<<WSAGetLastError()<<std::endl;
            WSACleanup();
        }
        int N=recv(u_sock,receivedCHARS,2,0);
        if(N==SOCKET_ERROR){
            std::cout<<"Socket Receiving: Error "<<WSAGetLastError()<<std::endl;
        }
    }

    void sendServoPos()
    {
        ostringstream CMDstream;
        CMDstream.str("");
        CMDstream.clear();
        CMDstream << Rx << " " << Ry << " " << Lx << " " << Ly << " " << Neck;
        sendPacket (CMDstream.str().c_str());
    }


};

#endif // OWL_H
