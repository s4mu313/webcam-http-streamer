#include <iostream>
#include "streamer.h"
#include "opencv2/opencv.hpp"

int main(int argc, char *argv[])
{
    Streamer s(3851, "http://192.168.1.11:8001/live");
    s.start();
    return 0;
}
