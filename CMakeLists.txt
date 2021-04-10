cmake_minimum_required(VERSION 3.15)
project(c_exam C)

set(CMAKE_C_STANDARD 99)

add_executable(c_exam SpreaderDetectorBackend.c SpreaderDetectorParams.h)