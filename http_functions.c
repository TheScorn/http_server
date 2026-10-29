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
 * Function is used when the user is not logged in and requests a resource that requires it.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @param clear_cookie boolean parameter that tells if clear-cookie header should be sent.
 * 
 * @return 0 if execution was successful, -1 if error occured inside send_routine.
 */
int send_http_found_login(int client_fd, bool clear_cookie) {
    char response[250];
    char date[50];
    
    size_t response_len;

    http_current_time(date);

    if(clear_cookie) {
        snprintf(response, sizeof(char) * 250,
            "HTTP/1.1 302 Found\r\n"
            "Location: /login_page.html\r\n"
            "Date: %s\r\n"
            "set-Cookie: session=; Max-Age=0; Path=/; HttpOnly; SameSite=Lax\r\n", date);
    }
    else {
        snprintf(response, sizeof(char) * 250,
            "HTTP/1.1 302 Found\r\n"
            "Date: %s\r\n"
            "Location: /login_page.html", date);  
    }
    
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
 * @param clear_cookie boolean parameter that tells if clear-cookie header should be sent.
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
         
    }
    else {
        snprintf(response, sizeof(char) * 250, "HTTP/1.1 302 Found\r\n"
                                                "Date: %s\r\n"
                                                "Location: main_page.html", date);
    }

    if(send_routine(client_fd, response) != 0) {
        return -1;
    }
    return 0;
}

/**
 * @brief Send Bad Request.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @return 0 if execution was successful, -1 if error occured while sending.
 */
int send_http_bad_request(int client_fd) {
    char date[50];
    http_current_time(date);

    char response[250];
    snprintf(response, sizeof(char) * 250, "HTTP/1.1 400 Bad Request\r\n"
                                            "Date: %s\r\n"
                                            "Content-Length: 0\r\n"
                                            "Connection: close\r\n"
                                            "\r\n", date);

    if(send_routine(client_fd, response) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Send 401 Unauthorized.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @return 0 if execution was successful, -1 if error occured while sending.
 */
int send_http_unauthorized(int client_fd) {
    char date[50];
    char response[250];
    http_current_time(date);

    snprintf(response, sizeof(char) * 250, "HTTP/1.1 401 Unauthorized\r\n"
                                            "Date: %s\r\n"
                                            "Content-Length: 0\r\n"
                                            "Connection: close\r\n\r\n", date);

    if(send_routine(client_fd, response) != 0) {
        return -1;
    }
    return 0;
}

/**
 * @brief Send 404 Not Found.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @return 0 if execution was successful, -1 if error occured while sending.
 */
int send_http_not_found(int client_fd) {
    char date[50];
    char response[300];

    

    http_current_time(date);

    snprintf(response, sizeof(char) * 200, "HTTP/1.1 404 Not Found\r\n"
                                            "Date: %s\r\n"
                                            "Content-Type: text/plain\r\n"
                                            "Content-Length: 13\r\n"
                                            "\r\n"
                                            "404 Not Found", date);

    if(send_routine(client_fd, response) != 0) {
        return -1;
    }
    return 0;
}


/**
 * @brief Send 200 OK and set Cookie
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @param session_id pointer to null-terminated string with session id stored.
 * 
 * @param session_id_lifespan int describing how long a token is viable in minutes.
 * 
 * @return 0 if execution was successful, -1 if error occured while sending.
 */
int send_http_ok_set_cookie(int client_fd, char* session_id, int session_id_lifespan) {
    int session_id_len = strlen(session_id);
    
    char date[50];
    char response[300 + session_id_len];

    http_current_time(date);

    snprintf(response, sizeof(char) * (300 + session_id_len), "HTTP/1.1 200 OK\r\n"
                                                                "Content-Type: application/json;charset=UTF-8\r\n"
                                                                "Date: %s\r\n"
                                                                "Set-Cookie: sessionId=%s; Path=/; HttpOnly; SameSite=Lax; Max-Age=%d"
                                                                , date, session_id, session_id_lifespan * 60);

    if(send_routine(client_fd, response) != 0) {
        return -1;
    }

    return 0;
}


/**
 * @brief Send 414 Uri Too Long
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @param uri_max_len int describing max length of URI that the server will accept.
 * 
 * @return 0 if execution was successful, -1 if error occured while sending.
 */
int send_http_URI_too_long(int client_fd, int uri_max_len) {
    char date[50];

    char body[100];
    char response[300];
    snprintf(body, sizeof(char) * 100, "URI exeeds the maximum allowed length of %d bytes.\n", uri_max_len);

    size_t len = strlen(body);

    http_current_time(date);

    snprintf(response, sizeof(char) * 300, "HTTP/1.1 414 URI Too Long\r\n"
                                            "Date: %s\r\n"
                                            "Content-Type: text/plain\r\n"
                                            "Content-Length: %zu\r\n"
                                            "Connection: close\r\n"
                                            "\r\n"
                                            "%s", date, len, body);

    if(send_routine(client_fd, response) != 0) {
        return -1;
    }

    return 0;
}


/**
 * @brief Send 403 Forbidden
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * 
 * @param error pointer to null terminated string that describes error that caused 403 response.
 * If error is a NULL-pointer the 403 message will have no body.
 * 
 * @return 0 if execution was successful, -1 if error occured while sending.
 */
int send_http_Forbidden(int client_fd, char* error) {
    char date[50];
    size_t error_len = 0;
    if(error != NULL) {
        error_len = strlen(error);
    }
    
    char response[250 + error_len];
    http_current_time(date);
    if(error != NULL) {
        snprintf(response, sizeof(char) * (250 + error_len),
                "HTTP/1.1 403 Forbidden\r\n"
                "Date: %s\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %zu\r\n"
                "Connection: close\r\n"
                "\r\n"
                "%s", date, error_len, error);
    }
    else {
        snprintf(response, sizeof(char) * (250),
                "HTTP/1.1 403 Forbidden\r\n"
                "Date: %s\r\n"
                "Content-Length: 0\r\n"
                "Connection: close\r\n"
                "\r\n", date);
    }
    

    if(send_routine(client_fd, response) != 0) {
        return -1;
    }

    return 0;
}