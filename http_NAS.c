#include "http_server_header.h"
#include <regex.h>
#include <string.h>


/**
 * @brief Function for handling all NAS requests.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * This funcion does not automatically close client_fd socket.
 * 
 * @param buffer pointer to null-terminated string with message recieved from user.
 * 
 * @param NAS_matches pointer to regmatch_t struct where results of regexec on buffer are stored.
 * 
 * 
 * 
 * @return 0 if execution successful, negative ints on fatal, server errors:
 * -1 if send error occured,
 * -2 if database could not be opened, -3 if SELECT query failed,
 * -4 if bind failed,
 * positive ints on user errors:
 * 1 if authorization unsuccessful, 2 if no NAS login info found.
 */
int http_NAS_routine(int client_fd, char* buffer, regmatch_t* NAS_matches, int session_id_length) {
    
    char session_id[session_id_length + 1];
    snprintf(session_id, sizeof(char) * (session_id_length + 1), "%s", buffer + (NAS_matches + 1)->rm_so);


    struct client_info_struct client_info;
    client_info.logged_in = false;
    client_info.elevated = 0;
    strncpy(client_info.name, "null", 30);
    strncpy(client_info.password, "default", 30);


    char status = authorize_NAS(session_id, &client_info);

    if(status == -1) {
        fprintf(stderr, "Database could not be opened, sending 500.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -2;
    }
    else if(status == -2) {
        fprintf(stderr, "SELECT query failed, sending 500.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -3;
    }
    else if(status == -3) {
        fprintf(stderr, "Bind operation failed, sending 500.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -4;
    }
    else if(status == -4 || status == -5) {
        if(send_http_found_login(client_fd, true) != 0) {
            return -1;
        }
        return 1; //tutaj pierwszy user error
    }
    else if(status == 1) {
        //nie ma danych logowania do NAS
        //chyba wysyłamy Forbidden

        char error[] = "NAS login info missing!";

        if(send_http_Forbidden(client_fd, error) != 0) {
            return -1;
        }
        return 2;
    }

    

}

