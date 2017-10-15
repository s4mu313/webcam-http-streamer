#include "streamer.h"
#include <string>
#include <iostream>


int main(int argc, char *argv[])
{
    int port;
    int address;
    if (argc > 2) {
        port = std::atoi(argv[1]);
        address = std::atoi(argv[2]);
        std::cout << "Wrapping camera " << address << " su porta: " << port << std::endl;
        Streamer s(port, address);
        s.start();
    } else
        std::cout << "Argomenti mancanti. Inserisci porta e numero porta usb" << std::endl;
    return 0;
}
