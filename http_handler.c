#include "http_server_header.h"
#include "PPHP.h"
#include "NAS_API.h"
#include <string.h>
#include <strings.h>
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
  

    bool NAS_connection = args->test_passed;
    bool verbose = args->verbose_init;
    int client_fd = args->client_fd;
    int session_id_length = args->session_id_length;
    int session_id_lifespan = args->session_id_lifespan;
    struct sockaddr_in NAS_add;
    memcpy(&NAS_add, &(args->NAS_add), sizeof(struct sockaddr_in));



    free(args);

    #ifdef DEBUG
    printf("DEBUG mode: args assigned and freed.\n");
    #endif
    //trzeba zaimplementować sprawdzanie typu połączenia
    //rozbić tą funkcję na kilka mniejszych
    //0: NONE - nierozpoznane połączenie
    //1: HTTP request
    //2: Login request
    //3: Logout request

    ////////////////////////////////////////////////////////////////
    
    enum connection_type_en connection_type = UNKNOWN;

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
        connection_type = GET;
    }

    //////////////////////////sprawdzamy czy typ 2
    regcomp(&regex, "^POST[[:space:]]+/[[:space:]]+HTTP/1\\.[01]\r?\n(.|\n)*Content-Type:[[:space:]]*multipart/form-data;[[:space:]]*boundary=([^\r\n]+)(.|\n)*name=\"login\"\r?\n\r?\n([^\r\n]+)(.|\n)*name=\"password\"\r?\n\r?\n([^\r\n]+)", REG_EXTENDED | REG_NEWLINE);
    if(regexec(&regex, buffer, 2, matches, 0) == 0) {
        connection_type = LOGIN;
    }

    
    //sprawdzanie czy typ 3
    //POST bo GET nie powinno zmieniać stanu strony
    regcomp(&regex, "^POST[[:space:]]/logout[[:space:]]HTTP/1.1", REG_EXTENDED);
    if(regexec(&regex, buffer, 2, matches, 0) == 0) {
        connection_type = LOGOUT;
    }

    regcomp(&regex, "^POST[[:space:]]/NAS/(LIST|GET|PUT|DEL|MKDIR)/([^ ]*)[[:space:]]+HTTP/1.1.*\r\nCookie: sessionId=([^ ]+)([\r\n|\r|\n]|$)", REG_EXTENDED);
    regmatch_t NAS_matches[4];
    if(regexec(&regex, buffer, 4, NAS_matches, 0) == 0) {
        connection_type = NAS;
    }


    //Obsługa http request
    if(connection_type == GET) {
        struct client_info_struct client_info;
        client_info.logged_in = false;
        client_info.elevated = 0;
        strncpy(client_info.name, "null", 30);
        strncpy(client_info.password, "default", 30);
 


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



        //struct file_info_struct file_info; //struktura do przechowywania info o stronie
        struct file_info_struct* file_info = (struct file_info_struct*)malloc(sizeof(struct file_info_struct));
        int db_file_info_status = get_file_info(file_name, file_info);
        

        if(db_file_info_status == -1) {
            fprintf(stderr, "Database could not be opened.\n");
            free(buffer);
            free(response);
            free(page_name_buffer);
            free(file_name);
            free(file_info);
            close(client_fd);
            return NULL;
        }
        else if(db_file_info_status == -2) {
            fprintf(stderr, "Select query on Files unsuccessful.\n");
            free(buffer);
            free(response);
            free(page_name_buffer);
            free(file_name);
            free(file_info);
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
            free(buffer);
            free(page_name_buffer);
            free(file_name);
            free(file_info);
            close(client_fd);
            return NULL;
        }

        //jednak chcemy zawsze sprawdzać najpierw czy jest ciasteczko żeby móc dodać info o zalogowanym użytkowniku
        //newet jeśli strona nie wymaga logowania

        regcomp(&regex, "\r\nCookie: sessionId=([^ ]+)([\r\n|\r|\n]|$)", REG_EXTENDED);
        if(regexec(&regex, buffer, 2, matches, 0) != 0) {
            //jeśli nie ma ciasteczka
            //sprawdzamy czy strona wymaga logowania
            if(file_info->zone_type != 0) {
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
                free(buffer);
                free(file_name);
                free(file_info);
                close(client_fd);
                return NULL;

            }
            else {
                //jeśli logowanie nie jest wymagane


                //no user config
                
                int build_response_status = build_http_response(file_info->file_path, file_ext, response, &response_len, buffer_size);
                if(build_response_status == -1) {
                    fprintf(stderr, "File stated in data base but could not be opened.\n");
                    //free(file_info.file_path);
                    //free(file_info.zone_name);
                    free(response);
                    free(page_name_buffer);
                    free(buffer);
                    free(file_name);
                    free(file_info);
                    close(client_fd);
                    return NULL;
                }
                response_len = strlen(response);
                response[response_len + 1] = '\0';
                ///////////////////////////////////////////////////////////////////
                //logika do uzupełniania
                //budujemy słownik ze zmiennych
                char keys[MAX_NUMBER_OF_PAGE_VAR][20] = {"version_major","version_minor","logged_in","username"};
                char vals[MAX_NUMBER_OF_PAGE_VAR][30];
                char version_major[4];
                char version_minor[4];
                char logged_in[2];
                snprintf(version_major, 4, "%d", HTTP_SERVER_VERSION_MAJOR);
                snprintf(version_minor, 4, "%d", HTTP_SERVER_VERISON_MINOR);
                snprintf(logged_in, 2, "%d", (int)client_info.logged_in);
                memcpy(vals[0], version_major, strlen(version_major) + 1);
                memcpy(vals[1], version_minor, strlen(version_minor) + 1);
                memcpy(vals[2], logged_in, 2);
                memcpy(vals[3], "null", 5);

                char* complete_response = (char*)calloc(response_len + 1, 1);
                int PPHP_status = PPHP_key_val_insert2(complete_response, response_len + 1, response, keys, vals, MAX_NUMBER_OF_PAGE_VAR);

                response_len = strlen(complete_response);

                size_t total = 0;
                while(total < response_len) {
                    ssize_t n = send(client_fd, complete_response + total, response_len - total, 0);
                    if(n <= 0) {
                        fprintf(stderr, "0 bytes sent. Breaking.\n");
                        break;
                    }
                    total += n;
                }

        
        
        
        
                //free(file_info.file_path);
                //free(file_info.zone_name);
                free(response);
                free(complete_response);
                free(page_name_buffer);
                free(buffer);
                free(file_name);
                free(file_info);
                close(client_fd);
                return NULL;


            }


        }
        else {
            //jeśli mamy ciasteczko


            buffer[matches[1].rm_eo] = '\0';

            char* session_id = buffer + matches[1].rm_so;
            session_id[strcspn(session_id, "\r\n")] = '\0';

            int authorize_status = authorize(session_id, &client_info);
            if(authorize_status == -1) {
                fprintf(stderr, "Database could not be opened.\n");
                free(response);
                free(page_name_buffer);
                free(buffer);
                free(file_name);
                free(file_info);
                close(client_fd);
                return NULL;
            }
            else if(authorize_status == -2) {
                fprintf(stderr, "Select query on Sessions unsuccessful.\n");
                free(response);
                free(page_name_buffer);
                free(buffer);
                free(file_name);
                free(file_info);
                close(client_fd);
                return NULL;
            }
            else if(authorize_status == -3 || authorize_status == -4) {
                //jeśli tokena nie ma w bazie lub jeśli jest expired
                //wysyłamy redirect do logowania oraz czyścimy nieprawidłowe ciastko
                //potem można pomyśleć o rozdzieleniu tego na dwa przypadki
                //póki nie ma mechanizmu usuwania sessionId to nie ma sensu
                char date[50];
                http_current_time(date);

                snprintf(response, buffer_size,
                    "HTTP/1.1 302 Found\r\n"
                    "Date: %s\r\n"
                    "Set-Cookie: sessionId=; Max-Age=0; Path=/; HttpOnly\r\n"
                    "Location: /login_page.html"
                    , date);

                response_len = strlen(response);
                
                send(client_fd, response, response_len, 0);

                free(response);
                free(page_name_buffer);
                free(buffer);
                free(file_name);
                free(file_info);
                close(client_fd);
                return NULL;

            }

            client_info.logged_in = true;
            

            
            //jeśli strona jest typu 0
            if(file_info->zone_type == 0 || file_info->zone_type == 2) {
                //nie musimy sprawdzać praw użytkownika do strony
                //też w przypadku strony typu 2, jeśli użytkownik jest zalogowany to powinna się wyświetlić.

                int build_response_status = build_http_response(file_info->file_path, file_ext, response, &response_len, buffer_size);
                if(build_response_status == -1) {
                    fprintf(stderr, "File stated in data base but could not be opened.\n");
                    free(file_info);
                    free(response);
                    free(page_name_buffer);
                    free(buffer);
                    free(file_name);
                    close(client_fd);
                    return NULL;
                }
                
                response_len = strlen(response);
                response[response_len + 1] = '\0';
                //Użytkownik zalogowany więc można użyć PHP z użytkownikiem

                char keys[MAX_NUMBER_OF_PAGE_VAR][20] = {"version_major","version_minor","logged_in","username"};
                char vals[MAX_NUMBER_OF_PAGE_VAR][30];
                char version_major[4];
                char version_minor[4];
                char logged_in[2];
                char username[33];
                snprintf(version_major, 4, "%d", HTTP_SERVER_VERSION_MAJOR);
                snprintf(version_minor, 4, "%d", HTTP_SERVER_VERISON_MINOR);
                snprintf(logged_in, 2, "%d", (int)client_info.logged_in);
                snprintf(username, 33, "\"%s\"", client_info.name);
                memcpy(vals[0], version_major, strlen(version_major)+ 1);
                memcpy(vals[1], version_minor, strlen(version_minor)+ 1);
                memcpy(vals[2], logged_in, 2);
                memcpy(vals[3], username, strlen(username) + 1);

                char* complete_response = (char*)malloc(response_len + 1);
                int PPHP_status = PPHP_key_val_insert(complete_response, response_len + 1, response, keys, vals, MAX_NUMBER_OF_PAGE_VAR);

                response_len = strlen(complete_response);

                size_t total = 0;
                while(total < response_len) {
                    ssize_t n = send(client_fd, complete_response + total, response_len - total, 0);
                    if(n <= 0) {
                        fprintf(stderr, "0 bytes sent. Breaking.\n");
                        break;
                    }
                    total += n;
                }

                free(file_info);
                free(complete_response);
                free(response);
                free(page_name_buffer);
                free(buffer);
                free(file_name);
                close(client_fd);
                return NULL;


            }
            else if(file_info->zone_type == 1) { //zone tylko dla elevated
                if(client_info.elevated == 0) {
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

                    free(file_info);
                    free(response);
                    free(page_name_buffer);
                    free(buffer);
                    free(file_name);
                    close(client_fd);
                    return NULL;

                }

                //jeśli ma to tak samo jak wcześniej
                int build_response_status = build_http_response(file_info->file_path, file_ext, response, &response_len, buffer_size);
                if(build_response_status == -1) {
                    fprintf(stderr, "File stated in data base but could not be opened.\n");

                    free(file_info);
                    free(response);
                    free(page_name_buffer);
                    free(buffer);
                    free(file_name);
                    close(client_fd);
                    return NULL;
                }
                
                response_len = strlen(response);
                response[response_len + 1] = '\0';

                //Użytkownik zalogowany więc można użyć PHP z użytkownikiem
                char keys[MAX_NUMBER_OF_PAGE_VAR][20] = {"version_major","version_minor","logged_in","username"};
                char vals[MAX_NUMBER_OF_PAGE_VAR][30];
                char version_major[4];
                char version_minor[4];
                char logged_in[2];
                char username[33];
                snprintf(username, 33, "\"%s\"", client_info.name);
                snprintf(version_major, 4, "%d", HTTP_SERVER_VERSION_MAJOR);
                snprintf(version_minor, 4, "%d", HTTP_SERVER_VERISON_MINOR);
                snprintf(logged_in, 2, "%d", (int)client_info.logged_in);
                memcpy(vals[0], version_major, strlen(version_major) + 1);
                memcpy(vals[1], version_minor, strlen(version_minor)+ 1);
                memcpy(vals[2], logged_in, 2);
                memcpy(vals[3], username, strlen(username) + 1);

                char* complete_response = (char*)malloc(response_len + 1);
                int PPHP_status = PPHP_key_val_insert(complete_response, response_len + 1, response, keys, vals, MAX_NUMBER_OF_PAGE_VAR);

                response_len = strlen(complete_response);


                size_t total = 0;
                while(total < response_len) {
                    ssize_t n = send(client_fd, complete_response + total, response_len - total, 0);
                    if(n <= 0) {
                        fprintf(stderr, "0 bytes sent. Breaking.\n");
                        break;
                    }
                    total += n;
                }

                free(response);
                free(complete_response);
                free(page_name_buffer);
                free(buffer);
                free(file_name);
                free(file_info);
                close(client_fd);
                return NULL;



            }
            else {
                
            }
            


        }

    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //obsługa logowania
    else if(connection_type == LOGIN) {

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
        //store user_id
        int user_id;

        int authentication_status = authenticate(login, password, &user_id);
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
        
        int save_token_status = save_session_id(session_id, user_id, expiry);
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

        char response[200 + session_id_length];
        char date[50];
        http_current_time(date);
        //tu trzeba zapisać token
        //free(token);
        snprintf(response, 200 + session_id_length, "HTTP/1.1 200 OK\r\n"
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
    else if(connection_type == LOGOUT) {
        //wyciągamy session_id
        //jeśli istnieje sesja (sprawdzamy czy dostarczone jest ciasteczko i czy jest w bazie)
        //to odnajdujemy je i usuwamy

        
        //wysyłamy wyłączanie cookie na stronie zawsze
        
        regcomp(&regex, "\r\nCookie: sessionId=([^ ]+)([\r\n|\r|\n]|$)", REG_EXTENDED);
        if(regexec(&regex, buffer, 2, matches, 0) == 0) {
            buffer[matches[1].rm_eo] = '\0';
            char* session_id = buffer + matches[1].rm_so;
            session_id[strcspn(session_id, "\r\n")] = '\0';

            int drop_session_status = drop_session(session_id);
            if(drop_session_status == -1) {
                fprintf(stderr, "Database could not be opened.\n");
                free(buffer);
                close(client_fd);
                return NULL;
            }
            else if(drop_session_status == -2) {
                fprintf(stderr, "DROP Query execution unsuccessful.\n");
                free(buffer);
                close(client_fd);
                return NULL;
            }
            else if(drop_session_status == -4) {
                //wysyłamy warning ale nie przerywamy wykonywania
                //warning świadczy o tym że dwukrotnie został wstawiony ten sam token
                fprintf(stderr, "Warning! Multiple session ids removed on one logout message.\n");
            }

        }
        
        char response[250];
        char date[50];
        http_current_time(date);

        snprintf(response, 250, "HTTP/1.1 302 Found\r\nLocation: main_page.html\r\nDate: %s\r\nSet-Cookie: session=; Max-Age=0; Path=/; HttpOnly; SameSite=Lax\r\n", date);
        size_t response_len = strlen(response);


        size_t total = 0;
        while(total < response_len) {
            ssize_t n = send(client_fd, response + total, response_len - total, 0);
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

    else if(connection_type == NAS) {

        #ifdef DEBUG
        printf("DEBUG mode: NAS connection type entered with buffer: %s\n", buffer);
        #endif
        //obojętnie jaki jest typ połączenia NAS i tak trzeba zrobić auth
        //wyciągnąć login i hasło do NAS
        // jdskasdmakdadw
        // s             e
        // 0             14
        char* session_id = (char*)malloc(sizeof(char) * (session_id_length + 1));
        snprintf(session_id, sizeof(char) * (session_id_length + 1), "%s", buffer + NAS_matches[3].rm_so);

        struct client_info_struct client_info;
        client_info.logged_in = false;
        client_info.elevated = 0;
        strncpy(client_info.name, "null", 30);
        strncpy(client_info.password, "default", 30);

        int authorize_status = authorize_NAS(session_id, &client_info);
        free(session_id);
        if(authorize_status == -1) {
            fprintf(stderr, "Database could not be opened.\n");
            free(buffer);
            regfree(&regex);
            close(client_fd);
            return NULL;
        }
        else if(authorize_status == -2) {
            fprintf(stderr, "Select query on Sessions unsuccessful.\n");
            free(buffer);
            regfree(&regex);
            close(client_fd);
            return NULL;
        }
        else if(authorize_status == -3 || authorize_status == -4) {
            //jeśli tokena nie ma w bazie lub jeśli jest expired
            //wysyłamy redirect do logowania oraz czyścimy nieprawidłowe ciastko
            //potem można pomyśleć o rozdzieleniu tego na dwa przypadki
            //póki nie ma mechanizmu usuwania sessionId to nie ma sensu
            char response[160];
            char date[50];
            http_current_time(date);
            regfree(&regex);
            snprintf(response, 160,
                "HTTP/1.1 302 Found\r\n"
                "Date: %s\r\n"
                "Set-Cookie: sessionId=; Max-Age=0; Path=/; HttpOnly\r\n"
                "Location: /login_page.html"
                , date);

            size_t response_len = strlen(response);
            
            size_t total = 0;
            while(total < response_len) {
                ssize_t n = send(client_fd, response + total, response_len - total, 0);
                if(n <= 0) {
                    fprintf(stderr, "0 bytes sent, closing connection.\n");
                    free(buffer);
                    close(client_fd);
                    return NULL;
                }
                total += n;
            }
            
            free(buffer);
            close(client_fd);
            return NULL;

        }

        client_info.logged_in = true;

        #ifdef DEBUG
        printf("DEBUG mode: authorization passed with NAS credentials: %s:%s\n", client_info.NAS_username, client_info.NAS_password);
        #endif

        //po zalogowaniu wyciągamy ścieżkę i typ wiadomości NAS
        size_t path_len = NAS_matches[2].rm_eo - NAS_matches[2].rm_so;
        #ifdef DEBUG
        printf("DEBUG mode: path_lel from regex matches = %ld\n", path_len);
        #endif
        char* path = (char*)malloc(sizeof(char) * (1 + path_len + 1));
        snprintf(path, sizeof(char) * (1 + path_len + 1), "/%s", buffer + NAS_matches[2].rm_so);

        #ifdef DEBUG
        printf("DEBUG mode: path: %s\n", path);
        #endif

        //łączenie z NAS
        int sockD = socket(AF_INET, SOCK_STREAM, 0);

        int connect_status = connect(sockD, (struct sockaddr*)&NAS_add, sizeof(NAS_add));
        if(connect_status == -1) {
            //TODO wysłanie errora na stronę
            fprintf(stderr, "Could not connect to NAS.\n");
            free(path);
            free(buffer);
            close(client_fd);
            return NULL;
            
        }

        #ifdef DEBUG
        printf("DEBUG mode: connected to NAS.\n");
        #endif

        //sprawdzamy która komenda została wpisana
        size_t req_len = NAS_matches[1].rm_eo - NAS_matches[1].rm_so;
        char* req = (char*)malloc(sizeof(char) * (req_len + 1));
        snprintf(req, sizeof(char) * (req_len + 1), "%s", buffer + NAS_matches[1].rm_so);
        free(buffer);

        #ifdef DEBUG
        printf("DEBUG mode: buffer freed. req: %s\n", req);
        #endif


        if(strcasecmp(req, "LIST") == 0) {
            free(req);

            #ifdef DEBUG
            printf("DEBUG mode: LIST handle entered, req freed.\n");
            #endif

            char* list;
            int list_routine_status = LIST_routine(sockD, path, client_info.NAS_username, client_info.NAS_password, &list);
            if(list_routine_status < 0) {
                //TODO tu też wypadałoby pokazywać info na stronie
                free(path);
                close(sockD);
                close(client_fd);
                fprintf(stderr, "Error number: %d occured in LIST_routine.\n", list_routine_status);
                return NULL;
            }
            else if(list_routine_status > 0) {
                free(path);
                close(sockD);
                close(client_fd);
                fprintf(stderr, "NAS returned an error: %s\n.", list);
                free(list);
                return NULL;
            }
            close(sockD);
            free(path);

            #ifdef DEBUG
            printf("DEBUG mode: LIST_routine returned with success. path freed. list: %s\n", list);
            #endif

            char* list_json;

            LIST_to_json(list, &list_json);
            free(list);

            #ifdef DEBUG
            printf("DEBUG mode: LIST_to_json ended. list freed. list_json: %s\n", list_json);
            #endif

            size_t content_len = strlen(list_json);

            char date[50];
            http_current_time(date);

            char* message = (char*)malloc(sizeof(char) * (198 + content_len));
            snprintf(message, sizeof(char) * (198 + content_len), "HTTP/1.1 200 OK\r\n"
                                                                "Date: %s\r\n"
                                                                "Content-Type: application/json\r\n"
                                                                "Content-Length: %ld\r\n"
                                                                "Connection: close\r\n"
                                                                "\r\n"
                                                                "%s", date, content_len, list_json);

            free(list_json);
            size_t message_len = strlen(message);

            #ifdef DEBUG
            printf("DEBUG mode: whole message: %s\n", message);
            #endif

            size_t total = 0;
            while(total < message_len) {
                ssize_t n = send(client_fd, message + total, message_len - total, 0);
                if(n <= 0) {
                    close(client_fd);
                    free(message);
                    fprintf(stderr, "Error occured during list json send.\n");
                    return NULL;
                }
                total += n;

            }
            free(message);
            close(client_fd);

            #ifdef DEBUG
            printf("DEBUG mode: json sent. message freed. client_fd closed. returning NULL.\n");
            #endif

            return NULL;
        }

        else if(strcasecmp(req, "MKDIR") == 0) {
            free(req);

            #ifdef DEBUG
            printf("DEBUG mode: MKDIR handle entered, req freed.\n");
            #endif

            char* response;
            int mkdir_routine_status = MKDIR_routine(sockD, path, client_info.NAS_username, client_info.NAS_password, response);
            if(mkdir_routine_status < 0) {
                //TODO trzeba wysłać na stronę error z NAS
                free(path);
                close(sockD);
                close(client_fd);
                fprintf(stderr, "Error number: %d occured in MKDIR_routine.\n", mkdir_routine_status);
                return NULL;
            }
            else if(mkdir_routine_status > 0) {
                //TODO wysyłanie errora na stronę
                free(path);
                close(sockD);
                close(client_fd);
                fprintf(stderr, "NAS returned an error: %s\n.", response);
                free(response);
                return NULL;
            }
            close(sockD);
            free(path);

            #ifdef DEBUG
            printf("DEBUG mode: MKDIR_routine returned with success. path freed.\n");
            #endif

            char response2[160];
            char date[50];
            http_current_time(date);
            snprintf(response, 160,
                    "HTTP/1.1 201 Created\r\n"
                    "Date: %s\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: 16\r\n\r\n"
                    "{\"success\":true}"
                    , date);
            
            size_t response_len = strlen(response2);

            size_t total = 0;
            while(total < response_len) {
                ssize_t n = send(client_fd, response2 + total, response_len - total, 0);
                if(n <= 0) {
                    close(client_fd);
                    fprintf(stderr, "Error occured during mkdir response send.\n");
                    return NULL;
                }
                total += n;
            }


            close(client_fd);
            return NULL;

        }

        
        else if(strcasecmp(req, "DEL") == 0) {
            free(req);

            #ifdef DEBUG
            printf("DEBUG mode: DEL handle entered, req freed.\n");
            #endif

            char* response;
            int del_routine_status = DEL_routine(sockD, path, client_info.NAS_username, client_info.NAS_password, response);
            if(del_routine_status < 0) {
                //TODO wiadomość na stronę
                free(path);
                close(sockD);
                close(client_fd);
                fprintf(stderr, "Error number: %d occured in DEL_routine.\n", del_routine_status);
                return NULL;
            }
            else if(del_routine_status > 0) {
                //TODO wysyłanie errora na stronę
                free(path);
                close(sockD);
                close(client_fd);
                fprintf(stderr, "NAS returned an error: %s.\n", response);
                free(response);
                return NULL;
            }

            close(sockD);
            free(path);

            #ifdef DEBUG
            printf("DEBUG mode: DEL routine returned with success, path freed.\n");
            #endif

            char date[50];
            http_current_time(date);
            char response2[200];

            snprintf(response2, sizeof(char) * (250), "HTTP/1.1 200 OK\r\n"
                                                    "Date: %s\r\n"
                                                    "Content-Type: application/json\r\n"
                                                    "Content-Length: 16\r\n\r\n"
                                                    "{\"success\":true}"
                                                    , date);

            size_t response_len = strlen(response2);

            size_t total = 0;
            while(total < response_len) {
                ssize_t n = send(client_fd, response2 + total, response_len - total, 0);
                if(n <= 0) {
                    close(client_fd);
                    fprintf(stderr, "Error occured during del send.\n");
                    return NULL;
                }
                total += n;
            }

            close(client_fd);
            return NULL;


        }
        /*
        else if(strcasecmp(req, "GET") == 0) {

        }
        else if(strcasecmp(req, "PUT") == 0) {

        }
        */
        //W przypadku PUT trzeba sprawdzić jeszcze nagłówki
        else {
            #ifdef DEBUG
            printf("DEBUG mode: else entered.\n");
            #endif

            fprintf(stderr, "Unknown NAS command type: %s entered despite passing regex.\n", req);

            free(req);
            regfree(&regex);
            close(sockD);
            close(client_fd);
            
            #ifdef DEBUG
            printf("DEBUG mode: req freed, regex freed, sockD closed, client_fd closed. Returning.\n");
            #endif
            return NULL;
        }
        //W przypadku GET chyba po prostu wysyłamy plik



        
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


