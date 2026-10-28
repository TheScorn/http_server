#include "http_server_header.h"
#include <regex.h>
#include <string.h>

/**
 * @brief Function for handling LOGIN requests.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * This funcion does not automatically close client_fd socket.
 * 
 * @param buffer pointer to null-terminated string with message recieved from user
 * 
 * @param session_id_length int that determines lenght of session tokens.
 * 
 * @param session_id_lifespan int that describes number of minutes the sessions tokens are viable.
 * 
 * @return 0 if execution successful, -1 for every send error,
 * -2 if no login info found in message, -3 if database could not be opened,
 * -4 if error occured in SELECT Query, -5 if execution failed,
 * -6 if database could not be opened during token save,
 * -7 if INSERT query failed, 
 * 1 if authorization was not passed.
 */
int http_login_routine(int client_fd, char* buffer, int session_id_length, int session_id_lifespan) {
    regex_t regex;
    regmatch_t matches[4];

    char login[32];
    char password[32];

    regcomp(&regex, "form-data; name=\"login\"\r?\n\r?\n([^\r\n]+)(.|\n)*name=\"password\"\r?\n\r?\n([^\r\n]+)", REG_EXTENDED | REG_NEWLINE);

    if(regexec(&regex, buffer, 4, matches, 0) != 0) {
        fprintf(stderr, "No login info found in message.\n");
        if(send_http_bad_request(client_fd) != 0) {
            return -1;
        }
        
        return -2;
    }

    size_t len = matches[1].rm_eo - matches[1].rm_so;
    strncpy(login, buffer + matches[1].rm_so, len);
    login[len] = '\0';

    len = matches[3].rm_eo - matches[3].rm_so;
    strncpy(password, buffer + matches[3].rm_so, len);
    password[len] = '\0';

    
    int user_id;

    int authentication_status = authenticate(login, password, &user_id);
    if(authentication_status == -1) {
        fprintf(stderr, "Database could not be opened, sending 500.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -3;
    }
    else if(authentication_status == -2) {
        fprintf(stderr, "Error occured in SELECT Query.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -4;
    }
    else if(authentication_status == -5) {
        fprintf(stderr, "Execution failed.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -5;
    }
    else if(authentication_status == -3 || authentication_status == -4) {
        if(send_http_unauthorized(client_fd) != 0) {
            return -1;
        }
        return 1;
    }

    //auth passed

    //token generation

    char session_id[session_id_length + 1];
    generate_session_id(session_id_length, session_id);


    time_t expiry = time(NULL) + (60 * session_id_lifespan);

    //token saving
    int save_token_status = save_session_id(session_id, user_id, expiry);
    if(save_token_status == -1) {
        fprintf(stderr, "Database could not be opened.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -6;
    }
    else if(save_token_status == -2) {
        fprintf(stderr, "INSERT query execution unsuccessful.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -7;
    }

    if(send_http_ok_set_cookie(client_fd, session_id, session_id_lifespan) != 0) {
        return -1;
    }

    return 0;
}

