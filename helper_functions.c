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
        return NULL;
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
    else if(strcasecmp(file_ext, "pdf") == 0) {
        return "application/pdf";
    }
    else if(strcasecmp(file_ext, "csv") == 0) {
        return "text/csv";
    }
    else if(strcasecmp(file_ext, "json") == 0) {
        return "application/json";
    }
    else if(strcasecmp(file_ext, "xml") == 0) {
        return "application/xml";
    }
    else if(strcasecmp(file_ext, "doc") == 0) {
        return "application/msword";
    }
    else if(strcasecmp(file_ext, "docx") == 0) {
        return "application/vnd.openxmlformats-officedocument.wordpressingml.document";
    }
    else if(strcasecmp(file_ext, "xls") == 0) {
        return "application/vnd.ms-excel";
    }
    else if(strcasecmp(file_ext, "xlsx") == 0) {
        return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    }
    else if(strcasecmp(file_ext, "ppt") == 0) {
        return "application/vnd.ms-powerpoint";
    }
    else if(strcasecmp(file_ext, "pptx") == 0) {
        return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    }
    else if(strcasecmp(file_ext, "rtf") == 0) {
        return "application/rtf";
    }
    else if(strcasecmp(file_ext, "zip") == 0) {
        return "application/zip";
    }
    else if(strcasecmp(file_ext, "gz") == 0) {
        return "application/gzip";
    }
    else if(strcasecmp(file_ext, "tar") == 0) {
        return "application/x-tar";
    }
    else if(strcasecmp(file_ext, "7z") == 0) {
        return "application/x-7z-compressed";
    }
    else if(strcasecmp(file_ext, "rar") == 0) {
        return "application/vnd.rar";
    }
    else if(strcasecmp(file_ext, "gif") == 0) {
        return "image/gif";
    }
    else if(strcasecmp(file_ext, "svg") == 0) {
        return "image/svg+xml";
    }
    else if(strcasecmp(file_ext, "webp") == 0) {
        return "image/webp";
    }
    else if(strcasecmp(file_ext, "tiff") == 0 || strcasecmp(file_ext, "tif") == 0) {
        return "image/tiff";
    }
    else if(strcasecmp(file_ext, "ico") == 0) {
        return "image/vnd.microsoft.icon";
    }
    else if(strcasecmp(file_ext, "mp3") == 0) {
        return "audio/mpeg";
    }
    else if(strcasecmp(file_ext, "wav") == 0) {
        return "audio/wav";
    }
    else if(strcasecmp(file_ext, "ogg") == 0) {
        return "audio/ogg";
    }
    else if(strcasecmp(file_ext, "mp4") == 0) {
        return "video/mp4";
    }
    else if(strcasecmp(file_ext, "webm") == 0) {
        return "video/webm";
    }
    else if(strcasecmp(file_ext, "mpeg") == 0) {
        return "video/mpeg";
    }
    else if(strcasecmp(file_ext, "exe") == 0) {
        return "application/vnd.microsoft.portable-executable";
    }
    else if(strcasecmp(file_ext, "wasm") == 0) {
        return "application/wasm";
    }
    else if(strcasecmp(file_ext, "bin") == 0) {
        return "application/octet-stream";
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


char* get_file_case_insensitive(const char *file_name) {
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
char* get_username(char * authorization) {

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
char* get_password(char * authorization) {

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


/**
 * @brief Function for finding last occurence of a char in string
 * 
 * 
 * @param str null terminated string
 * 
 * @param chr character to be found
 * 
 * @returns index of a last occurence of a string. -1 If no match found
 */
int last_occurence(char* str, char chr) {
    
    int last_occur = -1;
    //string strlen 6 
    for(int idx = 0; idx < strlen(str); idx++) {
        if(*(str + idx) == chr) {
            last_occur = idx;

        }

    }

    return last_occur;

}

