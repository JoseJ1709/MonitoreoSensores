cmake_minimum_required(VERSION 3.27)
project(MonitoreoSensores)

set(CMAKE_CXX_STANDARD 17)

add_executable(MonitoreoSensores main.cpp
        sensor.cpp
        monitor.cpp
        buffer.cpp
        buffer.h)
