cmake_minimum_required(VERSION 3.28)
project(computer_networks)

set(CMAKE_CXX_STANDARD 20)

add_executable(computer_networks main.cpp
        src/utils.cpp
        src/utils.h
        src/event.cpp
        src/event.h
        include/tomlcpp.cpp
        include/toml.c
        src/config.cpp
        src/config.h
)
