#include "http_server_header.h"
#include <strings.h>


/**
 * @brief Function for accquireing info aboute file from data base
 * 
 * Function(WILL BE) using connector to receive data from data base. It will
 * connect file to its zone, then return required information.
 * 
 * @param filename Name of the file in data base.
 * 
 * @param page_info Struct to be updated by the function.
 * 
 * @return 0 if file was found, -1 if wasn't.
 */
int get_file_info(char *filename, struct file_info_struct* file_info) {
    //trzeba będzie ogarnąć łączenie z bazą danych
    //najepiej sprawdzać je w inicie i ewentualnie przerwać init


    //teraz trochę oszukaństwo żeby nie kombinować ze złożonością póki nie mamy autoryzacji
    if(strcasecmp(filename, "test.html\0") == 0) {
        file_info->file_path = "test.html";
        file_info->zone_id = 100;
        file_info->zone_name = "Test";
        file_info->zone_type = 0;
        return 0;
    }
    else if(strcasecmp(filename, "test.js\0") == 0) {
        file_info->file_path = "test.js";
        file_info->zone_id = 100;
        file_info->zone_name = "Test";
        file_info->zone_type = 0;
        return 0;
    }
    else if(strcasecmp(filename, "restricted.html\0") == 0) {
        file_info->file_path = "restricted.html";
        file_info->zone_id = 0b100;
        file_info->zone_name = "Restricted";
        file_info->zone_type = 1;
        return 0;
    }
    return -1;

}


int get_client_info(char* name, char* password, struct client_info_struct* client_info) {
    //To Do (db connection)



    //zwraca 0 jeśli logowanie poprawne

    //zwraca -1 jeśli nie ma użytkownika w bazie

    //zwraca -2 jeśli hasło niepoprawne

    if(strcasecmp(name, "admin") == 0) {
        if(strcasecmp(password, "passwd") == 0) {
            client_info->name = name;
            client_info->password = password;
            client_info->email = "mucha446@gmail.com";
            client_info->access_flags = 0b111;
            client_info->logged_in = true;
            return 0;
        }
        else {
            return -2;
        }
    }
    else {
        return -1;    
    }


}





//ORGANIZACJA BAZY PLIKÓW

//TABELA Z PLIKAMI
//W tabeli musi być:
//id
//nazwa(na ten moment to co się podaje tutaj test.html)
//codename(można odpuścić ale można dodać dla ułatwienia)
//ścieżka jeśli chcemy podzielić strony na foldery
//id strefy do której należy plik

//TABELA ZE STREFAMI
//Musi w niej być
//id
//nazwa(pewnie odnosząca się do strony)
//typ(0 - dostęp wolny, 1 - administracja, 2 - personalne)
//(czy lepiej przypisać dostęp użytkownik do strefy czy strefa do użytkownika?)(chyba lepiej strefa do użytkownika)

//Strefy:
//Global - na skrypty dostępne z każdej podstrony
//Main - strona główna
//Test - strona do testów 
//  id 100
//  type 0  

//ostatecznie oddać chcemy ścieżkę do pliku, typ, nazwę i id strefy do której należy