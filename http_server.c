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

static volatile int serverSocket;
static void sig_handler(int _);

int main(int argc, char **argv) {
    
    
    
    bool verbose_init = VERBOSE_INIT_DEFAULT;
    bool verbose_input = VERBOSE_INPUT_DEFAULT;
    bool print_help = PRINT_HELP_DEFAULT;
    uint16_t selected_port = DEFAULT_PORT;
    int token_length = DEFAULT_TOKEN_LENGTH;
    //obsługa argumentów funkcji main
    //zmiana portu
    //zmiana trybu
    //pomoc

    

    //struktura argumentów do przekazania do funkcji obsługującej zapytania
    struct input_args_struct input_args;
    struct handle_args_struct handle_args;
    input_args.verbose_init = verbose_init;
    input_args.print_help = print_help;
    input_args.selected_port = selected_port;
    input_args.token_length = token_length;

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

    handle_args.verbose_init = input_args.verbose_init;
    handle_args.token_length = input_args.token_length;

    if(input_args.verbose_init) {
        printf("Version: %d.%d\n", HTTP_SERVER_VERSION_MAJOR, HTTP_SERVER_VERISON_MINOR);
    }

    
    if(input_args.verbose_init) {
        printf("Token length selected: %d\n", token_length);
    }


    //db connection test
    int test_db_con = test_con();
    if(test_db_con == -1) {
        fprintf(stderr, "MYSQL structure failed to initialize.\n");
        return -1;
    }
    else if(test_db_con == -2) {
        fprintf(stderr, "Connection test to database failed.\n");
        return -1;
    }

    //connection successful
    if(input_args.verbose_init) {
        printf("Connection to database successful.\n");
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


    


    signal(SIGINT, sig_handler);
    //handle
    while(true) {

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = (int *)malloc(sizeof(int));

        if((*client_fd = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            fprintf(stderr, "Accept failed.\n");
            continue;
        }


        

        pthread_t thread_id;

        
        handle_args.client_fd = client_fd;
        pthread_create(&thread_id, NULL, handle_client, (void *)&handle_args); //było wcześniej (void *)client_fd
        pthread_detach(thread_id);

    }


    close(serverSocket);
    printf("Socket closed\n");
    printf("http server shutting down.\n");

}



static void sig_handler(int _) {
    (void)_;
    close(serverSocket);
    printf("\nSocket closed\n");
    printf("Http Server shutting down.\n");
    exit(0);
}