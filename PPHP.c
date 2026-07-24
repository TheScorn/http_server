#include <regex.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "PPHP.h"
//chcemy móc w zaczytanym stringu znaleźć pola typu
//funkcja typu simple insert
//szuka pierwszego wystąpienia

/**
 * @brief Function for simple single insert operations
 * 
 * Function finds first field and replaces it with given value
 * 
 * @param buffer space for processed text
 * 
 * @param buffer_size size of buffer space
 * 
 * @param unprocessed char pointer to unprocessed text
 * 
 * @param value char pointer to value which should be inserted
 * 
 * @return 0 if execution successful, -1 if no field found
 */
int PPHP_first_field_insert(char* buffer, size_t buffer_size, char* unprocessed, char* value) {
    regmatch_t matches[2];
    
    regex_t regex;
    regcomp(&regex, "<<([^ ]+)>>", REG_EXTENDED);

    if(regexec(&regex, unprocessed, 2, matches, 0) != 0) {
        regfree(&regex);
        snprintf(buffer, buffer_size, "%s", unprocessed);
        return -1;
    } 

    
    char* before_field = unprocessed;
    before_field[matches[0].rm_so] = '\0';

    char* after_field = unprocessed + matches[0].rm_eo;


    snprintf(buffer, buffer_size, "%s%s%s", before_field, value, after_field);

    return 0;
}

/**
 * @brief Function for insert operation based on variable
 * 
 * Function finds first field with variable that matches param "variable"
 * 
 * @param buffer space for processed text
 * 
 * @param buffer_size size of buffer_space
 * 
 * @param unprocessed char pointer to unprocessed text
 * 
 * @param variable char pointer to variable of field to be replaced
 * 
 * @param value char pointer to value which should be inserted
 * 
 * @return 0 if execution successful, -1 if no field found
 * 
 */
int PPHP_var_field_insert(char* buffer, size_t buffer_size, char* unprocessed, char* variable, char* value) {
    regmatch_t matches[2];

    regex_t regex;
    
    char expression[40];
    snprintf(expression, 41, "<<%s>>", variable);

    regcomp(&regex, expression, REG_EXTENDED);

    if(regexec(&regex, unprocessed, 2, matches, 0) != 0) {
        regfree(&regex);
        snprintf(buffer, buffer_size, "%s", unprocessed);
        return -1;
    }

    char* befeore_field = unprocessed;
    befeore_field[matches[0].rm_so] = '\0';
    
    char* after_field = unprocessed + matches[0].rm_eo;

    snprintf(buffer, buffer_size, "%s%s%s", befeore_field, value, after_field);
    return 0;
}

/**
 * @brief Function for inserting values to coresponding fields
 * 
 * Functions seeks for fields, then finds variable inside and checks if it's known. If so, inserts corresponding value from values.
 * 
 * @param buffer space for processed text
 * 
 * @param buffer_size size of buffer space
 * 
 * @param unproessed char pointer to unprocessed text
 * 
 * @param keys list of char arrays containing keys to be searched for inside fields. Assuming keys are not longer than 20 characters.
 * 
 * @param values list of char arrays containg values with indexes corresponding with keys. Assuming the values are not longer then 30 characters
 * 
 * @param dict_len number of keys and values
 * 
 * @return 0 if execution successful, -1 if unknown variable found.
 * 
 */
int PPHP_key_val_insert(char* buffer, size_t buffer_size, char* unprocessed, char keys[][20], char values[][30], int dict_len) {
    regmatch_t matches[2];

    regex_t regex;

    //szukamy najpierw pierwszego wystąpienia <<>>
    //jak znajdziemy to mamy miejsce całości i możemy wyciągnąć nazwę zmiennej

    
    regcomp(&regex, "<<([^ ]+)>>", REG_EXTENDED);

    char* temp_buffer;
    //dzięki temu możemy od razu tworzyć temp_buffer na bazie bufora i nie martwimy się że buffer będzie pusty jesli nie znajdziemy zmiennej
    strncpy(buffer, unprocessed, buffer_size);
    char* before_field;
    char* after_field;
    char* var;
    char* val;
    int index;
    bool value_found;


    while(regexec(&regex, buffer, 2, matches, 0) == 0) {
        //w temp_buffer zawsze musi być na koniec pętli najnowsza wersja
        temp_buffer = (char*)malloc(buffer_size);
        
        strncpy(temp_buffer, buffer, buffer_size);


        //najpierw wyciągamy nazwę zmiennej
        temp_buffer[matches[1].rm_eo] = '\0';
        var = temp_buffer + matches[1].rm_so;
        
        value_found = false;
        

        for(index = 0; index < dict_len; index++) {
            if(strcasecmp(var, keys[index]) == 0) {
                val = values[index];
                value_found = true;
            }
        }
        if(!value_found) {
            free(temp_buffer);
            regfree(&regex);
            return -1;
        }
        

        before_field = temp_buffer;
        before_field[matches[0].rm_so] = '\0';

        after_field = temp_buffer + matches[0].rm_eo;

        //można chyba wpisać nową wartość w bufor końcowy
        //potem kopiować do temp końcowy i zaczynać od nowa
        snprintf(buffer, buffer_size, "%s%s%s", before_field, val, after_field);

        free(temp_buffer);


    }

    return 0;
}