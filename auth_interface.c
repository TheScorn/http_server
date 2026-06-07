#include "http_server_header.h"
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

/**
 * @brief Function testing connection to authorization server.
 * 
 * Function assumes that auth server is hosted locally.
 * In the future it migh include auth server address
 * to enable remote hosting.
 * 
 * Function sends TEST_STRING message to auth server. 
 * Expects TEST_ACK message on recv.
 * 
 * @param auth_port port of the authorization server.
 * 
 * 
 * @return 0 if connection successful, -1 if socket could not be created,
 * -2 if connection failed, -3 if no bytes were received, -4 if ack message was incorrect.
 */
int test_auth_con(uint16_t auth_port) {

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket == -1) {
        close(clientSocket);
        return -1;
    }

    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(auth_port);
    inet_pton(AF_INET, "0.0.0.0", &servAddr.sin_addr);

    int connection_status = connect(clientSocket, (const struct sockaddr*)&servAddr, sizeof(servAddr));
    if(connection_status == -1) {
        close(clientSocket);
        return -2;
    }
    
    char test_message[2048];
    snprintf(test_message, 2048, "TEST_STRING");

    send(clientSocket, test_message, sizeof(test_message), 0);

    ssize_t bytes_recvd = recv(clientSocket, test_message, sizeof(test_message), 0);
    close(clientSocket);
    
    if(bytes_recvd > 0) {
        if(strcasecmp(test_message, "TEST_ACK") == 0) {
            return 0;
        }
        else {
            return -4;
        }
    }
    else {
        return -3;
    }


    
    

    

}

