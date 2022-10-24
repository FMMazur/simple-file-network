cmake_minimum_required(VERSION 3.23)
project(ftp C)

set(CMAKE_C_STANDARD 99)

include_directories(common)

add_subdirectory(client)
add_subdirectory(server)