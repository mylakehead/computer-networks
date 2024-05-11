cmake_minimum_required(VERSION 3.28)
project(computer_networks)

set(CMAKE_CXX_STANDARD 20)

add_executable(computer_networks main.cpp
        src/fifo.cpp
        src/fifo.h
        src/packet.cpp
        src/packet.h
        src/utils.cpp
        src/utils.h
        src/event.cpp
        src/event.h
)
