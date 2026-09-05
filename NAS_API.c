#include "NAS_API.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>

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
        fprint(stderr, "Function convert: str had garbage chars in the end.\n");
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

