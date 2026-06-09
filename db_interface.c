#include "http_server_header.h"
#include <strings.h>
#include <mysql/mysql.h>



/**
 * @brief Function for testing db connection
 * 
 * Function tests if database is available and if the user and password are correct.
 * 
 * @return 0 if connection is successful, -1 if error occured during initializing of MYSQL object, -2 if connection was unsuccessful.
 * 
 */
int test_con() {
    return 0;
    MYSQL *conn = mysql_init(NULL); //init a mysql structure

    if(!conn) {
        return -1;
    }

    if(!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, DB, 0, NULL, 0)) {
        return -2;
    }

    mysql_close(conn);

    return 0;

}




/**
 * @brief Function for accquiring info aboute file from data base
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
    if(strcasecmp(filename, "main_page.html\0") == 0) {
        file_info->file_path = "main_page/main_page.html";
        file_info->zone_id = 000;
        file_info->zone_name = "Main";
        file_info->zone_type = 0;
        return 0;
    }
    else if(strcasecmp(filename, "main_page.js\0") == 0) {
        file_info->file_path = "main_page/main_page.js";
        file_info->zone_id = 000;
        file_info->zone_name = "Main";
        file_info->zone_type = 0;
        return 0;
    }
    else if(strcasecmp(filename, "main_page_style.css\0") == 0) {
        file_info->file_path = "main_page/main_page_style.css";
        file_info->zone_id = 000;
        file_info->zone_name = "Main";
        file_info->zone_type = 0;
        return 0;
    }
    if(strcasecmp(filename, "login_page.html\0") == 0) {
        file_info->file_path = "login_page/login_page.html";
        file_info->zone_id = 000;
        file_info->zone_name = "Main";
        file_info->zone_type = 0;
        return 0;
    }
    else if(strcasecmp(filename, "login_page.js\0") == 0) {
        file_info->file_path = "login_page/login_page.js";
        file_info->zone_id = 000;
        file_info->zone_name = "Main";
        file_info->zone_type = 0;
        return 0;
    }
    else if(strcasecmp(filename, "login_page_style.css\0") == 0) {
        file_info->file_path = "login_page/login_page_style.css";
        file_info->zone_id = 000;
        file_info->zone_name = "Main";
        file_info->zone_type = 0;
        return 0;
    }

    else if(strcasecmp(filename, "restricted.html\0") == 0) {
        file_info->file_path = "restricted/restricted.html";
        file_info->zone_id = 0b100;
        file_info->zone_name = "Restricted";
        file_info->zone_type = 1;
        return 0;
    }
    return -1;

}



/**
 * @brief Function for acquiring and checking user info
 * 
 * Function retrieves users password, email and access flags.
 * Checks password validity and logs user by changing field "logged_in" to True.
 * 
 * @param name char list containing username
 * 
 * @param password char list containing password to be validated
 * 
 * @param client_info_struct structure containing client info to be updated
 * 
 * @return 0 if username is in database and password is correct, -1 if user does not appear in database, -2 if password is incorrect.
 * 
 */
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