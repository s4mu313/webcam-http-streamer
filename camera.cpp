#include "camera.h"
#include <opencv2/imgproc/imgproc.hpp>

std::string Camera::getGrayFrameInByteArray()
{
    cv::Mat m;
    mutex.lock();
    capture >> m;
    mutex.unlock();
    cv::cvtColor(m, m, CV_BGR2GRAY);
    std::vector<uchar> buf;
    cv::imencode(".jpg", m, buf);
    std::string byteArray(buf.begin(), buf.end());
    return byteArray;
}

std::string Camera::getGrayBlurFrameInByteArray()
{
    cv::Mat m;
    mutex.lock();
    capture >> m;
    mutex.unlock();
    cv::cvtColor(m, m, CV_BGR2GRAY);
    cv::GaussianBlur(m, m, cv::Size(7,7), 1.5, 1.5);
    std::vector<uchar> buf;
    cv::imencode(".jpg", m, buf);
    std::string byteArray(buf.begin(), buf.end());
    return byteArray;
}

std::string Camera::getFrameInByteArray()
{
    cv::Mat m, resized;
    mutex.lock();
    capture >> m;
    mutex.unlock();
    std::vector<uchar> buf;
    cv::imencode(".jpg", m, buf);
    std::string byteArray(buf.begin(), buf.end());
    return byteArray;
}

Camera::~Camera()
{
    capture.release();
}

Camera::Camera(int n)
{
    capture.open(n);
}

Camera::Camera(std::string address)
{
    capture.open(address);
}
