#include "streamer.h"
#include <cstring>
#include <sys/types.h>
#include <unistd.h>


void Streamer::stream(Camera &cam, int socket)
{
    std::string header =  "HTTP/1.1 200 OK\r\n"
                          "Connection: close\r\n"
                          "Max-Age: 0\r\n"
                          "Expires: 0\r\n"
                          "Cache-Control: no-cache, private\r\n"
                          "Pragma: no-cache\r\n"
                          "Content-Type: multipart/x-mixed-replace;boundary=--boundary\r\n\r\n";

    std::string image, buffer;
    int check = send(socket, header.c_str(), header.size(), MSG_NOSIGNAL);
    while (check > 0)
    {
        image = cam.getFrameInByteArray();
        buffer = "--boundary\r\n"
                 "Content-Type: image/jpeg\r\n"
                 "Content-Length: " + std::to_string(image.size()) + "\r\n\r\n" + image;
        check = send(socket, buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
    }
    close(socket);
}

void Streamer::stop()
{
    exit(EXIT_SUCCESS);
}

void Streamer::start()
{
    bindChannel();
    listen(server, 5);

    while(true) {
        socklen_t clientLenght = sizeof(clientAddress);
        client = accept(server, (struct sockaddr *) &clientAddress, &clientLenght);
        if (client < 0)
            error("Error accepting request");
        std::thread{Streamer::stream, std::ref(*camera), client}.detach();
    }
    close(server);
}

void Streamer::error(std::string msg)
{
    perror(msg.c_str());
    exit(1);
}

void Streamer::bindChannel()
{
    if (bind(server, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        error("Error binding operation");
}

Streamer::~Streamer()
{
    free(camera);
}

Streamer::Streamer(int port, int cameraAddress)
{
    camera = new Camera(cameraAddress);
    server = socket(PF_INET, SOCK_STREAM, 0);
    bzero((char*) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = PF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
}

Streamer::Streamer(int port, std::string cameraAddress)
{
    camera = new Camera(cameraAddress);
    server = socket(PF_INET, SOCK_STREAM, 0);
    bzero((char*) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = PF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
}

