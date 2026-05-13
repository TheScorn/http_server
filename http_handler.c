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
    //to jest samo w sobie okej, ale trzeba sprawdzać czy jesteśmy w pages
    //pages powinno być ustawiane podczas instalacji

    struct handle_args_struct* args = arg;

    int client_fd = *(args->client_fd);
    bool verbose = args->verbose_init;

    
    //socket

    int buffer_size = DEFAULT_BUFFER_SIZE;
    //miejsce na mechanizm przypisujący ine wartości
    ////////////////////////////////////

    char *buffer = (char *)malloc(buffer_size * sizeof(char));
    //miejsce na przychodzącą wiad

    

    //odebranie wiad
    ssize_t bytes_received = recv(client_fd, buffer, buffer_size, 0);
    
    //do tej pory jest git i się nic nie zmieni poza logami z połączeń



    if(bytes_received > 0) {//jeśli otrzymaliśmy cokolwiek
        
        regex_t regex;
        //w zasadzie wszystko tutaj trzeba zmienić
        //chcemy regexem wyciągnąć nazwę strony
        //(najlepiej żeby działało i z rozszerzeniem strony i bez)
        //tu trzeba sprawdzić czy jest kropka - jeśli jest to sprawdzić rozszrzenie i szukać tylko rzeczy przed nią
        //z nazwą strony lub pliku odwołujemy się do bazy danych(to na później teraz wpiszemy to na stałe)
        //Z bazy wyciągamy info o stronie czyli
        //1) jej nazwę pliku lub może lepiej ścieżkę(na razie niech będzie nazwa)
        //2) jej typ (0 - dostęp ogólny, 1 - dostęp po zalogowaniu(administracyjna), 2 - dostęp po zalogowaniu(strona personalna))
        //
        //każdy resource trzeba dodać do bazy i odnosić do danej przestrzeni strony
        //dostępność określamy na podstawie przestrzeni
        //(można najpierw spróbować zrobić zabezpieczenia tylko na stronach i sprawdzić czy da się dobrać do zasobów tam gdzie powinno się dać)
        //(i czy są blokowane tam gdzie powinny być)

        //główną będzie global
        
        //Jeśli jest nagłówek to:
        //Dekodujemy base64
        //Bierzemy username i szukamy użytkownika w bazie
        //Wyciągamy jego hasło
        //Robimy strcasecmp
        //Jeśli się zgadza to:
        // oddajemy stronę
        //(pomijamy na ten moment strony personalne które trzeba jakoś ustawić przed wysłaniem. Od tego będzie pewnie kolejny wielki moduł)
        //Jeśli nie to:
        //Wysyłamy 403 Forbidden

        //Jeśli nagłówka nie ma to wysyłamy 401 Unauthorized i zastanawiamy się jak to obsłużyć automatycznie

        regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);//poprawić kiedyś tak żeby przechodziła pusta strona i napisać przekierowanie na main

        regmatch_t matches[2];

        if(regexec(&regex, buffer, 2, matches, 0) != 0) {
            if(verbose) {
                printf("No match found in regexec 1. No data sent.");
            }
            close(client_fd);
            free(buffer);
            exit(0);
        }


        //tutaj chcemy działać na skopiowanym buforze żeby nie zespuć sobie potem autoryzacji
        char *page_name_buffer = (char *)malloc(sizeof(char) * buffer_size);

        strcpy(page_name_buffer, buffer);

        //ustawiamy koniec dopasowania na null terminator
        //page_name_buffer[matches[1].rm_eo] = '\0';
        page_name_buffer[matches[1].rm_eo] = '\0';


        //char z zapisaną nazwą pliku
        const char *url_encoded_file_name = page_name_buffer + matches[1].rm_so;

        //dekodowanie url
        char *file_name = url_decode(url_encoded_file_name);

        
        //stąd widzimy jak ograniczony jest regex

        //sprawdzanie typu pliku.
        char file_ext[32];
        strcpy(file_ext, get_file_extension(file_name));
        
        
        //w tym miejscu odwołujemy się do bazy danych z plikami
        //niestety chyba trzeba sprawdzić każdy i zestawić ze strefami
        struct file_info_struct file_info; //create page info structure pointer
        int db_file_info_status = get_file_info(file_name, &file_info);
        
        //Od razu można obsłużyć wszystkie przypadki gdy pliku nie udało się znaleźć

        char* response = (char *)malloc(buffer_size * 2 * sizeof(char));
        size_t response_len = strlen(response);

        //file not in db send
        if(db_file_info_status < 0) {
            snprintf(response, buffer_size,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "404 Not Found");

            response_len = strlen(response);
            
            send(client_fd, response, response_len, 0);
        }
        else {
            int build_response_status = build_http_response(file_name, file_ext, response, &response_len, buffer_size);
            if(build_response_status == -1) {
                fprintf(stderr, "File stated in data base but could not be opened.\n");
            }
        
            send(client_fd, response, response_len, 0);
            
        }

        free(response);
        free(file_name);

        
    }
    close(client_fd);
    free(buffer);


    return NULL;
}


