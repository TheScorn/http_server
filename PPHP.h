#include <stdlib.h>
#include <stdio.h>

#ifndef PPHP_HEADER
#define PPHP_HEADER

struct matches_struct {
    char* outer_start;
    char* inner_start;
    char* inner_end;
    char* outer_end;
};


#ifdef __cplusplus
extern "C" {
#endif

int PPHP_first_field_insert(char* buffer, size_t buffer_size, char* unprocessed, char* value);

int PPHP_var_field_insert(char* buffer, size_t buffer_size, char* unprocessed, char* variable, char* value);

int PPHP_key_val_insert(char* buffer, size_t buffer_size, char* unprocessed, char keys[][20], char values[][30], int dict_len);

int find_matches(char* buffer, struct matches_struct* matches);

int PPHP_key_val_insert2(char* buffer, size_t buffer_size, char* unprocessed, char keys[][20], char values[][30], int dict_len);

#ifdef __cplusplus
}
#endif

#endif