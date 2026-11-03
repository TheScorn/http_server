#include "http_server_header.h"
#include <regex.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include "NAS_API.h"
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

/**
 * @brief Function for handling all NAS requests.
 * 
 * @param client_fd socket descriptor - connection should be established before using this function.
 * This funcion does not automatically close client_fd socket.
 * 
 * @param buffer pointer to null-terminated string with message recieved from user.
 * After execution, buffer is automatically freed so that if NAS communication takes a long time,
 * the memory is not unneccessaryly occupied.
 * 
 * @param NAS_matches pointer to regmatch_t struct where results of regexec on buffer are stored.
 * 
 * 
 * 
 * @return 0 if execution successful, negative ints on fatal, server errors:
 * -1 if send error occured,
 * -2 if database could not be opened, -3 if SELECT query failed,
 * -4 if bind failed, -5 if send error occured,
 * -6 if NAS response was in some way unexpected,
 * -7
 * positive ints on user or external NAS server errors:
 * 1 if authorization unsuccessful, 2 if no NAS login info found,
 * 3 if uri for NAS was too long, 4 if unable to connect to NAS,
 * 5 if NAS sent an error
 */
int http_NAS_routine(int client_fd, char* buffer, regmatch_t* NAS_matches, int session_id_length, struct sockaddr_in* NAS_add) {
    
    //stary sposób ale kopiowanie jest drogie
    char session_id[session_id_length + 1];
    snprintf(session_id, sizeof(char) * (session_id_length + 1), "%s", buffer + (NAS_matches + 3)->rm_so);

    //nie można psuć bufora bo potem możliwe że trzeba szukać file-size


    struct client_info_struct client_info;
    client_info.logged_in = false;
    client_info.elevated = 0;
    strncpy(client_info.name, "null", 30);
    strncpy(client_info.password, "default", 30);


    char status = authorize_NAS(session_id, &client_info);

    if(status == -1) {
        fprintf(stderr, "Database could not be opened, sending 500.\n");
        if(send_http_500_Internal_Server_Error(client_fd, NULL) != 0) {
            return -1;
        }
        return -2;
    }
    else if(status == -2) {
        fprintf(stderr, "SELECT query failed, sending 500.\n");
        if(send_http_500_Internal_Server_Error(client_fd, NULL) != 0) {
            return -1;
        }
        return -3;
    }
    else if(status == -3) {
        fprintf(stderr, "Bind operation failed, sending 500.\n");
        if(send_http_500_Internal_Server_Error(client_fd, NULL) != 0) {
            return -1;
        }
        return -4;
    }
    else if(status == -4 || status == -5) {
        #ifdef DEBUG
        printf("DEBUG mode: entered first found login.\n");
        #endif
        if(send_http_302_Found_login(client_fd, true) != 0) {
            return -1;
        }
        return 1; //tutaj pierwszy user error
    }
    else if(status == 1) {
        //nie ma danych logowania do NAS
        //chyba wysyłamy Forbidden

        char error[] = "NAS login info missing!";

        if(send_http_403_Forbidden(client_fd, error) != 0) {
            return -1;
        }
        return 2;
    }

    client_info.logged_in = true;

    size_t path_len = (NAS_matches + 2)->rm_eo - (NAS_matches + 2)->rm_so;

    //max limit for path 4096
    //todo, add new param for nas path max len
    if(path_len > URI_MAX_LEN_LIMIT) {
        if(send_http_414_Request_URI_Too_Long(client_fd, URI_MAX_LEN_LIMIT) != 0) {
            return -1;
        }
        return 3;
    }

    char path[path_len + 1];
    snprintf(path, sizeof(char) * (1 + path_len + 1), "/%s", buffer + (NAS_matches + 2)->rm_so);


    int sockD = socket(AF_INET, SOCK_STREAM, 0);

    status = connect(sockD, (struct sockaddr*)NAS_add, sizeof(struct sockaddr_in));
    if(status == -1) {
        if(send_http_503_Service_Unavailable(client_fd, "Could not connect to NAS.")) {
            return -1;
        }
        return 4;
    }


    //this pointer is not NULL only in PUT
    //char* file_size_buffer = NULL; //może można ich nie tworzyć póki wiemy że to nie PUT
    //char* mtime_buffer = NULL;

    size_t req_len = (NAS_matches + 1)->rm_eo - (NAS_matches + 1)->rm_so;
    char request[req_len + 1];
    snprintf(request, sizeof(char) * (req_len + 1), "%s", buffer + (NAS_matches + 1)->rm_so);

    //POST /NAS/PUT/

    //próba bez kopiowania wszystkiego żeby nie spowalniać procesu
    //path kopiujemy bo jest na nim sporo operacji
    //komendę wskazujemy
    
    //do PUT przekazujemy też bufor żeby można było szukać file size tylko wtedy

    if(strcasecmp(request, "LIST") == 0) {
        free(buffer);
        //probably a list, but might be an error
        char* response;
        status = LIST_routine(sockD, path, client_info.NAS_username, client_info.NAS_password, &response);
        if(status == -1) {
            //problem inside send
            if(send_http_500_Internal_Server_Error(client_fd, NULL) != 0) {
                close(sockD);
                return -1;
            }
            close(sockD);
            return -5;
        }
        else if(status == -2 || status == -3 || status == -4 || status == -5 || status == -6) {
            if(send_http_502_Bad_Gateway(client_fd) != 0) {
                close(sockD);
                return -1;
            }
            close(sockD);
            return -6;
        }
        else if(status == 1) {
            //sending 400 Bad Request with NAS ERROR
            if(send_http_400_Bad_Request(client_fd, response) != 0) {
                close(sockD);
                free(response);
                return -1;
            }
            close(sockD);
            free(response);
            return 5;
        }


        //final sockD close
        close(sockD);

        char* list_json;
        LIST_to_json(response, &list_json);
        free(response);

        size_t content_len = strlen(list_json);

        char date[50];
        char* message = (char*)malloc(sizeof(char) * (198 + content_len));
        http_current_time(date);
        snprintf(message, sizeof(char) * (198 + content_len), 
                "HTTP/1.1 200 OK\r\n"
                "Date: %s\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %ld\r\n"
                "Connection: close\r\n"
                "\r\n"
                "%s", date, content_len, list_json);

        free(list_json);
        content_len = strlen(message);

        if(send_routine(client_fd, message) != 0) {
            free(message);
            return -1;
        }
        free(message);

        return 0;
    }



    else if(strcasecmp(request, "MKDIR") == 0) {
        free(buffer);

        char* response;
        status = MKDIR_routine(sockD, path, client_info.NAS_username, client_info.NAS_password, &response);
        if(status == -1) {
            if(send_http_500_Internal_Server_Error(client_fd, NULL) != 0) {
                close(sockD);
                return -1;
            }
            close(sockD);
            return -5;
        }
        else if(status == -2 || status == -3 || status == -4 || status == -5 || status == -6 || status == -7) {
            if(send_http_502_Bad_Gateway(client_fd) != 0) {
                close(sockD);
                return -1;
            }
            close(sockD);
            return -6;
        }
        else if(status == 1) {
            //sending 400 Bad Request with NAS ERROR
            if(send_http_400_Bad_Request(client_fd, response) != 0) {
                close(sockD);
                free(response);
                return -1;
            }
            close(sockD);
            free(response);
            return 5;
        }

        close(sockD);

        if(send_http_201_Created(client_fd, "{\"success\":true}") != 0) {
            return -1;
        }

        return 0;
    }

    else if(strcasecmp(request, "DEL") == 0) {
        free(buffer);


        char* response;
        status = DEL_routine(sockD, path, client_info.NAS_username, client_info.NAS_password, &response);
        if(status == -1) {
            if(send_http_500_Internal_Server_Error(client_fd, NULL) != 0) {
                close(sockD);
                return -1;
            }
            close(sockD);
            return -5;
        }
        else if(status == -2 || status == -3 || status == -4 || status == -5 || status == -6 || status == -7) {
            if(send_http_502_Bad_Gateway(client_fd) != 0) {
                close(sockD);
                return -1;
            }
            close(sockD);
            return -6;
        }
        else if(status == 1) {
            //sending 400 Bad Request with NAS ERROR
            if(send_http_400_Bad_Request(client_fd, response) != 0) {
                close(sockD);
                free(response);
                return -1;
            }
            close(sockD);
            free(response);
            return 5;
        }

        close(sockD);

        if(send_http_200_OK(client_fd, "{\"success\":true}") != 0) {
            return -1;
        }

        return 0;
    }

    else if(strcasecmp(request, "GET") == 0) {
        free(buffer);
        
        char* response;
        unsigned long long mtime;
        unsigned long long filesize;


        //int idx = last_occurence(path, '/') + 1;
        //char filename[strlen(path) - idx + 1];
        //snprintf(filename, sizeof(char) * (strlen(path) - idx + 1), "%s", path + idx);

        //chyba można szybciej
        char* filename = path + last_occurence(path, '/') + 1;

        const char* extension = get_file_extension(filename);
        const char* mime_type = get_mime_type(extension);

        status = pre_GET_routine_acc(sockD, path, client_info.NAS_username, client_info.NAS_password, &response, &mtime, &filesize);
        if(status == -1) {
            if(send_http_500_Internal_Server_Error(client_fd, NULL) != 0) {
                close(sockD);
                return -1;
            }
            close(sockD);
            return -5;
        }
        else if(status < 0) {
            if(send_http_502_Bad_Gateway(client_fd) != 0) {
                close(sockD);
                return -1;
            }
            close(sockD);
            return -6;
        }
        else if(status == 1) {
            if(send_http_400_Bad_Request(client_fd, response) != 0) {
                close(sockD);
                free(response);
                return -1;
            }
            close(sockD);
            free(response);
            return 5;
        }


        char date[50];
        char response2[300];
        http_current_time(date);

        snprintf(response2, sizeof(char) * 300,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Date: %s\r\n"
                "Content-Length: %lld\r\n"
                "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n"
                , mime_type, date, filesize, filename);

        if(send_routine(client_fd, response2) != 0) {
            close(sockD);
            return -1;
        }

        unsigned char* hold_buffer = (unsigned char*)malloc(DEFAULT_FILE_BLOCK_SIZE);

        size_t to_recieve;
        size_t total;
        unsigned long long remaining;
        size_t recieved = 0;

        while(recieved < filesize) {
            remaining = filesize - recieved;
            to_recieve = remaining < DEFAULT_FILE_BLOCK_SIZE ? remaining : DEFAULT_FILE_BLOCK_SIZE;

            ssize_t n = recv(sockD, hold_buffer, to_recieve, 0);

            if(n <= 0) {
                fprintf(stderr, "0 or less bytes recieved while transmitting file in GET, sending 500.\n");
                send_http_500_Internal_Server_Error(client_fd, NULL);
                close(sockD);
                free(hold_buffer);
                return -1;
            }

            total = 0;
            while(total < n) {
                ssize_t n2 = send(client_fd, hold_buffer + total, n - total, 0);
                if(n2 <= 0) {
                    fprintf(stderr, "0 or less bytes sent while transmitting file in GET.\n");
                    close(sockD);
                    free(hold_buffer);
                    return -1;
                }

                total += n2;
            }


            recieved += n;
        }

        free(hold_buffer);
        close(sockD);
        return 0;
    }

    else if(strcasecmp(request, "PUT") == 0) {
        //tutaj nie możemy zwolnić bufora bo jest w nim plik

        //tu dopiero odpalamy regexa i szukamy czy jest filesize albo mtime
        regex_t regex;
        regmatch_t meta_matches[3];
        regcomp(&regex, "File-Size:[[:space:]]*([0-9]+)\r?\nmTime:[[:space:]]([0-9]+)", REG_EXTENDED);

        if(regexec(&regex, buffer, 3, meta_matches, 0) != 0) {
            close(sockD);
            free(buffer);
            //jeśli nie ma metadanych
            if(send_http_400_Bad_Request(client_fd, "ERROR, no metadata for file found.") != 0) {
                fprintf(stderr, "Error occured during send 400 bad request in PUT.\n");
                return -1;
            }
            return 6;
        }

        size_t file_size_len = meta_matches[1].rm_eo - meta_matches[1].rm_so;
        char file_size_buffer[file_size_len + 1];
        snprintf(file_size_buffer, sizeof(char) * (file_size_len + 1), "%s", buffer + meta_matches[1].rm_so);

        size_t mtime_len = meta_matches[2].rm_eo - meta_matches[2].rm_so;
        char mtime_buffer[file_size_len + 1];
        snprintf(mtime_buffer, sizeof(char) * (mtime_len + 1), "%s", buffer + meta_matches[2].rm_so);


        int idx = last_occurence(path, '/');
        char filename[strlen(path) - idx + 1];
        char* dir_path = path;
        snprintf(filename, sizeof(char) * (strlen(path) - idx + 1), "%s", path + idx + 1);
        dir_path[idx + 1] = '\0';


        regfree(&regex);

        
        unsigned long long file_size;
        unsigned long long mtime;

        errno = 0;
        char* end;

        file_size = strtoull(file_size_buffer, &end, 10);
        if((file_size == 0 && end == file_size_buffer) || file_size == ULLONG_MAX && errno || *end) {
            free(buffer);
            close(sockD);
            if(send_http_400_Bad_Request(client_fd, "ERROR, File-Size was not an int or was too large.") != 0) {
                return 0;
            }
            return 7;
        }

        mtime = strtoull(mtime_buffer, &end, 10);
        if((mtime == 0 && end == mtime_buffer) || (mtime == ULLONG_MAX && errno) || (*end)) {
            free(buffer);
            close(sockD);
            if(send_http_400_Bad_Request(client_fd, "ERROR, Mtime was not an int or was too large.") != 0) {
                return 0;
            }
            return 7;
        }


        char* response;
        status = pre_PUT_routine(sockD, dir_path, filename, client_info.NAS_username, client_info.NAS_password, &file_size, &mtime, &response);

        if(status == -1 || status == -8 || status == -11) {
            free(buffer);
            close(sockD);
            if(send_http_500_Internal_Server_Error(client_fd, NULL) != 0) {
                return -1;
            }
            return -5;
        }
        else if(status == -2 || status == -3 || status == -4 || status == -5 || status == -6 || status == -7 || status == -10) {
            close(sockD);
            free(buffer);
            if(send_http_502_Bad_Gateway(client_fd) != 0) {
                
                return -1;
            }
            return -6;
        }
        else if(status == -9) {
            close(sockD);
            free(buffer);
            if(send_http_400_Bad_Request(client_fd, "NAS refused file.") != 0) {
                return -1;
            }
            return 8;
        }
        else if(status == 1) {
            free(buffer);
            close(sockD);
            if(send_http_400_Bad_Request(client_fd, response) != 0) {
                free(response);
                return -1;
            }
            free(response);
            return 9;
        }
        

        char* file_contents = strstr(buffer, "\r\n\r\n") + 4;
        *(file_contents - 1) = '\0';

        size_t header_len = strlen(buffer);

        size_t recvd_file_content_len = file_size < (DEFAULT_BUFFER_SIZE - header_len - 1) ? file_size : (DEFAULT_BUFFER_SIZE - header_len - 1);
        size_t total_streamed = 0;
        unsigned long long remaining;
        size_t to_recieve;
        size_t total;

        while(total_streamed < recvd_file_content_len) {
            ssize_t n = send(sockD, file_contents + total_streamed, recvd_file_content_len - total_streamed, 0);
            if(n <= 0) {
                fprintf(stderr, "Error occured while sending first batch.\n");
                close(sockD);
                free(buffer);
                send_http_500_Internal_Server_Error(client_fd, NULL);
                return -1;
            }
            total_streamed += n;
        }

        free(buffer);

        bool freeb = false;
        if(total_streamed < file_size) {
            freeb = true;
            buffer = (char*)malloc(DEFAULT_FILE_BLOCK_SIZE);
        }

        while(total_streamed < file_size) {
            remaining = file_size - total_streamed;
            to_recieve = remaining < DEFAULT_FILE_BLOCK_SIZE ? remaining : DEFAULT_FILE_BLOCK_SIZE;

            ssize_t n = recv(client_fd, buffer, to_recieve, 0);
            if(n <= 0) {
                free(buffer);
                close(sockD);
                close(client_fd);
                return -1;
            }

            total = 0;
            while(total < n) {
                ssize_t n2 = send(sockD, buffer + total, n - total, 0);
                if(n2 <= 0) {
                    send_http_500_Internal_Server_Error(client_fd, NULL);
                    free(buffer);
                    close(client_fd);
                    close(sockD);
                    return -1;
                }
                total += n2;

            }

            total_streamed += n;
        }


        close(sockD);
        if(freeb) {
            free(buffer);
        }
        
        if(send_http_200_OK(client_fd, "{\"success\":true}") != 0) {
            return -1;
        }
        return 0;
    }

    else {
        fprintf(stderr, "Unknown NAS command type: %s entered despite passing regex.\n", request);
        free(buffer);
        close(sockD);
        return -15;
    }
}

