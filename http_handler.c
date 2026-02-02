#include "http_server_header.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <regex.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>


const char *get_file_extension(const char *filename) {

    const char *dot = strrchr(filename, '.');

    if(!dot || dot == filename) {
        return "";
    }

    return dot + 1;

}

const char *get_mime_type(const char *file_ext) {
    if(strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext,"htm") == 0) {
        return "text/html";    
    }
    else if(strcasecmp(file_ext, "txt") == 0) {
        return "text/plain";
    }
    else if(strcasecmp(file_ext, "png") == 0) {
        return "image/png";
    }
    else if(strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0) {
        return "image/jpeg";
    }
    else {
        return "application/octet-stream";
    }

}

bool case_insensitive_compare(const char *word1, const char *word2) {
    while(*word1 && *word2) {
        if(tolower((unsigned char)*word1) != tolower((unsigned char)*word2)) {
            return false;
        }
        word1++;
        word2++;
    }

    return *word1 == *word2;
}


char *get_file_case_insensitive(const char *file_name) {
    DIR *dir = opendir(".");
    if(dir == NULL) {
        fprintf(stderr, "No valid dir");
        return NULL;
    }

    struct dirent *entry;
    char *found_file_name = NULL;
    while ((entry = readdir(dir)) != NULL ) {
        if(case_insensitive_compare(entry->d_name, file_name)) {
            found_file_name = entry->d_name;
            break;
        }
    }

    closedir(dir);
    return found_file_name;

}

char *url_decode(const char *src) {
    size_t src_len = strlen(src);

    char *decoded = (char *)malloc(src_len + 1);

    size_t decoded_len = 0;

    for(size_t i = 0; i < src_len; i++) {
        if(src[i] == '%' && i+2 < src_len) {
            int hex_val;
            sscanf(src + i + 1, "%2x", &hex_val);
            decoded[decoded_len++] == hex_val;
            i += 2;
        }
        else {
            decoded[decoded_len++] = src[i];
        }
    }

    decoded[decoded_len] = '\0';
    return decoded;
}


void build_http_response(const char *file_name, const char *file_ext, char *response, size_t *response_len, int buffer_size) {

    const char *mime_type = get_mime_type(file_ext);


    char *header = (char *)malloc(buffer_size * sizeof(char));

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
        return;
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
}



void *handle_client(void *arg) {
    
    chdir(PAGES);
    //to jest samo w sobie okej, ale trzeba sprawdzać czy jesteśmy w pages
    //pages powinno być ustawiane podczas instalacji

    int client_fd = *((int *)arg);
    //socket

    int buffer_size = DEFAULT_BUFFER_SIZE;
    //miejsce na mechanizm przypisujący ine wartości
    ////////////////////////////////////

    char *buffer = (char *)malloc(buffer_size * sizeof(char));
    //miejsce na przychodzącą wiad


    //odebranie wiad
    ssize_t bytes_received = recv(client_fd, buffer, buffer_size, 0);
    printf("request recvd\n");


    if(bytes_received > 0) {
        regex_t regex;
        regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);

        regmatch_t matches[2];

        if(regexec(&regex, buffer, 2, matches, 0) == 0) {

            buffer[matches[1].rm_eo] = '\0';

            const char *url_encoded_file_name = buffer + matches[1].rm_so;

            char *file_name = url_decode(url_encoded_file_name);

            char file_ext[32];
            strcpy(file_ext, get_file_extension(file_name));

            char *response = (char *)malloc(buffer_size * 2 * sizeof(char));
            size_t response_len = strlen(response);

            build_http_response(file_name, file_ext, response, &response_len, buffer_size);

            send(client_fd, response, response_len, 0);
            printf("response sent\n");

            free(response);
            free(file_name);

        }
        regfree(&regex);

    }
    close(client_fd);
    free(arg);
    free(buffer);


    return NULL;
}


