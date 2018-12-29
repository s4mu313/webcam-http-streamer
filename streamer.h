#ifndef STREAMER_H
#define STREAMER_H

#include <opencv2/highgui/highgui.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <mutex>
#include <future>


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
    s.sin_addr.s_addr = inet_addr("127.0.0.1");
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

//---------------------------------- Some functions ----------------------------------------

/**
 * @brief is_ready
 * @param f
 * @return true if the std::future has complete its compute, false otherwise
 */
template<typename T>
bool is_ready(std::future<T> const& f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }

static const int nullpos = -1;

/**
 * @brief first_free
 * @param f
 * @return the position of the first free std::future in a container
 */
template<typename V>
int first_free(V& f) {
    for (size_t i = 0; i < f.size(); ++i)
        if (is_ready(f[i])) {
            f[i].get();
            return int(i);
        }
    return nullpos;
}

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
    void bind_channel() { if (bind(server, reinterpret_cast<sockaddr*>(&s_addr), sizeof(s_addr)) < 0) error("binding_failed"); }
    void send_datas(int socket);
    void run();

public:
    void start(){ bind_channel(); listen(server, 5); t = std::thread(&Streamer::run, this); }
    void stop() { shutdown(server, SHUT_RDWR); t.join(); }
    Streamer(uint16_t port, int cam_addr) : cam(cam_addr), server(socket(PF_INET, SOCK_STREAM, 0)), s_addr(init_server(port)) {}
    Streamer(uint16_t port, std::string cam_addr) : cam(cam_addr), server(socket(PF_INET, SOCK_STREAM, 0)), s_addr(init_server(port)) {}

};

void Streamer::send_datas(int socket) {
    std::string image, buffer = HEADER;
    while ((send(socket, buffer.c_str(), buffer.size(), MSG_NOSIGNAL)) > 0) {
        image = cam.jpg_encode();
        buffer = BODY + std::to_string(image.size()) + "\r\n\r\n" + image;
    }
    close(socket);
}

void Streamer::run() {
    std::vector<std::future<void>> tasks;
    while(true) {
        socklen_t c_length = sizeof(c_addr);
        if ((client = accept(server, reinterpret_cast<sockaddr*>(&c_addr), &c_length)) > 0) {
            if (int pos = first_free(tasks) == nullpos)
                tasks.push_back(std::async(std::launch::async, &Streamer::send_datas, this, client));
            else
                tasks[static_cast<size_t>(pos)] = std::async(std::launch::async, &Streamer::send_datas, this, client);
        } else {
            for (auto& e : tasks)
                e.get();
            break;
        }
    }
    close(server);
}

#endif // STREAMER_H
