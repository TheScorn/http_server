#minimalny cmake
cmake_minimum_required(VERSION 3.13)

#nazwa projektu
project(http_server VERSION 0.4)

configure_file(http_server_header.h.in ${PROJECT_SOURCE_DIR}/http_server_header.h)

add_executable(http_server.out http_server.c)

