#include "http_server_header.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>



int build_http_response(const char *file_name, const char *file_ext, char *response, size_t *response_len, int buffer_size) {

    //to może zostać - bardzo prosta funkcja
    const char *mime_type = get_mime_type(file_ext);


    char *header = (char *)malloc(buffer_size * sizeof(char));

    //Możliwe że warto poczytać jaki jeszcze header można tu wstawić.
    snprintf(header, buffer_size,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "\r\n",
            mime_type);

    int file_fd = open(file_name, O_RDONLY);
    if(file_fd == -1) {
        snprintf(response, buffer_size,
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "404 Not Found");
        *response_len = strlen(response);
        return -1; //jeśli plik jest w bazie danych ale nie można go z jakiegoś powodu otworzyć.
    }


    struct stat file_stat;

    fstat(file_fd, &file_stat);

    off_t file_size = file_stat.st_size;

    *response_len = 0;

    memcpy(response, header, strlen(header));

    *response_len += strlen(header);

    ssize_t bytes_read;

    while((bytes_read = read(file_fd, response + *response_len, buffer_size - *response_len)) > 0) {
        *response_len += bytes_read;
    }

    free(header);

    close(file_fd);
    
    return 0;
}
