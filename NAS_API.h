

//Basic and complex NAS communication functions

#ifndef NAS_API
#define NAS_API

int send_ACCEPT(int sockD);

int send_REFUSE(int sockD);

int send_ACK(int sockD);

int send_TEST(int sockD);

int send_LOGINTEST(int sockD, char* login, char* password);

int convert(unsigned long long* result, char* str);

char* add_prefix(char* message);



#endif