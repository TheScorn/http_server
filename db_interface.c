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
int get_file_info(char *filename, struct page_info_struct* page_info) {
    //trzeba będzie ogarnąć łączenie z bazą danych
    //najepiej sprawdzać je w inicie i ewentualnie przerwać init


    //teraz trochę oszukaństwo żeby nie kombinować ze złożonością póki nie mamy autoryzacji
    if(strcasecmp(filename, "test.html\0") == 0) {
        page_info->file_path = "test.html";
        page_info->zone_id = 100;
        page_info->zone_name = "Test";
        page_info->zone_type = 0;
        return 0;
    }
    else if(strcasecmp(filename, "test.js\0") == 0) {
        page_info->file_path = "test.js";
        page_info->zone_id = 100;
        page_info->zone_name = "Test";
        page_info->zone_type = 0;
        return 0;
    }

    return -1;

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