#include <iostream>
#include "streamer.h"


int main(int argc, char *argv[])
{
    uint16_t port, cam_id;
    if (argc > 2) {
        port = static_cast<uint16_t>(std::atoi(argv[1]));
        cam_id = static_cast<uint16_t>(std::atoi(argv[2]));
        std::cout << "Binding camera on port: " << port << std::endl;

        Streamer s(port, cam_id);
        s.start();

        std::cout << "Write something to exit" << std::endl;
        std::cin.get();

        s.stop();

    } else
        std::cout << "Missed arguments. Insert port and number of camera" << std::endl;
    return 0;
}
