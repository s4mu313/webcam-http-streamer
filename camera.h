#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <mutex>


class Camera {

private:
    std::mutex mutex;
    cv::VideoCapture capture;

public:
    std::string getFrameInByteArray();
    std::string getGrayFrameInByteArray();
    std::string getGrayBlurFrameInByteArray();

    Camera(std::string address);
    Camera(int n = 0);
    ~Camera();
};

#endif // CAMERA_H
