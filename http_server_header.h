#define HTTP_SERVER_VERSION_MAJOR 0
#define HTTP_SERVER_VERISON_MINOR 6

#include <stdio.h>
#include <stdbool.h>

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

void *handle_client(void *arg);

const char *get_file_extension(const char *filename);

const char *get_mime_type(const char *file_ext);

bool case_insensitive_compare(const char *word1, const char *word2);

char *get_file_case_insensitive(const char *file_name);

char *url_decode(const char *src);

void build_http_response(const char *file_name, const char *file_ext, char *response, size_t *response_len, int buffer_size);

struct args_struct {
    int* client_fd;
    bool verbose_init;
    bool verbose_input;
};


#endif

#define VERBOSE_INIT_DEFAULT true
#define VERBOSE_INPUT_DEFAULT true
#define DEFAULT_PORT 54001
#define PAGES "./pages"
#define DEFAULT_BUFFER_SIZE 104857600
