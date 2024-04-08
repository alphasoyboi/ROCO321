//James Rogers Jan 2024 (c) Plymouth University

/*
 * sources:
 * https://docs.opencv.org/3.4/da/d6a/tutorial_trackbar.html
 * https://docs.opencv.org/3.4/dd/d53/tutorial_py_depthmap.html
 * https://www.raspberrypi.com/documentation/accessories/camera.html
 * https://docs.opencv.org/4.x/d2/de8/group__core__array.html#gaf9771c991763233866bf76b5b5d1776f
 */

#include <iostream>
#include <fstream>
#include <type_traits>
#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>

#include "../owl.h"

using namespace cv;
using namespace std;

#define EYES_WIN_NAME "eyes"
#define DISP_WIN_NAME "disparity"
#define SAD_WIN_SIZE_TB_NAME "SAD Window Size"
#define NUM_DISPARITIES_TB_NAME "Number of Disparities"

#define SAD_WIN_SIZE_MIN 3
#define SAD_WIN_SIZE_MAX 21
#define NUM_DISPARITIES_MIN 16
#define NUM_DISPARITIES_MAX 512

#define DEFAULT_BASE_FOCAL_PRODUCT 675166.
#define CALIB_DIST_START 350
#define CALIB_DIST_INTERVAL 50
#define CALIB_COUNT 10

template <typename T, size_t N>
class ContinuousAverage {
public:
    ContinuousAverage() {
        static_assert(std::is_arithmetic<T>::value, "must be built-in arithmetic type.");
        static_assert(N > 0, "N must be greater than 0");
        memset(buf_, 0, N*sizeof(T));
    }
    void push(T value) {
        buf_[head_] = value;
        head_ = (head_+1) % N;
        if (!full()) {
            count_++;
        }
    }
    double average() const {
        T sum = 0;
        for (size_t i = 0; i < count_; i++) {
            sum += buf_[i];
        }
        return static_cast<double>(sum)/count_;
    }
    size_t count() const {
        return count_;
    }
    bool full() const {
        if (count_ < N) {
            return false;
        }
        return true;
    }
private:
    T buf_[N];
    size_t head_ = 0;
    size_t count_ = 0;
};

void on_tb_sad_window_size(int pos, void* userdata);
void on_tb_num_disparities(int pos, void* userdata);
void on_mouse(int event, int x, int y, int flags, void *userdata);
void draw_calibrate_ui(Mat& disp8, int distance, short disparity);
void draw_measure_ui(Mat& left, Mat& disp8, const Point& disp_coords, double distance);

int main(int argc, char** argv) {
    // connect with the owl and load calibration values
    // robotOwl owl(1475, 1510, 1550, 1440, 1560);
    robotOwl owl(1485, 1505, 1555, 1445, 1560);
    // calibration file paths
    string intrinsic_filename = "../intrinsics.xml";
    string extrinsic_filename = "../extrinsics.xml";

    //================================================Load Calibration Files===============================================
    Rect roi1, roi2;
    Mat Q;
    Size img_size = {640,480};

    FileStorage fs(intrinsic_filename, FileStorage::READ);
    if(!fs.isOpened()){
        printf("Failed to open file %s\n", intrinsic_filename.c_str());
        return -1;
    }

    Mat M1, D1, M2, D2;
    fs["M1"] >> M1;
    fs["D1"] >> D1;
    fs["M2"] >> M2;
    fs["D2"] >> D2;

    fs.open(extrinsic_filename, FileStorage::READ);
    if(!fs.isOpened())
    {
        printf("Failed to open file %s\n", extrinsic_filename.c_str());
        return -1;
    }
    Mat R, T, R1, P1, R2, P2;
    fs["R"] >> R;
    fs["T"] >> T;

    stereoRectify( M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, img_size, &roi1, &roi2 );

    Mat map11, map12, map21, map22;
    initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map11, map12);
    initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map21, map22);



    //==================================================Create Block Matcher==============================================
    int sad_window_size = 7;   //must be an odd number >=3
    int num_disparities = 144; //must be divisable by 16
    Ptr<StereoSGBM> sgbm = StereoSGBM::create(0,16,3);
    sgbm->setBlockSize(sad_window_size);
    sgbm->setPreFilterCap(63);
    sgbm->setP1(8*3*sad_window_size*sad_window_size);
    sgbm->setP2(32*3*sad_window_size*sad_window_size);
    sgbm->setMinDisparity(0);
    sgbm->setNumDisparities(num_disparities);
    sgbm->setUniquenessRatio(10);
    sgbm->setSpeckleWindowSize(100);
    sgbm->setSpeckleRange(32);
    sgbm->setDisp12MaxDiff(1);
    sgbm->setMode(StereoSGBM::MODE_SGBM);

    namedWindow(DISP_WIN_NAME);
    createTrackbar(SAD_WIN_SIZE_TB_NAME, DISP_WIN_NAME, &sad_window_size, SAD_WIN_SIZE_MAX, on_tb_sad_window_size, &sgbm);
    setTrackbarMin(SAD_WIN_SIZE_TB_NAME, DISP_WIN_NAME, SAD_WIN_SIZE_MIN);
    setTrackbarPos(SAD_WIN_SIZE_TB_NAME, DISP_WIN_NAME, sad_window_size);
    createTrackbar(NUM_DISPARITIES_TB_NAME, DISP_WIN_NAME, &num_disparities, NUM_DISPARITIES_MAX, on_tb_num_disparities, &sgbm);
    setTrackbarMin(NUM_DISPARITIES_TB_NAME, DISP_WIN_NAME, NUM_DISPARITIES_MIN);
    setTrackbarPos(NUM_DISPARITIES_TB_NAME, DISP_WIN_NAME, num_disparities);

    Point disp_coords = Point(img_size/2);
    setMouseCallback(DISP_WIN_NAME, on_mouse, &disp_coords);

    Mat left, right, eyes, disp, disp8;
    ContinuousAverage<double, 16> distance;

    double base_focal_product = DEFAULT_BASE_FOCAL_PRODUCT;
    ContinuousAverage<short, 16> disparity;
    ContinuousAverage<double, CALIB_COUNT> calibrations;

    int key_press = -1;
    bool running = true, calibrate = false;
    while (running) {

        // read the owls camera frames
        owl.getCameraFrames(left, right);

        // distort images to correct for lens/positional distortion
        remap(left, left, map11, map12, INTER_LINEAR);
        remap(right, right, map21, map22, INTER_LINEAR);

        // match left and right images to create disparity image
        sgbm->compute(left, right, disp);
        // convert disparity map to an 8-bit greyscale image so it can be displayed (do not use for mesurements)
        disp.convertTo(disp8, CV_8U, 255/(num_disparities*16.));

        if (calibrate) {
            short distance = CALIB_DIST_START + CALIB_DIST_INTERVAL*short(calibrations.count());
            disparity.push(disp.at<short>(img_size/2));
            if (key_press == ' ') {
                calibrations.push(distance*disparity.average());
                if (calibrations.full()) {
                    base_focal_product = calibrations.average();
                    calibrate = false;
                }
            }
            draw_calibrate_ui(disp8, distance, short(disparity.average()));
        } else {
            distance.push(base_focal_product/disp.at<short>(disp_coords));
            draw_measure_ui(disp8, disp_coords, distance.average());
        }

        // display images
        hconcat(left, right, eyes); // combine left and right into one window
        imshow(EYES_WIN_NAME, eyes);
        imshow(DISP_WIN_NAME, disp8);

        switch (key_press = waitKey(10)) {
        case 'c':
            calibrate = true;
            break;
        case 'q':
            running = false;
            break;
        }
    }

    return 0;
}

void on_tb_sad_window_size(int pos, void* userdata) {
    int sad_window_size = (pos%2) ? pos : pos+1;
    setTrackbarPos(SAD_WIN_SIZE_TB_NAME, DISP_WIN_NAME, sad_window_size);

    Ptr<StereoSGBM>& sgbm = *static_cast<Ptr<StereoSGBM>*>(userdata);
    sgbm->setBlockSize(sad_window_size);
    sgbm->setP1(8*3*sad_window_size*sad_window_size);
    sgbm->setP2(32*3*sad_window_size*sad_window_size);
}

void on_tb_num_disparities(int pos, void* userdata) {
    int num_disparities = pos - pos%16;
    setTrackbarPos(NUM_DISPARITIES_TB_NAME, DISP_WIN_NAME, num_disparities);

    Ptr<StereoSGBM>& sgbm = *static_cast<Ptr<StereoSGBM>*>(userdata);
    sgbm->setNumDisparities(num_disparities);
}

void on_mouse(int event, int x, int y, int flags, void *userdata) {
    switch(event) {
    case EVENT_LBUTTONDOWN:
        *static_cast<Point*>(userdata) = Point(x, y);
    }
}

void draw_calibrate_ui(Mat& disp8, int distance, short disparity) {
    circle(disp8, {disp8.cols/2, disp8.rows/2}, 8, Scalar(255, 255, 255), 1);
    putText(disp8, "calibration mode:", {5, 25}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 255), 1, LINE_AA);
    putText(disp8, "move to " + to_string(distance) + "mm and press space", {5, 45}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 255), 1, LINE_AA);
    putText(disp8, "disparity: " + to_string(disparity), {5, disp8.rows-5}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 255), 1, LINE_AA);
}

void draw_measure_ui(Mat& disp8, const Point& disp_coords, double distance) {
        circle(disp8, disp_coords, 8, Scalar(255, 255, 255), 1);
        putText(disp8, "distance: " + to_string(distance) + "mm", {5, left.rows-45}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 255), 1, LINE_AA);
        putText(disp8, "press c to calibrate", {5, disp8.rows-25}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 255), 1, LINE_AA);
        putText(disp8, "press q to quit", {5, disp8.rows-5}, FONT_HERSHEY_PLAIN, 1.5, Scalar(255, 255, 255), 1, LINE_AA);
}
