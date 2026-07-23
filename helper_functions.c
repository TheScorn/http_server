#include "http_server_header.h"
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
 * @brief Function for finding file extensions
 * 
 * Function returns pointer to the first char in filename after '.' char.
 * 
 * @param filename const char pointer to char list containing filename
 * 
 * @return pointer to beginning of file extension
 * 
 */
const char *get_file_extension(const char *filename) {

    const char *dot = strrchr(filename, '.');

    if(!dot || dot == filename) {
        return "";
    }

    return dot + 1;

}

/**
 * @brief Function returning mime-types
 * 
 * Function matches file extensions to their web page equvalent.
 * 
 * @param file_ext Constant pointer to char list containing file extension.
 * 
 * @return Constant pointer to char list containing mime-type.
 * 
 * 
 */
const char *get_mime_type(const char *file_ext) {
    if(strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext,"htm") == 0) {
        return "text/html";    
    }
    else if(strcasecmp(file_ext, "css") == 0) {
        return "text/css";
    }
    else if(strcasecmp(file_ext, "js") == 0) {
        return "application/javascript";
    }
    else if(strcasecmp(file_ext, "txt") == 0) {
        return "text/plain";
    }
    else if(strcasecmp(file_ext, "png") == 0) {
        return "image/png";
    }
    else if(strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0) {
        return "image/jpeg";
    }
    else {
        return "application/octet-stream";
    }

}

bool case_insensitive_compare(const char *word1, const char *word2) {
    while(*word1 && *word2) {
        if(tolower((unsigned char)*word1) != tolower((unsigned char)*word2)) {
            return false;
        }
        word1++;
        word2++;
    }

    return *word1 == *word2;
}


char *get_file_case_insensitive(const char *file_name) {
    DIR *dir = opendir(".");
    if(dir == NULL) {
        fprintf(stderr, "No valid dir");
        return NULL;
    }

    struct dirent *entry;
    char *found_file_name = NULL;
    while ((entry = readdir(dir)) != NULL ) {
        if(case_insensitive_compare(entry->d_name, file_name)) {
            found_file_name = entry->d_name;
            break;
        }
    }

    closedir(dir);
    return found_file_name;

}

/**
 * @brief Function for getting login from authorization
 * 
 * Function returns login from http base64 decoded authorization header in the form of login:password
 * 
 * @param authorization pointer to char list containing authorization info
 * 
 * @return pointer to char list containing username
 * 
 */
char * get_username(char * authorization) {

    char * pointer_to_colon = strchr(authorization, ':');

    int index_of_colon = (int)(pointer_to_colon - authorization);

    char * username = (char *)malloc(strlen(authorization) + 1);
    strcpy(username, authorization);
    username[index_of_colon] = '\0';

    return username;


}

/**
 * @brief Function for getting password from authorization
 * 
 * Function returns password from http base64 decoded authorization header in the form of login:password
 * 
 * @param authorization pointer to char list containing authorization info
 * 
 * @return pointer to char list containing password
 */
char * get_password(char * authorization) {

    char * pointer_to_colon = strchr(authorization, ':');

    return pointer_to_colon + 1;

}


/**
 * @brief url decoder
 * 
 * Function for decoding strings saved in an ASCII format
 * 
 * @param src pointer to char list containing URL in ASCII format
 * 
 * @return pointer to decoded character list
 * 
 */
char *url_decode(const char *src) {
    size_t src_len = strlen(src);

    char *decoded = (char *)malloc(src_len + 1);

    size_t decoded_len = 0;

    for(size_t i = 0; i < src_len; i++) {
        if(src[i] == '%' && i+2 < src_len) {
            int hex_val;
            sscanf(src + i + 1, "%2x", &hex_val);
            decoded[decoded_len++] == hex_val;
            i += 2;
        }
        else {
            decoded[decoded_len++] = src[i];
        }
    }

    decoded[decoded_len] = '\0';
    return decoded;
}



char charset[] = "0123456789abcdefghijklmnopqrstuwxyzABCDEFGHIJKLMNOPQRSTUWXYZ-._~+/";

/**
 * @brief Token generator
 * 
 * Function generates token of given length
 * 
 * @param length length of token generated
 * 
 * @param token address of the token
 */
void generate_session_id(int length, char* token) {
    
    char next_ix;
    char next;
    srand(time(0));
    for(int i = 0; i < length; i++) {
        
        next_ix = rand() % (sizeof(charset) - 1);
        next = charset[next_ix];
        *(token + i) = next;
    }
    token[length] = '\0';
    
}

/**
 * @brief HTTP Date
 * 
 * Function produces HTTP formated GMT date and time.
 * 
 * @param date address for the date
 * 
 * @return 0 if execution successful
 */
int http_current_time(char* date) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    strftime(date, 50, "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return 0;

}


