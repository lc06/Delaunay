cmake_minimum_required(VERSION 3.24)
project(delaunay)

set(CMAKE_CXX_STANDARD 17)
include_directories(D:/openglrepo/Include)
link_directories(D:/openglrepo/Libs)

add_executable(delaunay main.cpp glad.c config.h include/Delaunay.cpp include/Delaunay.h)
target_link_libraries(delaunay glfw3dll)