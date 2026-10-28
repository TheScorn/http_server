#include "http_server_header.h"
#include <regex.h>
#include <string.h>
#include <unistd.h>


/**
 * @brief Function for handling LOGOUT requests.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * This function does not automatially close client_fd socket.
 * 
 * @param buffer pointer to null-terminated string with message recieved from user
 * 
 * @return 0 if execution successful, -1 for every send error,
 * -2 if database could not be opened, -3 if DROP Query failed.
 */
int http_logout_routine(int client_fd, char* buffer) {
    regex_t regex;
    regmatch_t matches[2];


    regcomp(&regex, "\r\nCookie: sessionId=([^ ]+)([\r\n|\r|\n]|$)", REG_EXTENDED);
    if(regexec(&regex, buffer, 2, matches, 0) == 0) {
        buffer[matches[1].rm_eo] = '\0';
        char* session_id = buffer + matches[1].rm_so;
        session_id[strcspn(session_id, "\r\n")] = '\0';

        int drop_session_status = drop_session(session_id);
        if(drop_session_status == -1) {
            fprintf(stderr, "Database could not be opened, sending 500.\n");
            if(send_http_internal_server_error(client_fd, NULL) != 0) {
                return -1;
            }
            return -2;
        }
        else if(drop_session_status == -2) {
            fprintf(stderr, "DROP Query execution unsuccessful, sending 500.\n");
            if(send_http_internal_server_error(client_fd, NULL) != 0) {
                return -1;
            }
            return -3;
        }
        else if(drop_session_status == -4) {
            fprintf(stderr, "Warning! Multiple session id's rmoved on one logout message.\n");
        }
    }

    if(send_http_found_main(client_fd, true) != 0) {
        return -1;
    }

    return 0;
}

