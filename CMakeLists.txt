#cmake min version
cmake_minimum_required(VERSION 3.13)

#project name and version
project(http_server VERSION 1.6)

configure_file(http_server_header.h.in ${PROJECT_SOURCE_DIR}/http_server_header.h)

add_executable(http_server.out http_server.c)

#lib with function http_handler
add_library(http_handler http_handler.c)
set_target_properties(http_handler PROPERTIES PUBLIC_HEADER http_server_header)

#lib_with_function handle_arguments
add_library(argument_handler argument_handler.c)
set_target_properties(argument_handler PROPERTIES PUBLIC_HEADER http_server_header)


#lib with function build_http_response
add_library(http_build_response http_build_response.c)
set_target_properties(http_build_response PROPERTIES PUBLIC_HEADER http_server_header)

#lib with helper functions
add_library(helper_functions helper_functions.c)
set_target_properties(helper_functions PROPERTIES PUBLIC_HEADER http_server_header)

#lib for interacting with data base
add_library(db_interface db_interface.c)
set_target_properties(db_interface PROPERTIES PUBLIC_HEADER http_server_header)

add_library(sqlite STATIC sqlite3.c)
set_target_properties(sqlite PROPERTIES PUBLIC_HEADER sqlite3)
set_target_properties(sqlite PROPERTIES LINKER_LANGUAGE C)

add_library(PPHP STATIC PPHP.c)
set_target_properties(PPHP PROPERTIES PUBLIC_HEADER PPHP)
set_target_properties(PPHP PROPERTIES LINKER_LANGUAGE C)

add_library(NAS_API STATIC NAS_API.c)
set_target_properties(NAS_API PROPERTIES PUBLIC_HEADER NAS_API)
set_target_properties(NAS_API PROPERTIES LINKER_LANGUAGE C)

#lib for base64 decode
add_library(base64 base64.c)
set_target_properties(base64 PROPERTIES PUBLIC_HEADER http_server_header)

target_include_directories(http_server.out PUBLIC ${PROJECT_BINARY_DIR})

target_link_libraries(http_server.out PUBLIC http_handler argument_handler http_build_response helper_functions db_interface base64 sqlite PPHP NAS_API)