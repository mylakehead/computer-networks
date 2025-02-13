set(CMAKE_BUILD_TYPE Debug)
cmake_minimum_required(VERSION 3.28)
project(computer_networks)

set(CMAKE_CXX_STANDARD 20)

add_executable(computer_networks main.cpp
        src/utils.cpp
        src/utils.h
        src/source.cpp
        src/source.h
        include/tomlcpp.cpp
        include/toml.c
        src/config.cpp
        src/config.h
        src/queue.cpp
        src/queue.h
        src/runtime.h
)
