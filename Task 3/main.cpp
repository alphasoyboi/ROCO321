//James Rogers Nov 2023 (c) Plymouth University

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <chrono>
#include <cmath>

#include "../owl.h"

using namespace std;
using namespace cv;

#define TARGET_SIZE 60
#define FRAME_W 640
#define FRAME_H 480
#define MOVE_FACTOR 0.1f
#define INTER_EYE_DIST 67
#define SERVO_UPDATE_INTERVAL 250ms

int   calculate_servo_movement(const Point& min_loc);
float calculate_distance(float left_angle, float right_angle);
void  draw_selection_overlay(Mat& left, const Rect& target_pos);
void  draw_target_overlay(Mat& left, Mat& right, const Point& l_min_loc, const Point& r_min_loc, float distance);
void  draw_tracking_overlay(Mat& left, Mat& right, bool tracking);
void  draw_help_overlay(Mat& left, Mat& right);

int main()
{
    // connect with the owl and load calibration values
    robotOwl owl(1475, 1510, 1550, 1440, 1560);
    int rx_rst, ry_rst, lx_rst, ly_rst, neck;
    owl.getRawServoPositions(rx_rst, ry_rst, lx_rst, ly_rst, neck);

    Mat left, right, target, l_match, r_match;
    Rect target_pos(FRAME_W/2 - TARGET_SIZE/2, FRAME_H/2 - TARGET_SIZE/2, TARGET_SIZE, TARGET_SIZE);

    double l_min_val, l_max_val, r_min_val, r_max_val;
    Point l_min_loc, l_max_loc, r_min_loc, r_max_loc;

    bool running = true, selecting = true, tracking = false;
    auto time_prev = chrono::high_resolution_clock::now();
    chrono::milliseconds time_elap = 0ms;

    while (running)
    {
        // read the owls camera frames
        owl.getCameraFrames(left, right);

        // selection mode or tracking mode
        if (selecting) {
            draw_selection_overlay(left, target_pos);
            imshow("left", left);
        } else {
            // match target image to frames and min target in location
            matchTemplate(left, target, l_match, TM_SQDIFF_NORMED);
            matchTemplate(right, target, r_match, TM_SQDIFF_NORMED);
            minMaxLoc(l_match, &l_min_val, &l_max_val, &l_min_loc, &l_max_loc);
            minMaxLoc(r_match, &r_min_val, &r_max_val, &r_min_loc, &r_max_loc);

            // get servo control parameters
            int l_move = calculate_servo_movement(l_min_loc);
            int r_move = calculate_servo_movement(r_min_loc);

            // move servos if tracking is enabled
            if (tracking) {
                time_elap += chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - time_prev);
                if (time_elap >= SERVO_UPDATE_INTERVAL) {
                    owl.setServoRelativePositions(r_move, 0, l_move, 0, 0);
                    time_elap = 0ms;
                }
                time_prev = chrono::high_resolution_clock::now();
            }

            // calculate distance from servo angles
            float l_angle, r_angle, distance;
            owl.getServoAngles(l_angle, r_angle);
            distance = calculate_distance(l_angle, r_angle);

            // display camera frames
            draw_target_overlay(left, right, l_min_loc, r_min_loc, distance);
            draw_tracking_overlay(left, right, tracking);
            draw_help_overlay(left, right);
            imshow("left", left);
            imshow("right", right);
        }

        // process user control
        switch (waitKey(10)) {
        case ' ':
            if (selecting) {
                left(target_pos).copyTo(target);
                imshow("target", target);
            } else {
                tracking = false;
                owl.setServoRawPositions(rx_rst, ry_rst, lx_rst, ly_rst, neck);
            }
            selecting = !selecting;
            break;
        case 't':
            tracking = !tracking;
            break;
        case 'r':
            owl.setServoRawPositions(rx_rst, ry_rst, lx_rst, ly_rst, neck);
            break;
        case 'q':
        case 27:
            running = false;
            break;
        }
    }
}

// calculate how much to move servos to bring the target into the center of the frame
// get the difference between the target point and the center of the frame
// return the difference multiplied by a scaling factor
int calculate_servo_movement(const Point& min_loc) {
    int diff = min_loc.x - FRAME_W/2;
    return int(diff * MOVE_FACTOR);
}

// calculate distance from the left each eye using law of sines 
//     A
//     /\    
//  c /  \ b 
//   /____\   B is the left eye
//  B  a   C 
//
// c = a*sinC/sinA
float calculate_distance(float left_angle, float right_angle) {
    // angles need to be subtracted from 90 degrees or pi/2
    left_angle = float(M_PI_2) - left_angle;
    right_angle = float(M_PI_2) - right_angle;
    float eye_angle = float(M_PI) - left_angle - right_angle;
    // calculate left eye distance using sine laws
    float left_dist = INTER_EYE_DIST*sin(right_angle)/sin(eye_angle);
}

// draw target selection box and key binding info text
void draw_selection_overlay(Mat& left, const Rect& target_pos) {
    rectangle(left, target_pos, Scalar(0, 255, 0), 2);
    putText(left, "press space to capture target", {5, FRAME_H - 5}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA);
}

// draw target tracking box and distance estimate
void draw_target_overlay(Mat& left, Mat& right, const Point& l_min_loc, const Point& r_min_loc, float distance) {
    rectangle(left, {l_min_loc.x, l_min_loc.y, TARGET_SIZE, TARGET_SIZE}, Scalar(0, 255, 0), 1);
    rectangle(right, {r_min_loc.x, r_min_loc.y, TARGET_SIZE, TARGET_SIZE}, Scalar(0, 255, 0), 1);
    putText(left, to_string(distance) + "mm", {l_min_loc.x, l_min_loc.y-5}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA);
    putText(right, to_string(distance) + "mm", {r_min_loc.x, r_min_loc.y-5}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA);
}

// draw tracking status info
void draw_tracking_overlay(Mat& left, Mat& right, bool tracking) {
    if (tracking) {
        putText(left, "tracking enabled", {5, FRAME_H-85}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA);
        putText(right, "tracking enabled", {5, FRAME_H-85}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 255, 0), 1, LINE_AA);
    } else {
        putText(left, "tracking disbaled", {5, FRAME_H-85}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 1, LINE_AA);
        putText(right, "tracking disbaled", {5, FRAME_H-85}, FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 1, LINE_AA);
    }
}

// draw key binding help
void draw_help_overlay(Mat& left, Mat& right) {
    putText(left, "press t to toggle tracking", {5, FRAME_H-65}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 0), 1, LINE_AA);
    putText(right, "press t to toggle tracking", {5, FRAME_H-65}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 0), 1, LINE_AA);
    putText(left, "press r to reset servos", {5, FRAME_H-45}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 0), 1, LINE_AA);
    putText(right, "press r to reset servos", {5, FRAME_H-45}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 0), 1, LINE_AA);
    putText(left, "press space to select new target", {5, FRAME_H-25}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 0), 1, LINE_AA);
    putText(right, "press space to select new target", {5, FRAME_H-25}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 0), 1, LINE_AA);
    putText(left, "press q to quit", {5, FRAME_H-5}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 0), 1, LINE_AA);
    putText(right, "press q to quit", {5, FRAME_H-5}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 0), 1, LINE_AA);
}
