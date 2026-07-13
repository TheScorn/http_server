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
    regcomp(&regex, "^POST[[:space:]]+/[[:space:]]+HTTP/1\.[01]\r?\n(.|\n)*Content-Type:[[:space:]]*multipart/form-data;[[:space:]]*boundary=([^\r\n]+)(.|\n)*name=\"login\"\r?\n\r?\n([^\r\n]+)(.|\n)*name=\"password\"\r?\n\r?\n([^\r\n]+)", REG_EXTENDED | REG_NEWLINE);
    if(regexec(&regex, buffer, 2, matches, 0) == 0) {
        connection_type = 2;
    }

    
    //sprawdzanie czy typ 3
    //TODO


    //Obsługa http request
    if(connection_type == 1) {
        struct client_info_struct client_info;
        client_info.logged_in = false;
        client_info.name = "default";
        client_info.password = "default";
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

        //jeśli w bazie nie ma takiego pliku
        if(db_file_info_status < 0) {
            snprintf(response, buffer_size,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "404 Not Found");

            response_len = strlen(response);
            
            send(client_fd, response, response_len, 0);
            free(response);
            free(page_name_buffer);
            free(file_name);
            close(client_fd);
            return NULL;
        }



        //////////////////////////////////////////////////////
        //TUTUTUTUTUTUTUTUT
        //TU TRZEBA ZROBIĆ
        //
        //Jak obsługiwać bearer token
        //
        //Czy strona wymaga tokena:
        //TAK:
        //  Sprawdzamy czy jest token w requeście:
        //  TAK:
        //     Procesujemy i jeśli git to
        //     Wysyłamy odpowiedź ze stroną
        //  NIE:
        //     Wysyłamy 401 z podanym sposobem autoryzacji
        //    
        //NIE:
        //  Wysyłamy stronę
        //


        if(file_info.zone_type != 0) {
            //strona wymaga tokena
            regcomp(&regex, "\r\nAuthorization: Bearer ([^ ]+)([\r\n|\r|\n]|$)", REG_EXTENDED);
            if(regexec(&regex, buffer, 2, matches, 0) != 0) {
                //strona wymaga tokena, a tokena nie ma
                snprintf(response, buffer_size,
                    "HTTP/1.1 401 Unauthorized\r\n"
                    "WWW-Authenticate: Bearer realm=\"Global\""
                    );

                response_len = strlen(response);
                send(client_fd, response, response_len, 0);
                
                free(response);
                free(page_name_buffer);
                free(file_name);
                close(client_fd);
                return NULL;
                
            }

            buffer[matches[1].rm_eo] = '\0';

            char* auth_token = buffer + matches[1].rm_so;
            auth_token[strcspn(auth_token, "\r\n")] = '\0';
            //tutaj trzeba zrobić call do db albo struktury trzymającej tokeny
            //na tej podstawie do tokenu przypiszemy login
            //będzie potrzebny skrypt który co jakiś czas wyczyści bazę ze starych tokenów

            //jesli się zgadza to po prostu wychodzi z ifa dalej
            //jeśli nie to wysyła forbidden albo inny error w zależnośli od stanu tokena

        }
        
        //tu wchodzimy tylko jeśli nie jest wymagany token, albo jeśli jest wymagany i jest przesłany
        //Jeśli go nie ma to funkcja zakończy sie wcześniej
        int build_response_status = build_http_response(file_info.file_path, file_ext, response, &response_len, buffer_size);
        if(build_response_status == -1) {
            fprintf(stderr, "File stated in data base but could not be opened.\n");
            free(response);
            free(page_name_buffer);
            free(file_name);
            close(client_fd);
            return NULL;
        }
        
        send(client_fd, response, response_len, 0);
        free(response);
        free(page_name_buffer);
        free(file_name);
        close(client_fd);
        return NULL;
    }


    //obsługa logowania
    else if(connection_type == 2) {

        regmatch_t matches2[4];
        char login[32];
        char password[32];

        regcomp(&regex, "form-data; name=\"login\"\r?\n\r?\n([^\r\n]+)(.|\n)*name=\"password\"\r?\n\r?\n([^\r\n]+)", REG_EXTENDED | REG_NEWLINE);

        //czy jest sens sprawdzać ponownie poprawność - raczej nie, ale póki co zostaje

        if(regexec(&regex, buffer, 4, matches2, 0) != 0) {
            fprintf(stderr, "No login info found");
        }

        int len = matches2[1].rm_eo - matches2[1].rm_so;
        strncpy(login, buffer + matches2[1].rm_so, len);
        login[len] = '\0';

        len =  matches2[3].rm_eo - matches2[3].rm_so;
        strncpy(password, buffer + matches2[3].rm_so, len);
        password[len] = '\0';

        
        //obsługa logowania

        //sprawdzamy bazę danych
        //jeśli użytkownik istnieje i hasło poprawne to tworzymy token
        //stara funkcja nie działa bo trzeba od razu dopisać do użytkownika token

        

        //trzeba wyciągnąć hasło i login
        //porównać z bazą
        //jesli działa to stworzyć i wysłać token
        //przypisać login do tokenu
        free(buffer);
        close(client_fd);
        return NULL;
    }


    //obsługa wylogowania
    else if(connection_type == 3) {
        free(buffer);
        close(client_fd);
        return NULL;
    }

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


