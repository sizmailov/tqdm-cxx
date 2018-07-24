cmake_minimum_required(VERSION 3.12)
project(tqdm)

set(CMAKE_CXX_STANDARD 17)


set(CMAKE_CXX_FLAGS " -O0 -g -fsanitize=address -fno-omit-frame-pointer ${CMAKE_CXX_FLAGS}")
add_executable(tqdm
        main.cpp
        include/tqdm/console_codes.h
        include/tqdm/tqdm.h
        include/tqdm/utils.h)