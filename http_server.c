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

int main() {
    
    uint16_t selected_port = DEFAULT_PORT;

    

    //create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == -1) {
        fprintf(stderr, "Error occured while creating socket\n");
        close(serverSocket);
        return -1;
    }
    else {
        printf("Socket created\n");
    }


    //binding
    struct sockaddr_in hint;
    memset(&hint , 0, sizeof(hint));
    hint.sin_family = AF_INET;
    hint.sin_port = htons(selected_port); //konwersja na big endian
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);//0.0.0.0 samo wybiera adres

    if(bind(serverSocket, (struct sockaddr*)&hint, sizeof(hint)) == -1) {
        fprintf(stderr, "Error occured while binding");
        close(serverSocket);
        return -2;
    }
    else {
        printf("Bind complete\n");
    }


    //accepting
    if(listen(serverSocket, SOMAXCONN) == -1)  {//somaxconn max liczba połączń dla systemy
        fprintf(stderr, "Error occured while attepmting to listen");
        close(serverSocket);
        return -3;
    }
    else {
        printf("Listening on port: %d\n", selected_port);
    }


    //handle
    while(true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = (int *)malloc(sizeof(int));

        if((*client_fd = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            fprintf(stderr, "Accept failed");
            continue;
        }

        pthread_t thread_id;

        //pthread_create(&thread_id, NULL, http_handler, (void *)client_fd);
        //pthread_detach(thread_id);

    }


    close(serverSocket);
    printf("Socket closed\n");
    printf("http server shutting down.\n");

}
