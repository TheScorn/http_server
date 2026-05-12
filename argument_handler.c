#include "http_server_header.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief Function that handles arguments givent to main()
 * 
 * Function iterates over arguments givent to main(), then sets values to corresponding variables
 * Flags:
 * -h Prints help message
 * -p {port number} set port number
 * -v turn on verbose init mode
 * 
 * 
 * @param argc number of arguments given to main
 * @param argv address of the list of arguments given to main
 * @param args address of a struct consisting of variables to be set
 * 
 * @return 0 if execution was successful or -1 if error occured.
 */
int handle_arguments(int argc, char **argv, struct input_args_struct *args) {

    int ret = 0;


    //jeśli liczba argumentów < 2 to podany jest tylko main i wychodzimy stąd
    if(argc == 1) {
        return ret;
    }

    
    if(argc == 2){
        if(strcmp(argv[1], "-h") == 0) {
            args->print_help = true;
        }

        else if(strcmp(argv[1], "-v") == 0) {
            args->verbose_init = true;
        }

        else {
            ret = -1;
        }

    }

    if(argc == 3) {
        if(strcmp(argv[1], "-p") == 0) {
            args->selected_port = atoi(argv[2]);
        }
        else {
            ret = -2;
        }

    }


    if(argc == 4) {
        if(strcmp(argv[1], "-v") == 0 && strcmp(argv[2], "-p") == 0) {
            args->verbose_init = true;
            args->selected_port = atoi(argv[3]);
        }

        else if(strcmp(argv[3], "-v") == 0 && strcmp(argv[1], "-p") == 0) {
            args->verbose_init = true;
            args->selected_port = atoi(argv[2]);
        }

        else {
            ret = -3;
        }

    }

    if(argc > 4) {
        ret = -4;
    }


    return ret;
}