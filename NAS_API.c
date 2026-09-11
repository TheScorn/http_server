#include "NAS_API.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>

/**
 * @brief sending ACCEPT to NAS server.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @return 0 if execution successful, -1 if error occured during send
 */
int send_ACCEPT(int sockD) {
    char accept_message[] = "0000000000000006ACCEPT";
    size_t message_len = strlen(accept_message);

    size_t total = 0;
    while(total < message_len) {
        ssize_t n = send(sockD, accept_message + total, message_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_ACCEPT: Error inside send.\n");
            return -1;
        }
        total += n;
    }
    return 0;

}

/**
 * @brief sending REFUSE to NAS server.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @return 0 if execution successful, -1 if error occured during send
 */
int send_REFUSE(int sockD) {
    char refuse_message[] = "0000000000000006REFUSE";
    size_t message_len = strlen(refuse_message);

    size_t total = 0;
    while(total < message_len) {
        ssize_t n = send(sockD, refuse_message + total, message_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_REFUSE: Error inside send.\n");
            return -1;
        }
        total += n;
    }
    return 0;

}

/**
 * @brief sending ACK to NAS server.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @return 0 if execution successful, -1 if error occured during send
 */
int send_ACK(int sockD) {
    char ack_message[] = "0000000000000003ACK";
    size_t message_len = strlen(ack_message);

    size_t total = 0;
    while(total < message_len) {
        ssize_t n = send(sockD, ack_message + total, message_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_ACK: Error inside send.\n");
            return -1;
        }
        total += n;
    }
    return 0;
}

/**
 * @brief sending TEST to NAS server.
 * 
 * @param sockD socket desciptor. Socket should be connected to NAS beforehand.
 * 
 * @return 0 if execution successful, -1 if error occured during send.
 */
int send_TEST(int sockD) {
    char test_message[] = "0000000000000004TEST";
    size_t message_len = strlen(test_message);

    size_t total = 0;
    while(total < message_len) {
        ssize_t n = send(sockD, test_message + total, message_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_TEST: Error inside send.\n");
            return -1;
        }
        total += n;
    }
    return 0;
}

/**
 * @brief TEST for server response routine
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @return 0 if execution successful and ACK recieved, -1 if error occured in send_TEST,
 * -2 if error occured during ACK recieve, 1 if response was not ACK.
 * 
 */
int TEST_routine(int sockD) {
    if(send_TEST(sockD) != 0) {
        return -1;
    }
    
    char ack[20];
    
    size_t total = 0;
    while(total < 19) {
        ssize_t n = recv(sockD, ack + total, 19 - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function TEST_routine: 0 bytes recieved.\n");
            return -2;
        }
        total += n;
    }

    if(strncasecmp(ack, "0000000000000003ACK", 19) != 0) {
        return 1;
    }
    else {
        return 0;
    }
}

/**
 * @brief routine for testing server connection
 * 
 * Function establishes connection with NAS, then, using TEST_routine, checks server response.
 * 
 * @param NAS_add pointer to sockaddr_in struct, that stores NAS connection info.
 * 
 * @return 0 if execution successful and TEST passed, -1 if connection error occured,
 * -2 if error occured during send_TEST, -3 if error occured during ACK recieve,
 * -4 if response from server was not ACK.
 * 
 */
int test_NAS_connection(struct sockaddr_in* NAS_add) {

    int sockD = socket(AF_INET, SOCK_STREAM, 0);

    int connectStatus = connect(sockD, (struct sockaddr*)NAS_add, sizeof(*NAS_add));
    if(connectStatus == -1) {
        return -1;
    }

    int test_routine_status = TEST_routine(sockD);
    if(test_routine_status == -1) {
        close(sockD);
        return -2;
    }
    else if(test_routine_status == -2) {
        close(sockD);
        return -3;
    }
    else if(test_routine_status == 1) {
        close(sockD);
        return -4;
    }

    close(sockD);
    return 0;

}


/**
 * @brief sending LOGINTEST to NAS server.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param login pointer to null-terminated string with login
 * 
 * @param password pointer to null-terminated string with password
 * 
 * @return 0 if execution successful, -1 if error occured during send.
 */
int send_LOGINTEST(int sockD, char* login, char* password) {
    char* logintest_raw = (char*)malloc(sizeof(char) * (27 + strlen(login) + strlen(password)));
    snprintf(logintest_raw, sizeof(char) * (27 + strlen(login) + strlen(password)), "LOGINTEST login:%s password:%s", login, password);

    char* logintest = add_prefix(logintest_raw);
    free(logintest_raw);
    logintest_raw = NULL;

    size_t message_len = strlen(logintest);

    size_t total = 0;
    while(total < message_len) {
        ssize_t n = send(sockD, logintest + total, message_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_LOGINTEST: Error inside send.\n");
            free(logintest);
            return -1;
        }
        total += n;
    }
    free(logintest);
    return 0;
}

/**
 * @brief Encapsulation of LOGINTEST routine.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param login pointer to null-terminated string with login
 * 
 * @param password pointer to null-terminated string with password
 * 
 * @return 0 if execution successful with valid login, 
 * -1 if send_LOGINTEST returned with an error,
 * -2 if recv_convert_prefix returned with an error,
 * -3 if recv_message returned with an error
 * 1 if server responded with LOGINFALSE
 * 
 */
int LOGINTEST_routine(int sockD, char* login, char* password) {
    if(send_LOGINTEST(sockD, login, password) != 0) {
        return -1;
    }

    unsigned long long prefix;

    int recv_prefix_status = recv_convert_prefix(sockD, &prefix);
    if(recv_prefix_status != 0) {
        return -2;
    }

    char* buffer = (char*)malloc(sizeof(char) * (prefix + 1));

    int recv_message_status = recv_message(sockD, buffer, prefix);
    if(recv_message_status != 0) {
        free(buffer);
        return -3;
    }

    if(strcasecmp(buffer, "LOGINFALSE") == 0) {
        free(buffer);
        return 1;
    }
    else if(strcasecmp(buffer, "LOGINACK") != 0) {
        free(buffer);
        return -4;
    }
    
    return 0;

}


/**
 * @brief Conversion function from str to ull
 * 
 * @param result pointer to unsigned long long where result is stored
 * 
 * @param str pointer to null-terminated string containing hex value.
 * 
 * @return 0 if conversion successful, -1 if string was not a number,
 * -2 i the value does not fit in ull, -3 if 
 */
int convert(unsigned long long* result, char* str) {
    errno = 0;
    char* end;
    *result = strtoull(str, &end, 16);
    
    if(*result == 0 && end == str) {
        fprintf(stderr, "Function convert: String was not a number.\n");
        return -1;
    }
    else if(*result == ULLONG_MAX && errno) {
        fprintf(stderr, "Function convert: Value does not fit in unsigned long long.\n");
        return -2;
    }
    else if(*end) {
        fprintf(stderr, "Function convert: str had garbage chars in the end.\n");
        return -3;
    }
    return 0;
}

/**
 * @brief Prepend message with length-prefix
 * 
 * Function prepends message with 16 hexadecimal digits describing its length.
 * NOTE! returned message is dynamically alocated so it needs to be freed outside this function.
 * 
 * @param message pointer to null-terminated string with message to be modified
 * 
 * @return null-terminated string prepended with length prefix. 
 */
char* add_prefix(char* message) {
    size_t message_len = strlen(message);

    char* prefixed_message = (char*)malloc(sizeof(char) * (message_len + 16 + 1));

    snprintf(prefixed_message, sizeof(char) * (message_len + 16 + 1), "%016zX%s", message_len, message);

    return prefixed_message;
}

/**
 * @brief recieving and converting prefix to ull routine.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param prefix pointer to ull where converted prefix should be stored.
 * 
 * @return 0 if execution successful, -1 if server closed connection,
 * -2 if error occured during prefix recieve, -3 if error occured during conversion.
 */
int recv_convert_prefix(int sockD, unsigned long long* prefix) {
    char* prefix_buffer = (char*)malloc(sizeof(char) * 17);
    
    size_t bytes_recieved = 0;
    while(bytes_recieved < 16) {
        ssize_t n = recv(sockD, prefix_buffer + bytes_recieved, 16 - bytes_recieved, 0);

        if(n == 0) {
            fprintf(stderr, "Function recv_convert_prefix; Server closed connection.\n");
            free(prefix_buffer);
            return -1;
        }
        else if(n < 0) {
            fprintf(stderr, "Function recv_convert_prefix; Error occured during prefix recv.\n");
            free(prefix_buffer);
            return -2;
        }

        bytes_recieved += n;

    }

    *(prefix_buffer + 16) = '\0';

    int conversion_status = convert(prefix, prefix_buffer);
    free(prefix_buffer);
    if(conversion_status == -1) {
        fprintf(stderr, "Function recv_convert_prefix; Prefix was not a number.\n");
        return -3;
    }
    else if(conversion_status == -2) {
        fprintf(stderr, "Function recv_convert_prefix; Value converted does not fit in unsigned long long.\n");
        return -3;
    }
    else if(conversion_status == -3) {
        fprintf(stderr, "Function recv_convert_prefix; Prefix contained garbage values.\n");
        return -3;
    }

    return 0;

}

/**
 * @brief Simple recieve message routine.
 * 
 * Function recieves and stores message in buffer.
 * 
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param buffer pointer to char array where recieved message should be stored.
 * After successful execution it holds null-terminated string.
 * NOTE that the memory for the buffer should be allocated before using this function
 * and that the buffer needs to be at least one byte longer than the
 * message_len, so that null-terminator can fit.
 * 
 * @param message_len length of expected message in bytes
 * 
 * @return 0 if execution successful, -1 if server closed connection,
 * -2 if error occured during recv.
 */
int recv_message(int sockD, char* buffer, unsigned long long message_len) {
    size_t bytes_recieved = 0;

    while(bytes_recieved < message_len) {
        ssize_t n = recv(sockD, buffer + bytes_recieved, message_len - bytes_recieved, 0);
        if(n == 0) {
            return -1;
        }
        else if(n < 0) {
            return -2;
        }
        bytes_recieved += n;
    }

    *(buffer + message_len) = '\0';
    
    return 0;
}


/**
 * @brief Sending LIST to NAS server.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param path pointer to null-terminated string with path
 * 
 * @param login pointer to null-terminated string with login
 * 
 * @param password pointer to null-terminated string with password
 * 
 * @return 0 if execution successful, -1 if error occured during send.
 */
int send_LIST(int sockD, char* path, char* login, char* password) {
    
    char* request_raw = (char*)malloc(sizeof(char) * (5 + strlen(path) + 7 + strlen(login) + 10 + strlen(password) + 1));
    snprintf(request_raw, sizeof(char) * (5 + strlen(path) + 7 + strlen(login) + 10 + strlen(password) + 1), "LIST %s login:%s password:%s", path, login, password);

    char* request = add_prefix(request_raw);
    free(request_raw);
    request_raw = NULL;

    size_t request_len = strlen(request);

    size_t total = 0;
    while(total < request_len) {
        ssize_t n = send(sockD, request + total, request_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_LIST: Error inside send.\n");
            free(request);
            return -1;
        }
        total += n;
    }
    free(request);
    return 0;
}

/**
 * @brief routine for sending LIST request and recieving answer.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param path pointer to null-terminated string with path.
 * 
 * @param login pointer to null-terminated string with login.
 * 
 * @param password pointer to null-terminated string with password.
 * 
 * @param list_buffer pointer to null-terminated string where the LIST answer
 * from server should be stored. If this function returns 0,
 * the memory for the list will be dynamically allocated
 * and needs to be freed manually after use.
 * If function returns with value > 0,
 * this string will be set to the error recieved from NAS server.
 * In this case it should be freed as well.
 * On fatal errors (return > 0) list_buffer will be freed automatically.
 * 
 * @return 0 if execution successful, -1 if error occured inside send_LIST,
 * -2 if server closed connection while sending prefix,
 * -3 if error occured during prefix recieve,
 * -4 if error occured during conversion,
 * -5 if server closed connection while sending list,
 * -6 if error occured while sending list.
 * 1 if server sent an error message instead of list.
 */
int LIST_routine(int sockD, char* path, char* login, char* password, char** list_buffer) {
    char status;
    
    status = send_LIST(sockD, path, login, password);
    if(status == -1) {
        return -1;
    }

    unsigned long long prefix;

    status = recv_convert_prefix(sockD, &prefix);
    if(status == -1) {
        return -2;
    }
    else if(status == -2) {
        return -3;
    }
    else if(status == -3) {
        return -4;
    }

    *list_buffer = (char*)malloc(sizeof(char) * (prefix + 1));

    status = recv_message(sockD, *list_buffer, prefix);



    if(status == -1) {
        free(*list_buffer);
        return -5;
    }
    else if(status == -2) {
        free(*list_buffer);
        return -6;
    }

    if(strncasecmp(*list_buffer, "ERROR", 5) == 0) {
        return 1;
    }

    return 0;

}

/**
 * @brief Function for creating list of files as json
 * 
 * @param list_buffer pointer to null-termiated string containing raw list message from NAS
 * 
 * @param list_json char pointer where json formatted list should be stored
 * 
 * @return 0 if execution successul
 * 
 */
int LIST_to_json(char* list_buffer, char** list_json) {
    
    //czytamy plik za plikiem
    
    //wstawiamy [ jako początek listy obiektów.
    *list_json = (char*)malloc(sizeof(char) * 2);
    snprintf(*list_json, sizeof(char) * 2, "[");
    bool last;
    //wartość wskazująca na następny niezapisany char w list_json
    int next_to_write = 1;

    char* file_size_buffer = (char*)malloc(sizeof(char) * 17);
    char* last_mod_buffer = (char*)malloc(sizeof(char) * 17);
    char* nml_buffer = (char*)malloc(sizeof(char) * 5);
    char* filename;

    uint16_t namelength;
    unsigned long long file_size;
    unsigned long long last_mod;
    int file_size_len;
    int last_mod_len;

    enum Filetypes ft;
    char type;
    int reader = 0;
    while(reader < strlen(list_buffer)) {
        type = *(list_buffer + reader);
        if(type == '0') {
            ft = FILET;
        }
        else if(type == '1') {
            ft = DIRT;
        }
        else {
            ft = LINKT;
        }

        reader += 1;

        //czytamy 16, konwertujemy i zapisujemy wielkość pliku
        snprintf(file_size_buffer, 17, "%s", list_buffer + reader);
        file_size = (unsigned long long)strtoull(file_size_buffer, NULL, 16);
        
        reader += 16;
        
        //czytamy 16, konwertujemy na dec
        snprintf(last_mod_buffer, 17, "%s", list_buffer + reader);
        last_mod = (unsigned long long)strtoull(last_mod_buffer, NULL, 16);

        reader += 16;

        //czytamy 4 i zapisujemy wielkość nazwy
        snprintf(nml_buffer, 5, "%s", list_buffer + reader);
        namelength = (uint16_t)strtoul(nml_buffer, NULL, 16);

        reader += 4;

        filename = (char*)malloc(sizeof(char) * (namelength + 1));
        snprintf(filename, sizeof(char) * (namelength + 1), "%s", list_buffer + reader);


        //ile miejsca to wszystko zajmie
        //{"name":"{nazwa}","type":{0/1/2},"size":{size},"mtime":{mtime}}
        file_size_len = get_int_len(file_size);
        last_mod_len = get_int_len(last_mod);

        if(reader + namelength + 2 > strlen(list_buffer)) {
            last = true;
        }
        else {
            last = false;
        }
        //realloc
        //w zależności czy to ostatni obiekt
        if(last) {
            *list_json = realloc(*list_json, sizeof(char) * (strlen(*list_json) + 9 + namelength + 18 + file_size_len + 9 + last_mod_len + 3));
            snprintf(*list_json + next_to_write, sizeof(char) * (9 + namelength + 18 + file_size_len + 9 + last_mod_len + 3), "{\"name\":\"%s\",\"type\":%c,\"size\":%lld,\"mtime\":%lld}]", filename, type, file_size, last_mod);
        }
        else {
            *list_json = realloc(*list_json, sizeof(char) * (strlen(*list_json) + 9 + namelength + 18 + file_size_len + 9 + last_mod_len + 3));
            snprintf(*list_json + next_to_write, sizeof(char) * (9 + namelength + 18 + file_size_len + 9 + last_mod_len + 3), "{\"name\":\"%s\",\"type\":%c,\"size\":%lld,\"mtime\":%lld},", filename, type, file_size, last_mod);
        }
        
        free(filename);
        //po dopisaniu zmieniamy next_to_write na strlen jsona
        next_to_write = strlen(*list_json);



        reader += namelength;


    }


    free(file_size_buffer);
    free(last_mod_buffer);
    free(nml_buffer);

    return 0;

}


/**
 * @brief Sending GET to NAS server.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param path pointer to null-terminated string with path
 * 
 * @param login pointer to null-terminated string with login
 * 
 * @param password pointer to null-terminated string with password
 * 
 * @return 0 if execution successful, -1 if error occured during send.
 */
int send_GET(int sockD, char* path, char* login, char* password) {
    
    char* request_raw = (char*)malloc(sizeof(char) * (4 + strlen(path) + 7 + strlen(login) + 10 + strlen(password) + 1));
    snprintf(request_raw, sizeof(char) * (4 + strlen(path) + 7 + strlen(login) + 10 + strlen(password) + 1), "GET %s login:%s password:%s", path, login, password);

    char* request = add_prefix(request_raw);
    free(request_raw);
    request_raw = NULL;

    size_t request_len = strlen(request);

    size_t total = 0;
    while(total < request_len) {
        ssize_t n = send(sockD, request + total, request_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_GET: Error inside send.\n");
            free(request);
            return -1;
        }
        total += n;
    }
    free(request);
    return 0;
}

/**
 * @brief Sending PUT to NAS server.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param path pointer to null-terminated string with path
 * 
 * @param login pointer to null-terminated string with login
 * 
 * @param password pointer to null-terminated string with password
 * 
 * @return 0 if execution successful, -1 if error occured during send.
 */
int send_PUT(int sockD, char* path, char* login, char* password) {
    
    char* request_raw = (char*)malloc(sizeof(char) * (4 + strlen(path) + 7 + strlen(login) + 10 + strlen(password) + 1));
    snprintf(request_raw, sizeof(char) * (4 + strlen(path) + 7 + strlen(login) + 10 + strlen(password) + 1), "PUT %s login:%s password:%s", path, login, password);

    char* request = add_prefix(request_raw);
    free(request_raw);
    request_raw = NULL;

    size_t request_len = strlen(request);

    size_t total = 0;
    while(total < request_len) {
        ssize_t n = send(sockD, request + total, request_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_PUT: Error inside send.\n");
            free(request);
            return -1;
        }
        total += n;
    }
    free(request);
    return 0;
}


/**
 * @brief Sending DEL to NAS server.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param path pointer to null-terminated string with path
 * 
 * @param login pointer to null-terminated string with login
 * 
 * @param password pointer to null-terminated string with password
 * 
 * @param force_flag boolean value setting the forced deletion option
 * 
 * @return 0 if execution successful, -1 if error occured during send.
 */
int send_DEL(int sockD, char* path, char* login, char* password, bool force_flag) {
    
    char* request_raw = (char*)malloc(sizeof(char) * (4 + strlen(path) + 9 + strlen(login) + 10 + strlen(password) + 1));
    snprintf(request_raw, sizeof(char) * (4 + strlen(path) + 9 + strlen(login) + 10 + strlen(password) + 1), "DEL %s %d login:%s password:%s", path, (int)force_flag, login, password);

    char* request = add_prefix(request_raw);
    free(request_raw);
    request_raw = NULL;

    size_t request_len = strlen(request);

    size_t total = 0;
    while(total < request_len) {
        ssize_t n = send(sockD, request + total, request_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_DEL: Error inside send.\n");
            free(request);
            return -1;
        }
        total += n;
    }
    free(request);
    return 0;
}

/**
 * @brief routine for sending DEL and recieving answer.
 * 
 * This function always sends force_delete flag set to true.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param path pointer to null-terminated string with path.
 * 
 * @param login pointer to null-terminated string with login.
 * 
 * @param password pointer to null-terminated string with password.
 * 
 * @param response pointer to null-terminated string with response from NAS.
 * This pointer will be dynamically allocated after use 
 * if this function returns with value > 0 and it will contain message from server.
 * In this case this memory needs to be manually freed.
 * For any other return value, user should not try to free this pointer.
 * 
 * @return 0 if execution successful, -1 if error occured while sending DEL request,
 * -2 if server closed connection while sending prefix,
 * -3 if error occured during prefix recieve,
 * -4 if conversion error occured,
 * -5 if server closed connection while sending response
 * -6 if error occured during response recieve
 * -7 if unexpected message from NAS recieved
 * 1 if 
 */
int DEL_routine(int sockD, char* path, char* login, char* password, char* response) {
    char status;
    if(send_DEL(sockD, path, login, password, true) == -1) {
        return -1;
    }

    unsigned long long prefix;
    status = recv_convert_prefix(sockD, &prefix);
    if(status == -1) {
        return -2;
    }
    else if(status == -2) {
        return -3;
    }
    else if(status == -3) {
        return -4;
    }

    response = (char*)malloc(sizeof(char) * (prefix + 1));

    status = recv_message(sockD, response, prefix);
    if(status == -1) {
        free(response);
        return -5;
    }
    else if(status == -2) {
        free(response);
        return -6;
    }


    if(strncasecmp(response, "ERROR", 5) == 0) {
        return 1;
    }
    else if(strncasecmp(response, "ACK", 3) != 0) {
        free(response);
        return -7;
    }
    free(response);
    return 0;

}



/**
 * @brief Sending MKDIR to NAS server.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param path pointer to null-terminated string with path
 * 
 * @param login pointer to null-terminated string with login
 * 
 * @param password pointer to null-terminated string with password
 * 
 * @return 0 if execution successful, -1 if error occured during send.
 */
int send_MKDIR(int sockD, char* path, char* login, char* password) {
    
    char* request_raw = (char*)malloc(sizeof(char) * (6 + strlen(path) + 7 + strlen(login) + 10 + strlen(password) + 1));
    snprintf(request_raw, sizeof(char) * (6 + strlen(path) + 7 + strlen(login) + 10 + strlen(password) + 1), "MKDIR %s login:%s password:%s", path, login, password);

    char* request = add_prefix(request_raw);
    free(request_raw);
    request_raw = NULL;

    size_t request_len = strlen(request);

    size_t total = 0;
    while(total < request_len) {
        ssize_t n = send(sockD, request + total, request_len - total, 0);
        if(n <= 0) {
            fprintf(stderr, "Function send_MKDIR: Error inside send.\n");
            free(request);
            return -1;
        }
        total += n;
    }
    free(request);
    return 0;
}


/**
 * @brief routine for sending MKDIR and recieving answer.
 * 
 * @param sockD socket descriptor. Socket should be connected to NAS beforehand.
 * 
 * @param path pointer to null-terminated string with path.
 * 
 * @param login pointer to null-terminated string with login.
 * 
 * @param password pointer to null-terminated string with password.
 * 
 * @param response pointer to null-terminated string with response from NAS.
 * This pointer will be dynamically allocated after use 
 * if this function returns with value > 0 and it will contain message from server.
 * In this case this memory needs to be manually freed.
 * For any other return value, user should not try to free this pointer.
 * 
 * @return 0 if execution successful, -1 if error occured while sending MKDIR request,
 * -2 if server closed connection while sending prefix,
 * -3 if error occured during prefix recieve,
 * -4 if conversion error occured,
 * -5 if server closed connection while sending response
 * -6 if error occured during response recieve
 * -7 if unexpected message from NAS recieved
 * 1 if 
 */
int MKDIR_routine(int sockD, char* path, char* login, char* password, char* response) {
    char status;
    if(send_MKDIR(sockD, path, login, password) == -1) {
        return -1;
    }

    unsigned long long prefix;
    status = recv_convert_prefix(sockD, &prefix);
    if(status == -1) {
        return -2;
    }
    else if(status == -2) {
        return -3;
    }
    else if(status == -3) {
        return -4;
    }

    response = (char*)malloc(sizeof(char) * (prefix + 1));

    status = recv_message(sockD, response, prefix);
    if(status == -1) {
        free(response);
        return -5;
    }
    else if(status == -2) {
        free(response);
        return -6;
    }


    if(strncasecmp(response, "ERROR", 5) == 0) {
        return 1;
    }
    else if(strncasecmp(response, "ACK", 3) != 0) {
        free(response);
        return -7;
    }
    free(response);
    return 0;

}

int get_int_len(unsigned long long number) {
    int length = 0;

    if(number == 0) {
        return 1;
    }

    while(number > 0) {
        number /= 10;
        length++;
    }
    return length;

}