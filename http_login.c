#include "http_server_header.h"
#include <regex.h>


/**
 * @brief Function for handling LOGIN requests.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * This funcion does not automatically close client_fd socket.
 * 
 * @param buffer pointer to null-terminated string with message recieved from user
 * 
 * @return 0 if execution successful, -1 for every send error,
 * -2 if no login info found in message.
 * 
 */
int http_login_handler(int client_fd, char* buffer) {
    regex_t regex;
    regmatch_t matches[4];

    char login[32];
    char password[32];

    regcomp(&regex, "form-data; name=\"login\"\r?\n\r?\n([^\r\n]+)(.|\n)*name=\"password\"\r?\n\r?\n([^\r\n]+)", REG_EXTENDED | REG-REG_NEWLINE);

    if(regexec(&regex, buffer, 4, matches, 0) != 0) {
        fprintf(stderr, "No login info found in message.\n");
        if(send_http_bad_request(client_fd) != 0) {
            close(client_fd);
            return -1;
        }
        close(clie)
        return -2;
    }

}

