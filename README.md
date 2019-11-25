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

## Usage

### To create the server


```
std::size_t port = 5555;

http::Server s(port);
```

### Adding paths to the server

```

std::size_t n = 3;

s.get("/", [] (auto req, auto res) {
    for (const auto& h : req.headers()) {
        std::cout << h << std::endl;           /// Access to the headers
    }
}).get("/path", [&] (auto req, auto res) {
    n = 5;                                     /// Access to n by putting & in the lambda capture
}).get("/other", [] (auto req, auto res) {

    res >> "<html>"                            /// send header + message with osstream operator
           "    <body>"
           "        <h1>OTHER</h1>"
           "    </body>"
           "</html>";
           
           
    if (!res.send_msg("--boundary\r\n"         /// send just the message
                      "Content-Type: image/jpeg\r\n"
                      "Content-Length: " +
                      std::to_string(image.size()) +
                      "\r\n\r\n" +
                      image))
        return;

    res.headers.push_back("Max-Age: 0");        /// Add some headers
    res.headers.push_back("Expires: 0");
    res.headers.push_back("Pragma: no-cache");

    if (!res.send_header())                     /// Send just headers
        return;
        
        
})...;
```

### Start the server

To start the server call listen(). It is a blocking function.

```
s.listen();
```
