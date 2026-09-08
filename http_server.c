#define _POSIX_C_SOURCE 200809L

#include "http_server_header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

static volatile sig_atomic_t run;
static volatile int serverSocket;
static void sig_handler(int _);

int main(int argc, char **argv) {
    
    
    run = true;
    
    //obsługa argumentów funkcji main
    //zmiana portu
    //zmiana trybu
    //pomoc

    

    //struktura argumentów do przekazania do funkcji obsługującej zapytania
    struct input_args_struct input_args;
    input_args.verbose_init = VERBOSE_INIT_DEFAULT;
    input_args.print_help = false;
    input_args.selected_port = DEFAULT_PORT;
    input_args.session_id_length = DEFAULT_SESSION_ID_LENGTH;
    input_args.session_id_lifespan = DEFAULT_SESSION_ID_LIFESPAN;
    input_args.NAS_add.sin_port = htons(DEFAULT_NAS_PORT);
    inet_pton(AF_INET, DEFAULT_NAS_IP, &input_args.NAS_add.sin_addr);
    input_args.NAS_add.sin_family = AF_INET;

    int handle_arg_ret = handle_arguments(argc, argv, &input_args);
    if(handle_arg_ret != 0) {
        fprintf(stderr, "Error occured during argument processing. Error code: %d\n", handle_arg_ret);
        return -1;
    }

    if(input_args.print_help) {
        printf("######################################3\nHttp server\n###########################################\nVersion: %d.%d\nFlags:\n-h: Prints helper message.\nUsage: http_server.out -n\n-p: Sets port to be used by server.\nUsage: http_server.out -p [port number]\n-v: Turns on verbose mode.\nUsage: http_server.out -v\n",HTTP_SERVER_VERSION_MAJOR, HTTP_SERVER_VERISON_MINOR);
        return 0;
    }

    printf("Http Server initializing\n");


    
    //handle_args.verbose_init = input_args.verbose_init;
    //handle_args.session_id_length = input_args.session_id_length;
    //handle_args.session_id_lifespan = input_args.session_id_lifespan;

    if(input_args.verbose_init) {
        printf("Version: %d.%d\n", HTTP_SERVER_VERSION_MAJOR, HTTP_SERVER_VERISON_MINOR);
    }

    
    if(input_args.verbose_init) {
        printf("SessionID length: %d\n", input_args.session_id_length);
    }

    if(input_args.verbose_init) {
        printf("Session lifespan: %d minutes\n", input_args.session_id_lifespan);
    }

    //db connection test
    int test_db_con = test_con();
    if(test_db_con == -1) {
        fprintf(stderr, "No SQLite database found\n");
        return -1;
    }
    //connection successful
    if(input_args.verbose_init) {
        printf("Connection to database successful\n");
    }
    

    bool NAS_connection = true;
    //test połączenia z NAS
    if(input_args.verbose_init) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &input_args.NAS_add.sin_addr, ip, sizeof(ip));
        printf("Testing connection to NAS server at %s:%d\n", ip, ntohs(input_args.NAS_add.sin_port));
    }
    int test_NAS_connection_status = test_NAS_connection(&input_args.NAS_add);
    if(test_NAS_connection_status < 0) {
        fprintf(stderr, "Warning! test_NAS_connection returned with error code: %d. NAS connection could not be established.\n", test_NAS_connection_status);
        NAS_connection = false;
    }
    if(input_args.verbose_init && test_NAS_connection_status == 0) {
        printf("Connection established, TEST passed.\n");
    }

    

    //create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == -1) {
        fprintf(stderr, "Error occured while creating socket\n");
        close(serverSocket);
        return -1;
    }
    else if(input_args.verbose_init) {
        printf("Socket created\n");
    }


    //binding
    struct sockaddr_in hint;
    memset(&hint , 0, sizeof(hint));
    hint.sin_family = AF_INET;
    hint.sin_port = htons(input_args.selected_port); //konwersja na big endian
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);//0.0.0.0 samo wybiera adres

    if(bind(serverSocket, (struct sockaddr*)&hint, sizeof(hint)) == -1) {
        fprintf(stderr, "Error occured while binding\n");
        close(serverSocket);
        return -2;
    }
    else if(input_args.verbose_init) {
        printf("Bind complete\n");
    }


    //accepting
    if(listen(serverSocket, SOMAXCONN) == -1)  {//somaxconn max liczba połączń dla systemy
        fprintf(stderr, "Error occured while attepmting to listen");
        close(serverSocket);
        return -3;
    }
    else if(input_args.verbose_init) {
        printf("Listening on port: %d\n", input_args.selected_port);
    }


    /////////////////////////////////////////////////////////////
    //signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);


    //signal(SIGINT, sig_handler);
    //handle
    while(run) {

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int* client_fd = (int*)malloc(sizeof(int));
        //int client_fd;
        if((*client_fd = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            if(errno == EINTR && !run) {
                free(client_fd);
                break;
            }
            fprintf(stderr, "Accept failed.\n");
            continue;
        }

        struct handle_args_struct* handle_args = (struct handle_args_struct*)malloc(sizeof(struct handle_args_struct));
        if(handle_args == NULL) {
            fprintf(stderr, "No memory allocated for handle_args_struct");
            free(client_fd);
            break;
        }

        memcpy(&(handle_args->client_fd), client_fd, sizeof(int));
        memcpy(&(handle_args->session_id_length), &(input_args.session_id_length), sizeof(int));
        memcpy(&(handle_args->session_id_lifespan), &(input_args.session_id_lifespan), sizeof(int));
        memcpy(&(handle_args->verbose_init), &(input_args.verbose_init), sizeof(bool));

        pthread_t thread_id;

        
        //handle_args.client_fd = client_fd;
        
        //handle_args.client_fd = (int*)malloc(sizeof(int));
        //memcpy(handle_args.client_fd, client_fd, sizeof(int));
        
        pthread_create(&thread_id, NULL, handle_client, (void *)handle_args); //było wcześniej (void *)client_fd
        pthread_detach(thread_id);
        free(client_fd); //zwalniamy tą pamięć a tą ze structu zwolnimy w wątku
        
    }

    


    close(serverSocket);
    printf("\nSocket closed\n");
    int drop_all_status = drop_all_sessions();
    if(drop_all_status == -1) {
        fprintf(stderr, "Warning! Database could not be opened - sessions were not dropped.\n");
    }
    else {
        printf("Sessions dropped.\n");
    }
    printf("http server shutting down.\n");

}



static void sig_handler(int _) {
    (void)_;
    run = false;
}