#minimalny cmake
cmake_minimum_required(VERSION 3.13)

#nazwa projektu
project(http_server VERSION 0.75)

configure_file(http_server_header.h.in ${PROJECT_SOURCE_DIR}/http_server_header.h)

add_executable(http_server.out http_server.c)

add_library(http_handler http_handler.c)
set_target_properties(http_handler PROPERTIES PUBLIC_HEADER http_server_header)

add_library(argument_handler argument_handler.c)
set_target_properties(argument_handler PROPERTIES PUBLIC_HEADER http_server_header)

target_include_directories(http_server.out PUBLIC ${PROJECT_BINARY_DIR})

target_link_libraries(http_server.out PUBLIC http_handler argument_handler)