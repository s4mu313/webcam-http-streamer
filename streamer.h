#ifndef STREAMER_H
#define STREAMER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include "camera.h"
#include <thread>


class Streamer {

private:
    int server, client;
    struct sockaddr_in serverAddress, clientAddress;
    Camera* camera;

    void error(std::string msg);
    void bindChannel();
    static void stream(Camera &cam, int socket);

public:
    void start();
    void stop();

    Streamer(int port = 3851, int cameraAddress = 0);
    Streamer(int port, std::string cameraAddress);
    ~Streamer();
};

#endif // STREAMER_H
