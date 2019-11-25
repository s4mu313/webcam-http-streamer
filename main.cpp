#include <opencv4/opencv2/highgui.hpp>

#include "server.h"


int main()
{

    uint16_t port = 5555;                   /// Set the port
    http::Server s(port);
    cv::VideoCapture v(2, cv::CAP_V4L2);    /// Set the correct device id
    std::mutex mtx;

    s.get("/img", [&] (auto, auto res) {

        res.headers.push_back("Connection: close");
        res.headers.push_back("Max-Age: 0");
        res.headers.push_back("Expires: 0");
        res.headers.push_back("Cache-Control: no-cache, private");
        res.headers.push_back("Pragma: no-cache");
        res.headers.push_back("Content-Type: multipart/x-mixed-replace;boundary=--boundary");

        if (!res.send_header())
            return;

        cv::Mat m;
        std::vector<uchar> buf;

        while(true) {
            mtx.lock();
            v >> m;
            mtx.unlock();
            cv::imencode(".jpg", m, buf);
            std::string image (buf.begin(), buf.end());
            if (!res.send_msg("--boundary\r\n"
                              "Content-Type: image/jpeg\r\n"
                              "Content-Length: " +
                              std::to_string(image.size()) +
                              "\r\n\r\n" +
                              image))
                return;
        }

    }).get("/", [](auto, auto res) {
        res >> "<html>"
               "    <body>"
               "        <h1>CAMERA STREAMING</h1>"
                /// Set the correct ip address
               "        <img src='http://192.168.1.69:5555/img'/>"
               "    </body>"
               "</html>";
    }).listen();

    return 0;
}
