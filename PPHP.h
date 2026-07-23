#include <stdlib.h>
#include <stdio.h>

#ifndef PPHP_HEADER
#define PPHP_HEADER

#ifdef __cplusplus
extern "C" {
#endif

int PPHP_first_field_insert(char* buffer, size_t buffer_size, char* unprocessed, char* value);

int PPHP_var_field_insert(char* buffer, size_t buffer_size, char* unprocessed, char* variable, char* value);

#ifdef __cplusplus
}
#endif

#endif