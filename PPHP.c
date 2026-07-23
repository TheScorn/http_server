#include <regex.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
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

