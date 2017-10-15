#include "streamer.h"
#include <cstring>
#include <sys/types.h>
#include <unistd.h>



void Streamer::stream(int socket, int index)
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
        image = camera->getFrameInByteArray();
        buffer = "--boundary\r\n"
                 "Content-Type: image/jpeg\r\n"
                 "Content-Length: " + std::to_string(image.size()) + "\r\n\r\n" + image;
        check = send(socket, buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
    }
    std::cout << "Chiudo connessione" << std::endl;
    close(socket);
    threadTerminated[index] = true;
}

int Streamer::getFirstFreePos()
{
    int pos = -1;
    for (size_t i = 0; i < threadTerminated.size(); ++i)
        if (threadTerminated[i])
        {
            if (threadList[i] != nullptr) {
                threadList[i]->join();
                delete threadList[i];
                std::cout << "Ho liberato thread finito in pos: " << i << std::endl;
            }
            pos = i;
            break;
        }
    return pos;
}

void Streamer::stop()
{
    shutdown(server, SHUT_RDWR);
    close(server);
}

void Streamer::start()
{
    std::cout << "Bind..." << std::endl;
    bindChannel();
    std::cout << "Listen..." << std::endl << std::endl;
    listen(server, 5);

    std::thread ([this]{
        std::string str;
        std::cout << "Scrivi a caso per uscire" << std::endl;
        std::cin >> str;
        this->stop();
    }).detach();

    while(true) {
        socklen_t clientLenght = sizeof(clientAddress);
        client = accept(server, (struct sockaddr *) &clientAddress, &clientLenght);
        if (client > 0) {
            int pos = getFirstFreePos();
            if (pos == -1)
                std::cout << "Maximum number of connections reached" << std::endl;
            else {
                threadTerminated[pos] = false;
                threadList[pos] = new std::thread(&Streamer::stream, this, client, pos);
                std::cout << "Creo thread in pos: " << pos << std::endl;
            }
        }
        else
            break;
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
    for (auto& v : threadTerminated)
        v = true;
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
        threadList[i] = nullptr;
    std::cout << "Initialize camera " << cameraAddress << std::endl;
    camera = new Camera(cameraAddress);
    server = socket(PF_INET, SOCK_STREAM, 0);
    bzero((char*) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = PF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
}

Streamer::Streamer(int port, std::string cameraAddress)
{
    for (auto& v : threadTerminated)
        v = true;
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
        threadList[i] = nullptr;
    camera = new Camera(cameraAddress);
    server = socket(PF_INET, SOCK_STREAM, 0);
    bzero((char*) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = PF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
}

