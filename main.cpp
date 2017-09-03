#include <iostream>
#include "streamer.h"
#include "opencv2/opencv.hpp"

int main(int argc, char *argv[])
{
    Streamer s(3851, 0);
    s.start();
    return 0;
}
