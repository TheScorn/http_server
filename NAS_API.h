
#include <stdbool.h>
//Basic and complex NAS communication functions

#ifndef NAS_API
#define NAS_API


enum Filetypes {
    FILET, DIRT, LINKT
};

int send_ACCEPT(int sockD);

int send_REFUSE(int sockD);

int send_ACK(int sockD);

int send_TEST(int sockD);

int TEST_routine(int sockD);

int send_LOGINTEST(int sockD, char* login, char* password);

int LOGINTEST_routine(int sockD, char* login, char* password);

int send_LIST(int sockD, char* path, char* login, char* password);

int LIST_routine(int sockD, char* path, char* login, char* password, char* list_buffer);

int LIST_to_json(char* list_buffer, char* list_json);

int send_GET(int sockD, char* path, char* login, char* password);

int send_PUT(int sockD, char* path, char* login, char* password);

int send_DEL(int sockD, char* path, char* login, char* password, bool force_flag);

int DEL_routine(int sockD, char* path, char* login, char* password, char* response);

int send_MKDIR(int sockD, char* path, char* login, char* password);

int MKDIR_routine(int sockD, char* path, char* login, char* password, char* response);

int convert(unsigned long long* result, char* str);

int recv_convert_prefix(int sockD, unsigned long long* prefix);

char* add_prefix(char* message);

int recv_message(int sockD, char* buffer, unsigned long long message_len);

int get_int_len(unsigned long long number);

#endif