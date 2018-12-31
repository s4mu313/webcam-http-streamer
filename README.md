# webcam-http-streamer
A simple C++ MJPEG streamer with C socket and OpenCV

## How to compile on Raspberry PI with ArchLinux installed
In this project std::thread and OpenCV has been used so you must add lib references to the compiler (and install necessary dependences).

```
g++ main.cpp -o <output_file_name> `pkg-config --cflags --libs opencv` -std=c++17 -pthread
```

## How to compile on Desktop with ArchLinux installed
I wrote this project with QtCreator under ArchLinux, so if you want to compile it, you have to write the .pro file in this way:

```
TEMPLATE = app
CONFIG += console c++0x thread
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv4

HEADERS += \
    streamer.h
```

##Usage

###To create the server

cam_id is the number of the device connected to USB (ID are sequential: 0, 1, 2,...)

```
size_t cam_id = 0; 
size_t port = 1234;

Streamer s(port, cam_id);
```

###To start and stop the server

Start() is asynchronous, when you call it, the server launch a thread to receive the request and send response

```
s.start();
...
s.stop();
```
