#include <iostream>
#include "streamer.h"


int main(int argc, char *argv[])
{
    int port, cam_id;
    if (argc > 2) {
        port = std::atoi(argv[1]);
        cam_id = std::atoi(argv[2]);
        std::cout << "Binding camera on port: " << port << std::endl;
        Streamer s(3852, "http://192.168.1.67:3851");
        s.start();

        std::string str;
        std::cout << "Write something to exit" << std::endl;
        std::cin >> str;

        s.stop();

    } else
        std::cout << "Missed arguments. Insert port and number of camera" << std::endl;
    return 0;
}
