cmake_minimum_required(VERSION 3.28)
project(computer_networks)

set(CMAKE_CXX_STANDARD 20)

add_executable(computer_networks main.cpp
        src/fifo.cpp
        src/fifo.h
        src/audio.cpp
        src/audio.h
        src/data.cpp
        src/data.h
        src/video.cpp
        src/video.h
        src/source.cpp
        src/source.h
        src/packet.cpp
        src/packet.h
        src/cdf.cpp
        src/cdf.h
)
