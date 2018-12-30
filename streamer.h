#ifndef STREAMER_H
#define STREAMER_H

#include <opencv2/highgui/highgui.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <mutex>
#include <atomic>
#include <thread>


//---------------------------------- Print error -----------------------------------------

void error [[noreturn]] (std::string msg) {
    perror(msg.c_str());
    exit(1);
}

/**
 * @brief HEADER
 * HTTP response header
 */
static const std::string HEADER = "HTTP/1.1 200 OK\r\n"
                                  "Connection: close\r\n"
                                  "Max-Age: 0\r\n"
                                  "Expires: 0\r\n"
                                  "Cache-Control: no-cache, private\r\n"
                                  "Pragma: no-cache\r\n"
                                  "Content-Type: multipart/x-mixed-replace;boundary=--boundary\r\n\r\n";

/**
 * @brief HEADER
 * HTTP response BODY. To be completed before sending
 */
static const std::string BODY = "--boundary\r\n"
                                "Content-Type: image/jpeg\r\n"
                                "Content-Length: ";

/**
 * @brief init_server
 * @param port
 * @return a socketaddr_in struct that contain server information
 */
sockaddr_in init_server(uint16_t port) {
    sockaddr_in s;
    bzero(reinterpret_cast<char*>(&s), sizeof(s));
    s.sin_family = PF_INET;
    s.sin_addr.s_addr = inet_addr("0.0.0.0");
    s.sin_port = htons(port);
    return s;
}

//---------------------------------- Camera handle -----------------------------------------

/**
 * @brief The Camera handler class
 * Handle an OpenCV VideoCapture object in order to capture frames and convert them in jpg
 */
class Camera {
private:
    std::mutex l;
    cv::VideoCapture c;
    cv::Mat get_frame() { cv::Mat m; l.lock(); c >> m; l.unlock(); return m; }

public:
    Camera(int n) : c(n) {}
    Camera(std::string n) : c(n) {}
    ~Camera() { c.release(); }
    std::string jpg_encode() {
        std::vector<uchar> buf;
        cv::imencode(".jpg", get_frame(), buf);
        return std::string(buf.begin(), buf.end());
    }
};

//---------------------------------- Streamer class -----------------------------------------

/**
 * @brief The Streamer class
 * Streamer is the "server". It receive HTTP requests and send responses
 */
class Streamer {
private:
    Camera cam;
    int server, client;
    sockaddr_in s_addr, c_addr;

    std::thread t;
    std::vector<std::thread*> tasks;
    std::vector<std::atomic<bool>*> ended_t;
    size_t get_pos();

    void bind_channel() { if (bind(server, reinterpret_cast<sockaddr*>(&s_addr), sizeof(s_addr)) < 0) error("binding_failed"); }
    void send_datas(int socket, size_t pos);
    void run();

public:
    void start(){ bind_channel(); listen(server, 5); t = std::thread(&Streamer::run, this); }
    void stop() { shutdown(server, SHUT_RDWR); t.join(); }
    Streamer(uint16_t port, int cam_addr) : cam(cam_addr), server(socket(PF_INET, SOCK_STREAM, 0)), s_addr(init_server(port)) {}
    Streamer(uint16_t port, std::string cam_addr) : cam(cam_addr), server(socket(PF_INET, SOCK_STREAM, 0)), s_addr(init_server(port)) {}
    ~Streamer() { for (size_t i = 0; i < tasks.size(); ++i) { if (tasks[i]->joinable()) tasks[i]->join(); delete tasks[i]; delete ended_t[i];}}
};

size_t Streamer::get_pos() {
    for (size_t i = 0; i < tasks.size(); ++i)
        if (*ended_t[i]) {
            if (tasks[i]->joinable())
                tasks[i]->join();
            *ended_t[i] = false;
            return i;
        }
    tasks.push_back(nullptr);
    ended_t.push_back(new std::atomic_bool(false));
    return tasks.size() - 1;
}

void Streamer::send_datas(int socket, size_t pos) {
    std::string image, buffer = HEADER;
    while ((send(socket, buffer.c_str(), buffer.size(), MSG_NOSIGNAL)) > 0) {
        image = cam.jpg_encode();
        buffer = BODY + std::to_string(image.size()) + "\r\n\r\n" + image;
    }
    *ended_t[pos] = true;
    close(socket);
}

void Streamer::run() {
    while(true) {
        socklen_t c_length = sizeof(c_addr);
        if ((client = accept(server, reinterpret_cast<sockaddr*>(&c_addr), &c_length)) > 0) {
            size_t p = get_pos();
            tasks[p] = new std::thread(&Streamer::send_datas, this, client, p);
        } else
            break;
    }
    close(server);
}

#endif // STREAMER_H
