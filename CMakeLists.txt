cmake_minimum_required(VERSION 3.10)
project(classic)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(classic
        Base.h
        classic_tools.h
        Request.h
#        Substrate.h main.cpp)
        Substrate.h classic_main.cpp)
