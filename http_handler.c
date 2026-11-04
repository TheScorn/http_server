    #include "http_server_header.h"
#include "PPHP.h"
#include "NAS_API.h"
#include <string.h>
#include <strings.h>
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
#include <time.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Function handling http clients after connection is established
 * 
 * To do
 * 
 * @param arg void pointer representing the struct of type handle_args_struct
 */
void *handle_client(void *arg) {
    
  
    chdir(PAGES);
    struct handle_args_struct* args = arg;
  

    bool NAS_connection = args->test_passed;
    bool verbose = args->verbose_init;
    int client_fd = args->client_fd;
    int session_id_length = args->session_id_length;
    int session_id_lifespan = args->session_id_lifespan;
    int uri_max_len = args->uri_max_len;
    struct sockaddr_in NAS_add;
    memcpy(&NAS_add, &(args->NAS_add), sizeof(struct sockaddr_in));



    free(args);

    #ifdef DEBUG
    printf("DEBUG mode: args assigned and freed.\n");
    #endif
    //trzeba zaimplementować sprawdzanie typu połączenia
    //rozbić tą funkcję na kilka mniejszych
    //0: NONE - nierozpoznane połączenie
    //1: HTTP request
    //2: Login request
    //3: Logout request

    ////////////////////////////////////////////////////////////////

    int buffer_size = DEFAULT_BUFFER_SIZE;
    char *buffer = (char *)malloc(buffer_size * sizeof(char));
    //miejsce na przychodzącą wiad


    //odebranie wiad
    ssize_t bytes_received = recv(client_fd, buffer, buffer_size, 0);
    //czy cokolwiek odebrane
    if(bytes_received == 0) {
        free(buffer);
        close(client_fd);
        return NULL;
    }

    //////////////////////sprawdzamy czy to typ 1
    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];
    
    if(regexec(&regex, buffer, 2, matches, 0) == 0) {
        //czy wyciągamy scieżkę przed funkcją czy przekazujemy matches?

        //przekazanie matches daje lepszą modularność, można wtedy spróbować zrobić to samo w login, logout
        http_get_routine(client_fd, buffer, matches, uri_max_len, DB_PATH);
        close(client_fd);
        free(buffer);
        return NULL;
    }

    //////////////////////////connection_type LOGIN
    regcomp(&regex, "^POST[[:space:]]+/[[:space:]]+HTTP/1\\.[01]\r?\n(.|\n)*Content-Type:[[:space:]]*multipart/form-data;[[:space:]]*boundary=([^\r\n]+)(.|\n)*name=\"login\"\r?\n\r?\n([^\r\n]+)(.|\n)*name=\"password\"\r?\n\r?\n([^\r\n]+)", REG_EXTENDED | REG_NEWLINE);
    if(regexec(&regex, buffer, 2, matches, 0) == 0) {
        http_login_routine(client_fd, buffer, session_id_length, session_id_lifespan, DB_PATH);
        close(client_fd);
        free(buffer);
        return NULL;
    }

    
    //sprawdzanie czy typ 3
    //POST bo GET nie powinno zmieniać stanu strony
    regcomp(&regex, "^POST[[:space:]]/logout[[:space:]]HTTP/1.1", REG_EXTENDED);
    if(regexec(&regex, buffer, 2, matches, 0) == 0) {
        http_logout_routine(client_fd, buffer, DB_PATH);
        close(client_fd);
        free(buffer);
        return NULL;
    }

    regcomp(&regex, "^POST[[:space:]]/NAS/(LIST|GET|PUT|DEL|MKDIR)/([^ ]*)[[:space:]]+HTTP/1.1.*\r\nCookie: sessionId=([^ ]+)*([\r\n|\r|\n]|$)", REG_EXTENDED);
    regmatch_t NAS_matches[4];
    if(regexec(&regex, buffer, 4, NAS_matches, 0) == 0) {
        http_NAS_routine(client_fd, buffer, NAS_matches, session_id_length, &NAS_add, DB_PATH);
        close(client_fd);
        return NULL;
    }



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //nieznane requesty
    else {

        #ifdef DEBUG
        printf("DEBUG mode: else entered.");
        #endif


        if(verbose) {
            fprintf(stderr, "Unknown request, no data sent.\n");
        }

        send_http_400_Bad_Request(client_fd, "Unknown request.");

        close(client_fd);
        free(buffer);
        return NULL;
    }
    
    



    
    close(client_fd);
    return NULL;
}


