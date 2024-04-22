cmake_minimum_required(VERSION 3.27)
project(MonitoreoSensores)

set(CMAKE_CXX_STANDARD 17)

add_executable(monitor monitor.cpp buffer.cpp)
target_link_libraries(monitor pthread)

add_executable(sensor sensor.cpp)
target_link_libraries(sensor pthread)