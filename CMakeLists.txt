cmake_minimum_required(VERSION 3.16)
project(untitled3 C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled3 main.c parser.c parser.h fifo.c fifo.h)