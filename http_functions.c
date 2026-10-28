#include "http_server_header.h"
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

/**
 * @brief Sending routine
 * 
 * Function checks strlen of message and, using while loop, sends all bytes
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @param message pointer to null-terminated string user want to send
 * 
 * @return 0 if execution was successful, -1 if error occured while sending.
 */
int send_routine(int client_fd, char* message) {
    
    size_t message_len = strlen(message);

    size_t total = 0;
    while(total < message_len) {
        ssize_t n  = send(client_fd, message + total, message_len - total, 0);
        if(n <= 0) {
            return -1;
        }
        total += n;
    }

    return 0;
}


/**
 * @brief HTTP Date
 * 
 * Function produces HTTP formated GMT date and time.
 * 
 * @param date address for the date
 * 
 * @return 0 if execution successful
 */
int http_current_time(char* date) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    strftime(date, 50, "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return 0;

}


/**
 * @brief Send Internal Server Error message.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @param error pointer to null-terminated error message. If this is a NULL-pointer, the response has no content body.
 * 
 * @return 0 if execution was successful, -1 if error occured while sending.
 */
int send_http_internal_server_error(int client_fd, char* error) {
    char date[50];
    http_current_time(date);

    if(error != NULL) {
        size_t error_len = strlen(error);

        char response[300 + error_len];

        snprintf(response, sizeof(char) * (300 + error_len), "HTTP/1.1 500 Internal Server Error\r\n"
                                                            "Date: %s\r\n"
                                                            "Content-Type: text/plain; charset=utf-8\r\n"
                                                            "Content-Length: %zu\r\n"
                                                            "Connection: close\r\n"
                                                            "\r\n"
                                                            "%s", date, error_len, error);

        if(send_routine(client_fd, response) != 0) {
            return -1;
        }

    }   
    else {
        char response[250];

        snprintf(response, sizeof(char) * 250, "HTTP/1.1 500 Internal Server Error\r\n"
                                            "Date: %s\r\n"
                                            "Content-Length: 0\r\n"
                                            "Connection: close\r\n\r\n", date);

        
        if(send_routine(client_fd, response) != 0) {
            return -1;
        }
        return 0;
    }
}



/**
 * @brief Redirect to login_page
 * 
 * Function is used when the user is not logged in and requests a resource that demands that.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @return 0 if execution was successful, -1 if error occured inside send_routine.
 */
int send_http_found_login(int client_fd) {
    char response[200];
    char date[50];
    http_current_time(date);

    snprintf(response, sizeof(char) * 200,
            "HTTP/1.1 302 Found\r\n"
            "Date: %s\r\n"
            "Location: /login_page.html", date);

    size_t response_len = strlen(response);

    if(send_routine(client_fd, response) != 0) {
        return -1;
    }
    return 0;
}

/**
 * @brief Redirect to main (with optional cookie clearing)
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @return 0 if execution was successful, -1 if error occured inside send_routine.
 */
int send_http_found_main(int client_fd, bool clear_cookie) {
    char response[250];
    char date[50];
    http_current_time(date);

    size_t response_len;

    if(clear_cookie) {
        snprintf(response, sizeof(char) * 250, "HTTP/1.1 302 Found\r\n"
                                                "Location: main_page.html\r\n"
                                                "Date: %s\r\n"
                                                "Set-Cookie: session=; Max-Age=0; Path=/; HttpOnly; SameSite=Lax\r\n", date);
        
        response_len = strlen(response);
    }
    else {
        snprintf(response, sizeof(char) * 250, "HTTP/1.1 302 Found\r\n"
                                                "Date: %s\r\n"
                                                "Location: main_page.html", date);
        response_len = strlen(response);
    }

    if(send_routine(client_fd, response) != 0) {
        return -1;
    }
    return 0;
}

