#include "http_server_header.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <regex.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Function handling http clients after connection is established
 * 
 * To do
 * 
 * @param arg void pointer representing the struct of type handle_args_struct
 */
void *handle_client(void *arg) {
    
    chdir(PAGES);
    struct handle_args_struct* args = arg;

    int client_fd = *(args->client_fd);
    bool verbose = args->verbose_init;
    int session_id_length = args->session_id_length;
    int session_id_lifespan = args->session_id_lifespan;
    //trzeba zaimplementować sprawdzanie typu połączenia
    //rozbić tą funkcję na kilka mniejszych
    //0: NONE - nierozpoznane połączenie
    //1: HTTP request
    //2: Login request
    //3: Logout request

    ////////////////////////////////////////////////////////////////
    char connection_type = 0;


    int buffer_size = DEFAULT_BUFFER_SIZE;
    char *buffer = (char *)malloc(buffer_size * sizeof(char));
    //miejsce na przychodzącą wiad


    //odebranie wiad
    ssize_t bytes_received = recv(client_fd, buffer, buffer_size, 0);
    //czy cokolwiek odebrane
    if(bytes_received == 0) {
        free(buffer);
        close(client_fd);
        return NULL;
    }

    //////////////////////sprawdzamy czy to typ 1
    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];
    
    if(regexec(&regex, buffer, 2, matches, 0) == 0) {
        connection_type = 1;
    }

    //////////////////////////sprawdzamy czy typ 2
    regcomp(&regex, "^POST[[:space:]]+/[[:space:]]+HTTP/1\\.[01]\r?\n(.|\n)*Content-Type:[[:space:]]*multipart/form-data;[[:space:]]*boundary=([^\r\n]+)(.|\n)*name=\"login\"\r?\n\r?\n([^\r\n]+)(.|\n)*name=\"password\"\r?\n\r?\n([^\r\n]+)", REG_EXTENDED | REG_NEWLINE);
    if(regexec(&regex, buffer, 2, matches, 0) == 0) {
        connection_type = 2;
    }

    
    //sprawdzanie czy typ 3
    //TODO


    //Obsługa http request
    if(connection_type == 1) {
        struct client_info_struct client_info;
        client_info.logged_in = false;
        strncpy(client_info.name, "default", 30);
        strncpy(client_info.password, "default", 30);
        client_info.access_flags = 0;


        //szukamy jaką stronę trzeba wysłać
        regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);

        //nie sprawdzamy czy jest jakiś match bo to już nam ogarnął regex do typu połączenia
        regexec(&regex, buffer, 2, matches, 0);

        //miejsce na page name
        char *page_name_buffer = (char *)malloc(sizeof(char) * buffer_size);

        strcpy(page_name_buffer, buffer);
        
        page_name_buffer[matches[1].rm_eo] = '\0';

        //char z nazwą pliku
        const char *url_encoded_file_name = page_name_buffer + matches[1].rm_so;
        char *file_name = url_decode(url_encoded_file_name);
        //rozszerzenie pliku
        char file_ext[32];
        strcpy(file_ext, get_file_extension(file_name));
        
        //miejsce na odpowiedź
        char* response = (char *)malloc(buffer_size * 2 * sizeof(char));
        size_t response_len = strlen(response);

        struct file_info_struct file_info; //struktura do przechowywania info o stronie
        int db_file_info_status = get_file_info(file_name, &file_info);
        
        if(db_file_info_status == -1) {
            fprintf(stderr, "Database could not be opened.\n");
            free(response);
            free(page_name_buffer);
            free(file_name);
            close(client_fd);
            return NULL;
        }
        else if(db_file_info_status == -2) {
            fprintf(stderr, "Select query on Files unsuccessful.\n");
            free(response);
            free(page_name_buffer);
            free(file_name);
            close(client_fd);
            return NULL;
        }
        else if(db_file_info_status == -3) {
            
            char date[50];
            http_current_time(date);

            snprintf(response, buffer_size,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Date: %s\r\n"
            "404 Not Found", date);

            response_len = strlen(response);
            
            size_t total = 0;
            while(total < response_len) {
                ssize_t n = send(client_fd, response, response_len, 0);
                if(n <= 0) {
                    fprintf(stderr, "0 bytes sent. Breaking.\n");
                    break;
                }
                total += n;
            }


            
            free(response);
            free(page_name_buffer);
            free(file_name);
            close(client_fd);
            return NULL;
        }

        //jednak chcemy zawsze sprawdzać najpierw czy jest ciasteczko żeby móc dodać info o zalogowanym użytkowniku
        //newet jeśli strona nie wymaga logowania

        regcomp(&regex, "\r\nCookie: sessionId=([^ ]+)([\r\n|\r|\n]|$)", REG_EXTENDED);
        if(regexec(&regex, buffer, 2, matches, 0) != 0) {
            //jeśli nie ma ciasteczka
            //sprawdzamy czy strona wymaga logowania
            if(file_info.zone_type != 0) {
                //jeśli nie ma ciesteczka a jest wymagane
                char date[50];
                http_current_time(date);

                snprintf(response, buffer_size,
                    "HTTP/1.1 302 Found\r\n"
                    "Date: %s\r\n"
                    "Location: /login_page.html"
                    , date);

                response_len = strlen(response);

                size_t total = 0;
                while(total < response_len) {
                    ssize_t n = send(client_fd, response, response_len, 0);
                    if(n <= 0) {
                        fprintf(stderr, "0 bytes sent. Breaking.\n");
                        break;
                    }
                    total += n;
                }


                
                
                //free(file_info.file_path);
                //free(file_info.zone_name);
                free(response);
                free(page_name_buffer);
                free(file_name);
                close(client_fd);
                return NULL;

            }
            else {
                //jeśli logowanie nie jest wymagane


                //no user config
                int build_response_status = build_http_response(file_info.file_path, file_ext, response, &response_len, buffer_size);
                if(build_response_status == -1) {
                    fprintf(stderr, "File stated in data base but could not be opened.\n");
                    //free(file_info.file_path);
                    //free(file_info.zone_name);
                    free(response);
                    free(page_name_buffer);
                    free(file_name);
                    close(client_fd);
                    return NULL;
                }

                size_t total = 0;
                while(total < response_len) {
                    ssize_t n = send(client_fd, response + total, response_len - total, 0);
                    if(n <= 0) {
                        fprintf(stderr, "0 bytes sent. Breaking.\n");
                        break;
                    }
                    total += n;
                }

        
        
        
        
                //free(file_info.file_path);
                //free(file_info.zone_name);
                free(response);
                free(page_name_buffer);
                free(file_name);
                close(client_fd);
                return NULL;


            }


        }
        else {
            //jeśli jest ciasteczko
            //sprawdzamy info o użytkowniku
            buffer[matches[1].rm_eo] = '\0';

            char* session_id = buffer + matches[1].rm_so;
            session_id[strcspn(session_id, "\r\n")] = '\0';

            int authorize_status = authorize(session_id, &client_info);
            if(authorize_status == -1) {

            }
            
            if(db_file_info_status == -1) {
                fprintf(stderr, "Database could not be opened.\n");
                free(response);
                free(page_name_buffer);
                free(file_name);
                //free(file_info.file_path);
                //free(file_info.zone_name);
                close(client_fd);
                return NULL;
            }
            else if(db_file_info_status == -2) {
                fprintf(stderr, "Select query on Files unsuccessful.\n");
                free(response);
                free(page_name_buffer);
                free(file_name);
                //free(file_info.file_path);
                //free(file_info.zone_name);
                close(client_fd);
                return NULL;
            }
            else if(db_file_info_status == -3 || db_file_info_status == -4) {
                //jeśli tokena nie ma w bazie lub jeśli jest expired
                //wysyłamy redirect do logowania oraz czyścimy nieprawidłowe ciastko
                //potem można pomyśleć o rozdzieleniu tego na dwa przypadki
                //póki nie ma mechanizmu usuwania sessionId to nie ma sensu
                char date[50];
                http_current_time(date);

                snprintf(response, buffer_size,
                    "HTTP/1.1 302 Found\r\n"
                    "Date: %s\r\n"
                    "Set-Cookie: sessionId=; Max-Age=0; Path=/; HttpOnly"
                    "Location: /login_page.html"
                    , date);

                response_len = strlen(response);
                
                send(client_fd, response, response_len, 0);

                free(response);
                free(page_name_buffer);
                free(file_name);
                //free(file_info.file_path);
                //free(file_info.zone_name);
                close(client_fd);
                return NULL;

            }


            client_info.logged_in = true;
            

            
            //jeśli strona jest typu 0
            if(file_info.zone_type == 0) {
                //nie musimy sprawdzać praw użytkownika do strony
                int build_response_status = build_http_response(file_info.file_path, file_ext, response, &response_len, buffer_size);
                if(build_response_status == -1) {
                    fprintf(stderr, "File stated in data base but could not be opened.\n");
                    //free(file_info.file_path);
                    //free(file_info.zone_name);
                    //free(client_info.name);
                    free(response);
                    free(page_name_buffer);
                    free(file_name);
                    close(client_fd);
                    return NULL;
                }
                
                //Użytkownik zalogowany więc można użyć PHP z użytkownikiem

                size_t total = 0;
                while(total < response_len) {
                    ssize_t n = send(client_fd, response + total, response_len - total, 0);
                    if(n <= 0) {
                        fprintf(stderr, "0 bytes sent. Breaking.\n");
                        break;
                    }
                    total += n;
                }

                //free(file_info.file_path);
                //free(file_info.zone_name);
                //free(client_info.name);
                free(response);
                free(page_name_buffer);
                free(file_name);
                close(client_fd);
                return NULL;


            }
            else if(file_info.zone_type == 1) {
                if(file_info.zone_id & client_info.access_flags == 0) {
                    //user nie ma praw do strony
                    char date[50];
                    http_current_time(date);

                    snprintf(response, buffer_size,
                    "HTTP/1.1 403 Forbidden\r\n"
                    "Date: %s\r\n"
                    "Content-Type: text/html"
                    , date);

                    response_len = strlen(response);

                    size_t total = 0;
                    while(total < response_len) {
                        ssize_t n = send(client_fd, response, response_len, 0);
                        if(n <= 0) {
                            fprintf(stderr, "0 bytes sent. Breaking.\n");
                            break;
                        }
                        total += n;
                    }

                    //free(file_info.file_path);
                    //free(file_info.zone_name);
                    //free(client_info.name);
                    free(response);
                    free(page_name_buffer);
                    free(file_name);
                    close(client_fd);
                    return NULL;

                }

                //jeśli ma to tak samo jak wcześniej
                int build_response_status = build_http_response(file_info.file_path, file_ext, response, &response_len, buffer_size);
                if(build_response_status == -1) {
                    fprintf(stderr, "File stated in data base but could not be opened.\n");
                    //free(file_info.file_path);
                    //free(file_info.zone_name);
                    //free(client_info.name);
                    free(response);
                    free(page_name_buffer);
                    free(file_name);
                    close(client_fd);
                    return NULL;
                }
                
                //Użytkownik zalogowany więc można użyć PHP z użytkownikiem

                size_t total = 0;
                while(total < response_len) {
                    ssize_t n = send(client_fd, response, response_len, 0);
                    if(n <= 0) {
                        fprintf(stderr, "0 bytes sent. Breaking.\n");
                        break;
                    }
                    total += n;
                }

                //free(file_info.file_path);
                //free(file_info.zone_name);
                //free(client_info.name);
                free(response);
                free(page_name_buffer);
                free(file_name);
                close(client_fd);
                return NULL;



            }
            else {
                //zone_type 2 (póki co nie ma takiej strony to na później)
            }
            


        }

    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //obsługa logowania
    else if(connection_type == 2) {

        regmatch_t matches2[4];
        char login[32];
        char password[32];

        regcomp(&regex, "form-data; name=\"login\"\r?\n\r?\n([^\r\n]+)(.|\n)*name=\"password\"\r?\n\r?\n([^\r\n]+)", REG_EXTENDED | REG_NEWLINE);

        //czy jest sens sprawdzać ponownie poprawność - raczej nie, ale póki co zostaje

        if(regexec(&regex, buffer, 4, matches2, 0) != 0) {
            fprintf(stderr, "No login info found in message type 2.\n");
            free(buffer);
            close(client_fd);
            return NULL;
        }

        int len = matches2[1].rm_eo - matches2[1].rm_so;
        strncpy(login, buffer + matches2[1].rm_so, len);
        login[len] = '\0';

        len =  matches2[3].rm_eo - matches2[3].rm_so;
        strncpy(password, buffer + matches2[3].rm_so, len);
        password[len] = '\0';

        //###
        //obsługa logowania:
        //#
        //check credentials
        int authentication_status = authenticate(login, password);
        if(authentication_status == -1) {
            fprintf(stderr, "Error occured while opening database.\n");
            free(buffer);
            close(client_fd);
            return NULL;

        }
        else if(authentication_status == -2) {
            fprintf(stderr, "Error occured while executing SELECT query.\n");
            free(buffer);
            close(client_fd);
            return NULL;
        }
        else if(authentication_status == -5) {
            fprintf(stderr, "Execution failed.\n");
            free(buffer);
            close(client_fd);
            return NULL;
        }
        else if(authentication_status == -3 || authentication_status == -4) {
            
            char date[50];
            http_current_time(date);

            char response[150];

            snprintf(response, 150, "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\nDate: %s\r\n", date);
            
            size_t response_len = strlen(response);

            size_t total = 0;
            while(total < response_len) {
                ssize_t n = send(client_fd, response, response_len, 0);
                if(n <= 0) {
                    fprintf(stderr, "0 bytes sent. Breaking.\n");
                    break;
                }
                total += n;
            }
            



            free(buffer);
            close(client_fd);
            return NULL;
        }
        
        
        //# Jeśli autentykacja poprawna
        //generate token


        char session_id[session_id_length + 1];
        generate_session_id(session_id_length, session_id);
        
        
        //moment wygaśnięcia tokenu
        time_t expiry = time(NULL) + (60 * session_id_lifespan);
        
        int save_token_status = save_session_id(session_id, login, expiry);
        if(save_token_status == -1) {
            fprintf(stderr, "Database could not be opened.\n");
            close(client_fd);
            free(buffer);
            return NULL;
        }
        else if(save_token_status == -2) {
            fprintf(stderr, "INSERT query execution unsuccessful.\n");
            close(client_fd);
            free(buffer);
            return NULL;
        }

        char response[250];
        char date[50];
        http_current_time(date);
        //tu trzeba zapisać token
        //free(token);
        snprintf(response, 250, "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json;charset=UTF-8\r\n"
            "Date: %s\r\n"
            "Set-Cookie: sessionId=%s; Path=/; HttpOnly; SameSite=Lax; Max-Age=%d"
            , date, session_id, session_id_lifespan * 60);

        size_t response_len = strlen(response);

            
        size_t total = 0;
        while(total < response_len) {
            ssize_t n = send(client_fd, response, response_len, 0);
            if(n <= 0) {
                fprintf(stderr, "0 bytes sent. Breaking.\n");
                break;
            }
            total += n;
        }


        
        

        free(buffer);
        
        close(client_fd);
        return NULL;
    }



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //obsługa wylogowania
    else if(connection_type == 3) {
        free(buffer);
        close(client_fd);
        return NULL;
    }




    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //nieznane requesty
    else {
        if(verbose) {
            fprintf(stderr, "Unknown request, no data sent.\n");
        }
        close(client_fd);
        free(buffer);
        return NULL;
    }
    
    



    
    close(client_fd);
    return NULL;
}


