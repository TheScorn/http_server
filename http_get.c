#include "http_server_header.h"
#include "PPHP.h"
#include <string.h>
#include <regex.h>
#include <stdlib.h>

/**
 * @brief Function for handling GET requests
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * This funcion does not automatically close client_fd socket.
 * 
 * @param buffer pointer to null-terminated string with message recieved from user.
 * 
 * 
 * @param matches pointer to regmtach_t struct with indexes telling where in buffer URI starts and stops.
 * 
 * @param uri_max_len int that limits uri length.
 * 
 * @return 0 if execution successful. Any other value is considered an error.
 * Every positive coded error is assumed to be originating from the user,
 * negative coded errors are internal and should be treated as fatal.
 * List of fatal errors: -1 if send failed inside any function,
 * -2 if Database could not be opened, -3 bind or prepare failed,
 * -4 if file could not be opened despite beeing resolved earlier in Database,
 * 
 * List of user-errors: 1 if URI sent was too long,
 * 2 if page was not found, 3 if invalid token presented,
 * 4 if user had insufficient rights to access requested resource
 * 
 */
int http_get_routine(int client_fd, char* buffer, regmatch_t* matches, int uri_max_len) {
    struct client_info_struct client_info;
    client_info.logged_in = false;
    client_info.elevated = 0;
    strncpy(client_info.name, "null", 30);
    strncpy(client_info.password, "default", 30);

    size_t page_name_len = (matches + 1)->rm_eo - (matches + 1)->rm_so;

    //catch potential error if user inputs very long page name
    //assuming that no page should have names beyond 4096 characters
    if(page_name_len > uri_max_len) {
        if(send_http_URI_too_long(client_fd, uri_max_len) != 0) {
            return -1;
        }
        return 1;
    }


    char page_name[page_name_len + 1];
    snprintf(page_name, sizeof(char) * (page_name_len + 1), "%s", buffer + (matches + 1)->rm_so);

    char* file_name = url_decode(page_name);

    char file_ext[32];

    strcpy(file_ext, get_file_extension(file_name));


    struct file_info_struct file_info;
    char status = get_file_info(file_name, &file_info);
    free(file_name);

    if(status == -1) {
        fprintf(stderr, "Files table in database could not be opened, sending 500.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -2;
    }
    else if(status == -2) {
        fprintf(stderr, "Prepare failed, sending 500.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -3;
    }
    else if(status == -3) {
        fprintf(stderr, "Bind failed, sending 500.\n");
        if(send_http_internal_server_error(client_fd, NULL) != 0) {
            return -1;
        }
        return -3;
    }
    else if(status == -4) {
        if(send_http_not_found(client_fd) != 0) {
            return -1;
        }
        return 2;
    }

    regex_t regex;

    regcomp(&regex, "\r\nCookie: sessionId=([^ ]+)([\r\n|\r|\n]|$)", REG_EXTENDED);
    if(regexec(&regex, buffer, 2, matches, 0) != 0) {
        //There is no Cookie in the request

        #ifdef DEBUG
        printf("DEBUG mode: function http_get_routine: no Cookie entered.\n");
        #endif


        //Check If page requires login
        if(file_info.zone_type != 0) {
            //No Cookie and Login required
            if(send_http_found_login(client_fd, false) != 0) {
                return -1;
            }
            return 0;
        }
        else {
            //No Cookie but no login required

            char* file;

            status = load_file(file_info.file_path, &file);
            if(status == -1) {
                fprintf(stderr, "File stated in Database but could not be opened, sending 500.\n");
                if(send_http_internal_server_error(client_fd, NULL) != 0) {
                    return -1;
                }
                return -4;
            }
            
            char keys[MAX_NUMBER_OF_PAGE_VAR][20] = {"version_major", "version_minor", "logged_in", "username", "elevated"};
            char vals[MAX_NUMBER_OF_PAGE_VAR][30];
            char version_major[4];
            char version_minor[4];
            char logged_in[2];
            char elevated[2];
            snprintf(version_major, sizeof(char) * 4, "%d", HTTP_SERVER_VERSION_MAJOR);
            snprintf(version_minor, sizeof(char) * 4, "%d", HTTP_SERVER_VERISON_MINOR);
            snprintf(logged_in, sizeof(char) * 2, "%d", (int)client_info.logged_in);
            memcpy(vals[0], version_major, strlen(version_major) + 1);
            memcpy(vals[1], version_minor, strlen(version_minor) + 1);
            memcpy(vals[2], logged_in, 2);
            memcpy(vals[3], "null", 5);
            memcpy(vals[4], "0", 2);

            size_t file_len = strlen(file);
            char* processed_file = (char*)calloc(file_len + 1, 1);
            status = PPHP_key_val_insert2(processed_file, file_len + 1, file, keys, vals, MAX_NUMBER_OF_PAGE_VAR);

            free(file);

            file_len = strlen(processed_file);

            char* response;
            add_response_header(processed_file, file_ext, &response);
            free(processed_file);

            if(send_routine(client_fd, response) != 0) {
                free(response);
                return -1;
            }

            free(response);

            return 0;

        }


    }
    else {
        //Cookie found

        #ifdef DEBUG
        printf("DEBUG mode: function http_get_routine: cookie found.\n");
        #endif

        buffer[matches[1].rm_eo] = '\0';
        char* session_id = buffer + matches[1].rm_so;

        //czy ta linia jest potrzebna??
        session_id[strcspn(session_id, "\r\n")] = '\0';

        status = authorize(session_id, &client_info);
        if(status == -1) {
            fprintf(stderr, "Users table in database could not be opened, sending 500.\n");
            if(send_http_internal_server_error(client_fd, NULL) != 0) {
                return -1;
            }
            return -2;
        }
        else if(status == -2) {
            fprintf(stderr, "SELECT query on Sessions unsuccessful, sending 500.\n");
            if(send_http_internal_server_error(client_fd, NULL) != 0) {
                return -1;
            }
            return -3;
        }
        else if(status == -3) {
            fprintf(stderr, "Bind failed, sending 500.\n");
            if(send_http_internal_server_error(client_fd, NULL) != 0) {
                return -1;
            }
            return -3;
        }
        else if((status == -4 || status == -5) && file_info.zone_type != 0) {
            //if token exists but its incorrect and the zone requires login
            //remove token -> tell user to login again

            if(send_http_found_login(client_fd, true) != 0) {
                return -1;
            }
            return 3;
        }
        else if((status == -3 || status == -4) && file_info.zone_type == 0) {
            char* file;

            status = load_file(file_info.file_path, &file);
            if(status == -1) {
                fprintf(stderr, "File stated in Database but could not be opened, sending 500.\n");
                if(send_http_internal_server_error(client_fd, NULL) != 0) {
                    return -1;
                }
                return -4;
            }
            
            char keys[MAX_NUMBER_OF_PAGE_VAR][20] = {"version_major", "version_minor", "logged_in", "username", "elevated"};
            char vals[MAX_NUMBER_OF_PAGE_VAR][30];
            char version_major[4];
            char version_minor[4];
            char logged_in[2];
            snprintf(version_major, sizeof(char) * 4, "%d", HTTP_SERVER_VERSION_MAJOR);
            snprintf(version_minor, sizeof(char) * 4, "%d", HTTP_SERVER_VERISON_MINOR);
            snprintf(logged_in, sizeof(char) * 2, "%d", (int)client_info.logged_in);
            memcpy(vals[0], version_major, strlen(version_major) + 1);
            memcpy(vals[1], version_minor, strlen(version_minor) + 1);
            memcpy(vals[2], logged_in, 2);
            memcpy(vals[3], "null", 5);
            memcpy(vals[4], "0", 2);

            size_t file_len = strlen(file);
            char* processed_file = (char*)calloc(file_len + 1, 1);
            status = PPHP_key_val_insert2(processed_file, file_len + 1, file, keys, vals, MAX_NUMBER_OF_PAGE_VAR);

            free(file);

            file_len = strlen(processed_file);

            char* response;
            add_response_header(processed_file, file_ext, &response);
            free(processed_file);

            if(send_routine(client_fd, response) != 0) {
                free(response);
                return -1;
            }

            free(response);

            return 0;
        }

        //after checking all possible outcomes we land here as a logged in user.
        client_info.logged_in = true;


        if((file_info.zone_type == 0 || file_info.zone_type == 2) || (file_info.zone_type == 1 && client_info.elevated)) {
            //(jeśli zona wpuszcza każdego lub dowolnego zalogowanego) lub (zona wpuszcza tylko elevated i elevated)
            
            #ifdef DEBUG
            printf("DEBUG mode: function http_get_routine: elevated: %d, file accessed.\n", client_info.elevated);
            #endif
            
            char* file;

            status = load_file(file_info.file_path, &file);
            if(status == -1) {
                fprintf(stderr, "File stated in Database but could not be opened, sending 500.\n");
                if(send_http_internal_server_error(client_fd, NULL) != 0) {
                    return -1;
                }
                return -4;
            }
            
            char keys[MAX_NUMBER_OF_PAGE_VAR][20] = {"version_major", "version_minor", "logged_in", "username", "elevated"};
            char vals[MAX_NUMBER_OF_PAGE_VAR][30];
            char version_major[4];
            char version_minor[4];
            char logged_in[2];
            char username[33];
            char elevated_str[2];
            snprintf(username, 33, "\"%s\"", client_info.name);
            snprintf(version_major, sizeof(char) * 4, "%d", HTTP_SERVER_VERSION_MAJOR);
            snprintf(version_minor, sizeof(char) * 4, "%d", HTTP_SERVER_VERISON_MINOR);
            snprintf(logged_in, sizeof(char) * 2, "%d", (int)client_info.logged_in);
            snprintf(elevated_str, sizeof(char) * 2, "%d", (int)client_info.elevated);
            memcpy(vals[0], version_major, strlen(version_major) + 1);
            memcpy(vals[1], version_minor, strlen(version_minor) + 1);
            memcpy(vals[2], logged_in, 2);
            memcpy(vals[3], username, strlen(username) + 1);
            memcpy(vals[4], elevated_str, 2);

            size_t file_len = strlen(file);


            char* processed_file = (char*)calloc(file_len + 1, 1);
            status = PPHP_key_val_insert2(processed_file, file_len + 1, file, keys, vals, MAX_NUMBER_OF_PAGE_VAR);

            free(file);

            file_len = strlen(processed_file);


            char* response;
            add_response_header(processed_file, file_ext, &response);
            free(processed_file);

            

            if(send_routine(client_fd, response) != 0) {
                free(response);
                return -1;
            }

            free(response);

            return 0;

        }
        else if(file_info.zone_type == 1 && !client_info.elevated) {
            if(send_http_Forbidden(client_fd, "Insufficient access rights!") != 0) {
                return -1;
            }
            return 4;
        }


    }


}