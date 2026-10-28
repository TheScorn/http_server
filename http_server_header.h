#define HTTP_SERVER_VERSION_MAJOR 1
#define HTTP_SERVER_VERISON_MINOR 10

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <netinet/in.h>

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H


struct input_args_struct {
    bool verbose_init;
    bool print_help;
    uint16_t selected_port;
    int session_id_length;
    int session_id_lifespan;
    struct sockaddr_in NAS_add;
};

struct handle_args_struct {
    bool verbose_init;
    bool test_passed;
    int client_fd;
    int session_id_length;
    int session_id_lifespan;
    struct sockaddr_in NAS_add;
};

struct file_info_struct {
    char file_path[100];
    int zone_id;
    char zone_name[30];
    int zone_type;
};

struct client_info_struct {
    bool logged_in;
    int elevated;
    char name[30];
    char email[40];
    char password[30];
    char NAS_username[30];
    char NAS_password[30];
    
};

enum connection_type_en {
    UNKNOWN, GET, LOGIN, LOGOUT, NAS
};

void *handle_client(void *arg);

const char *get_file_extension(const char *filename);

const char *get_mime_type(const char *file_ext);

bool case_insensitive_compare(const char *word1, const char *word2);

char *get_file_case_insensitive(const char *file_name);

char *url_decode(const char *src);

int build_http_response(const char *file_path, const char *file_ext, char **response);

int load_file(char* file_path, char** file);

int add_response_header(char* raw, const char* file_ext, char** processed);


////////////////////////////////////////////////
//http_functions prototypes

int send_routine(int client_fd, char* message);

int http_current_time(char* date);

int send_http_found_login(int client_fd);

int send_http_found_main(int client_fd, bool clear_cookie);

int send_http_internal_server_error(int client_fd, char* error);

int send_http_bad_request(int client_fd);

int send_http_unauthorized(int client_fd);

int send_http_ok_set_cookie(int client_fd, char* session_id, int session_id_lifespan);

/////////////////////////////////////////////////
//http request handlers

int http_logout_routine(int client_fd, char* buffer);

int http_login_routine(int client_fd, char* buffer, int session_id_length, int session_id_lifespan);

/////////////////////////////////////////////////

int handle_arguments(int argc, char **argv,struct input_args_struct *args);

int get_file_info(char *filename, struct file_info_struct* page_info);

int get_client_info(char* name, char* password, struct client_info_struct* client_info);

unsigned char * base64_decode(const unsigned char *src, size_t len, size_t *out_len);

char * get_username(char * authorization);

char * get_password(char * authorization);

int test_con();

int test_NAS_connection(struct sockaddr_in* NAS_add);

#ifdef __cplusplus
extern "C" {
#endif

void generate_session_id(int length, char* token);

#ifdef __cplusplus
}
#endif


int authenticate(char* login, char* password, int* user_id);

int save_session_id(char* session_id, int user_id, time_t expiry);

int authorize(char* session_id, struct client_info_struct* client_info);

int authorize_NAS(char* session_id, struct client_info_struct* client_info);

int drop_all_sessions();

int drop_session(char* sesion_id);

int last_occurence(char* str, char chr);

#define VERBOSE_INIT_DEFAULT false
#define VERBOSE_INPUT_DEFAULT false
#define DEFAULT_PORT 54001
#define PAGES "./pages"
//zmieniamy bufor wejściowy na 100kB, to i tak jest aż nadto
#define DEFAULT_BUFFER_SIZE 102400
#define DB_HOST "localhost"
#define DB_USER "http_server"
#define DB_PASSWORD "passwd"
#define DB "HTTP_SERVER_INFO"
#define DEFAULT_SESSION_ID_LENGTH 24
#define DEFAULT_SESSION_ID_LIFESPAN 30
#define MAX_NUMBER_OF_PAGE_VAR 10
#define DEFAULT_NAS_IP "127.0.0.1"
#define DEFAULT_NAS_PORT 54004

#endif
