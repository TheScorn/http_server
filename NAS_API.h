
#include <stdbool.h>
//Basic and complex NAS communication functions

#ifndef NAS_API
#define NAS_API

int send_ACCEPT(int sockD);

int send_REFUSE(int sockD);

int send_ACK(int sockD);

int send_TEST(int sockD);

int TEST_routine(int sockD);

int send_LOGINTEST(int sockD, char* login, char* password);

int LOGINTEST_routine(int sockD, char* login, char* password);

int send_LIST(int sockD, char* path, char* login, char* password);

int send_GET(int sockD, char* path, char* login, char* password);

int send_PUT(int sockD, char* path, char* login, char* password);

int send_DEL(int sockD, char* path, char* login, char* password, bool force_flag);

int send_MKDIR(int sockD, char* path, char* login, char* password);

int convert(unsigned long long* result, char* str);

char* add_prefix(char* message);

int recv_message(int sockD, char* buffer, unsigned long long message_len);

#endif