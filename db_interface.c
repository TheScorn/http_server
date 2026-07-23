#include "http_server_header.h"
#include <strings.h>
#include <string.h>
#include "sqlite3.h"
#include <time.h>
#include <stdlib.h>

/**
 * @brief Function for testing db connection
 * 
 * Function tests if database is available. Checks if all tables are in place.
 * 
 * @return 0 if connection is successful, 
 * 
 */
int test_con() {
    
    sqlite3 *db;
    if(sqlite3_open("../Database/http_server.db", &db) != 0) {
        //jeśli nie ma bazy danych
        return -1;
    }

    

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
 * @return 0 if file was found, -1 if no db found, -2 if select query unsuccessful, -3 if no file found.
 */
int get_file_info(char *filename, struct file_info_struct* file_info) {

    sqlite3 *db;
    if(sqlite3_open("../../Database/http_server.db", &db) != 0) {
        //jeśli nie ma bazy danych
        return -1;
    }

    sqlite3_stmt *stmt; //statement sqlite3

    char select_statement[250];

    snprintf(select_statement, 250, "SELECT Files.file_path, Files.zone_id, Zones.zone_name, Zones.zone_type FROM Files INNER JOIN Zones ON Zones.zone_id = Files.zone_id WHERE file_name = \"%s\";", filename);


    if(sqlite3_prepare_v2(db, select_statement, -1, &stmt, NULL) != 0) {
        sqlite3_close(db);
        return -2;
    }

    if(sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_close(db);
        sqlite3_finalize(stmt);
        return -3;
    }

    const char *file_path = sqlite3_column_text(stmt, 0);
    const char *zone_name = sqlite3_column_text(stmt, 2);

    file_info->file_path = (char *)malloc((strlen(file_path) + 1) * sizeof(char));
    file_info->zone_name = (char *)malloc((strlen(zone_name) + 1) * sizeof(char));

    strcpy(file_info->file_path, (char *)file_path);
    strcpy(file_info->zone_name, (char *)zone_name);
    
    

    file_info->zone_id = sqlite3_column_int(stmt, 1);
    file_info->zone_type = sqlite3_column_int(stmt, 3);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
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


/**
 * @brief Simple function for authentication
 * 
 * Function establishes connection to database. Finds user with given login, compares passwords and returns integer depending on the outcome.
 * 
 * @param login pointer to char list with login
 * 
 * @param password pointer to char list with password
 * 
 * @return 0 if authentication correct, -1 if no database found, -2 if error occured during select execution, -3 if no user with given login found, -4 if password incorrect, -5 if execution failed.
 */
int authenticate(char* login, char* password) {
    sqlite3 *db;
    if(sqlite3_open("../../Database/http_server.db", &db) != 0) {
        //jeśli nie ma bazy danych
        return -1;
    }

    sqlite3_stmt *stmt; //statement sqlite3

    char select_statement[100];

    snprintf(select_statement, 100, "SELECT password FROM Users WHERE username = \"%s\";", login);


    if(sqlite3_prepare_v2(db, select_statement, -1, &stmt, NULL) != 0) {
        sqlite3_close(db);
        return -2;
    }

    int sqlite_step = sqlite3_step(stmt);
    if(sqlite_step == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -3;
    }
    else if(sqlite_step != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -5;
    }
    

    const unsigned char* selected_password = (const unsigned char*)sqlite3_column_text(stmt, 0);
    

    if(strcasecmp(password, selected_password) != 0) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -4;
    }
    
    //nie można finalizować stmt przed sprawdzeniem hasła bo wtedy zmienia się też selected_password
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

/**
 * @brief Function for saving new session id in db
 * 
 * Function saves generated token with corresponding username
 * 
 * @param token pointer to char list representing session id
 * 
 * @param username pointer to char list representing username
 * 
 * @param expiry int representing time when token expires
 * 
 * @return 0 if execution successful, -1 if database could not be opened, -2 if query execution unsuccessful
 */
int save_session_id(char* session_id, char* username, time_t expiry) {
    sqlite3* db;
    if(sqlite3_open("../../Database/http_server.db", &db) != 0) {
        //jeśli nie ma bazy danych
        return -1;
    }

    sqlite3_stmt *stmt; //statement sqlite3

    char insert_statement[200];


    snprintf(insert_statement, 200, "INSERT INTO Sessions (session_id, username, expiry) VALUES(\"%s\", \"%s\", %ld);", session_id, username, expiry);


    if(sqlite3_exec(db, insert_statement, NULL, NULL, NULL) != 0) {

        sqlite3_close(db);
        return -2;
    }

    return 0;
}

/**
 * @brief Authorizing function
 * 
 * 
 * 
 * @param session_id pointer to char list containing sesion_id
 * 
 * @param client_info client info struct for retrieving username and his permissions
 * 
 * @return 0 if authorization successful, -1 if database could not be opened, -2 if select query execution failed, -3 if givent session_id not found, -4 if token expired.
 * 
 */
int authorize(char* session_id, struct client_info_struct* client_info) {
    sqlite3* db;
    if(sqlite3_open("../../Database/http_server.db", &db) != 0) {
        return -1;
    }

    sqlite3_stmt* stmt;

    char select_statement[250];

    snprintf(select_statement, 250, "SELECT Sessions.username, Sessions.expiry, Users.access FROM Sessions INNER JOIN Users ON Sessions.username = Users.username WHERE session_id=\"%s\"", session_id);

    if(sqlite3_prepare_v2(db, select_statement, -1, &stmt, NULL) != 0) {
        sqlite3_close(db);
        return -2;
    }

    if(sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_close(db);
        sqlite3_finalize(stmt);
        return -3;
    }

    //sprawdzanie czy token nie jest expired
    int expiry = sqlite3_column_int(stmt, 1);

    if(time(NULL) > expiry) {
        sqlite3_close(db);
        sqlite3_finalize(stmt);
        return -4;
    }



    //zapisanie info o uzytkowniku
    const char* name = sqlite3_column_text(stmt, 0);

    client_info->name = (char *)malloc((strlen(name) + 1) * sizeof(char));

    strcpy(client_info->name, (char *)name);

    client_info->access_flags = sqlite3_column_int(stmt, 2);

    sqlite3_close(db);
    sqlite3_finalize(stmt);
    return 0;
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