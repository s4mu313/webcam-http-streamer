#ifndef STREAMER_H
#define STREAMER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include "camera.h"
#include <thread>
#include <array>
#include <atomic>


class Streamer {  
private:
    static const int MAX_CONNECTIONS = 16;
    int server, client;
    struct sockaddr_in serverAddress, clientAddress;
    Camera* camera;
    std::thread* threadList[MAX_CONNECTIONS];
    std::array<std::atomic<bool>, MAX_CONNECTIONS> threadTerminated;

    void error(std::string msg);
    void bindChannel();
    void stream(int socket, int index);
    int getFirstFreePos();

public:
    void start();
    void stop();

    Streamer(int port = 3851, int cameraAddress = 0);
    Streamer(int port, std::string cameraAddress);
    ~Streamer();
};

#endif // STREAMER_H
