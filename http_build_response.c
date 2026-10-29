#include "http_server_header.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>


/**
 * @brief Function for loading file into a buffer.
 * 
 * @param file_path char pointer to null-terminated string where path to file is stored.
 * 
 * @param file pointer to char pointer used to store loaded file.
 * After successful execution memory will be dynamically allocated and needs to be manually freed.
 * 
 * @return 0 if execution successful, -1 if file could not be opened.
 */
int load_file(char* file_path, char** file) {
    
    int file_fd = open(file_path, O_RDONLY);
    if(file_fd == -1) {
        return -1;
    }

    struct stat file_stat;
    fstat(file_fd, &file_stat);
    off_t file_size = file_stat.st_size;

    *file = (char*)malloc((size_t)file_size + sizeof(char) * 1);

    size_t content_len = 0;
    ssize_t bytes_red;
    while((bytes_red = read(file_fd, *file + content_len, file_size - content_len)) > 0) {
        content_len += bytes_red;
    }

    return 0;
}

/**
 * @brief adding header to http response
 * 
 * @param raw char pointer to null-terminated string with http response without header
 * 
 * @param file_ext const char pointer to file extension
 * 
 * @param processed pointer to char pointer where result is stored
 * 
 * @return 0
 */
int add_response_header(char* raw, const char* file_ext, char** processed) {

    const char* mime_type = get_mime_type(file_ext);

    size_t raw_len = strlen(raw);

    char date[50];
    http_current_time(date);
    char header[250];
    snprintf(header, sizeof(char) * 250, "HTTP/1.1 200 OK\r\n"
                                        "Date: %s\r\n"
                                        "Content-Type: %s\r\n"
                                        "Content-Length: %zu\r\n"
                                        "\r\n", date, mime_type, raw_len);
    
    
    size_t header_len = strlen(header);
    *processed = (char*)malloc(sizeof(char) * (raw_len + header_len + 1));
    snprintf(*processed, sizeof(char) * (raw_len + header_len + 1), "%s%s", header, raw);

    return 0;
}


/**
 * @brief Function encapsulating opening files and adding HTTP headers
 * 
 * @param file_path const char pointer to null-terminated string where path to the file is stored
 * 
 * @param file_ext const char pointer to null-terminated string where file extension is stored
 * 
 * @param response char pointer where the result of this function is stored.
 * After this function executes, this will point to dynamically allocated 
 * memory, which needs to be freed manually.
 * 
 * @return 0 if execution successful, -1 if file could not be opened.
 * 
 */
int build_http_response(const char *file_path, const char *file_ext, char **response) {

    const char *mime_type = get_mime_type(file_ext);
    

    int file_fd = open(file_path, O_RDONLY);
    if(file_fd == -1) {
        char date[50];
        http_current_time(date);
        *response = (char*)malloc(sizeof(char) * 250);
        snprintf(*response, 250,
                "HTTP/1.1 404 Not Found\r\n"
                "Date: %s\r\n"
                "Content-Type: text/plain\r\n"
                "404 Not Found", date);
        return -1; //jeśli plik jest w bazie danych ale nie można go z jakiegoś powodu otworzyć.
    }



    struct stat file_stat;

    fstat(file_fd, &file_stat);

    off_t file_size = file_stat.st_size;


    char* file = (char*)malloc((size_t)file_size + sizeof(char) * 1);
    //o ile się nie mylę to wielkość pliku nie zawiera null-terminatora


    size_t content_length = 0;
    ssize_t bytes_read;
    while((bytes_read = read(file_fd, file + content_length, file_size - content_length)) > 0) {
        content_length += bytes_read;
    }

    #ifdef DEBUG
    printf("DEBUG mode: Function build_http_response: file red.\n");
    #endif

    char header[250];
    char date[50];
    http_current_time(date);


    //Możliwe że warto poczytać jaki jeszcze header można tu wstawić.
    snprintf(header, sizeof(char) * 250,
            "HTTP/1.1 200 OK\r\n"
            "Date: %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "\r\n",
            date, mime_type, content_length);

    #ifdef DEBUG
    printf("DEBUG mode: Function build_http_response: header done: %s\n", header);
    #endif
    
    *response = (char*)malloc(file_size + (sizeof(char) * 251));

    snprintf(*response, file_size + (sizeof(char) * 251), "%s%s", header, file);


    free(file);

    close(file_fd);
    
    return 0;
}
