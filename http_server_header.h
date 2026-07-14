#define HTTP_SERVER_VERSION_MAJOR 1
#define HTTP_SERVER_VERISON_MINOR 2

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H


struct input_args_struct {
    bool verbose_init;
    bool print_help;
    uint16_t selected_port;
    int token_length;
};

struct handle_args_struct {
    int * client_fd;
    int token_length;
    bool verbose_init;
};

struct file_info_struct {
    char* file_path;
    int zone_id;
    char* zone_name;
    int zone_type;
};

struct client_info_struct {
    bool logged_in;
    char* name;
    char* email;
    char* password;
    int access_flags;
};

void *handle_client(void *arg);

const char *get_file_extension(const char *filename);

const char *get_mime_type(const char *file_ext);

bool case_insensitive_compare(const char *word1, const char *word2);

char *get_file_case_insensitive(const char *file_name);

char *url_decode(const char *src);

int build_http_response(const char *file_name, const char *file_ext, char *response, size_t *response_len, int buffer_size);

int handle_arguments(int argc, char **argv,struct input_args_struct *args);

int get_file_info(char *filename, struct file_info_struct* page_info);

int get_client_info(char* name, char* password, struct client_info_struct* client_info);

unsigned char * base64_decode(const unsigned char *src, size_t len, size_t *out_len);

char * get_username(char * authorization);

char * get_password(char * authorization);

int test_con();

void generate_token(int length, char* token);

#endif

#define VERBOSE_INIT_DEFAULT false
#define VERBOSE_INPUT_DEFAULT false
#define PRINT_HELP_DEFAULT false
#define DEFAULT_PORT 54001
#define PAGES "./pages"
#define DEFAULT_BUFFER_SIZE 104857600
#define DB_HOST "localhost"
#define DB_USER "http_server"
#define DB_PASSWORD "passwd"
#define DB "HTTP_SERVER_INFO"
#define DEFAULT_TOKEN_LENGTH 24
