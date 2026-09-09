#include "http_server_header.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>

int verify_int(char* int_str);

/**
 * @brief Function that handles arguments givent to main()
 * 
 * Function iterates over arguments givent to main(), then sets values to corresponding variables
 * Flags:
 * -h Prints help message
 * -p {port number} set port number
 * -v turn on verbose init mode
 * -sidLn {length} set session id length
 * -sidLfs {lifespan} set session id lifespan in minutes
 * -NAS {NAS ip}:{NAS port} set NAS server ip and port
 * 
 * @param argc number of arguments given to main
 * @param argv address of the list of arguments given to main
 * @param args address of a struct consisting of variables to be set
 * 
 * @return 0 if execution was successful some negative int if error occured.
 */
int handle_arguments(int argc, char **argv, struct input_args_struct *args) {


    //jeśli liczba argumentów < 2 to podany jest tylko main i wychodzimy stąd
    if(argc == 1) {
        return 0;
    }


    for(int i = 1; i < argc; i++) {
        if(strcasecmp(argv[i], "-h") == 0) {
            args->print_help = true;
            return 0;
        }
        else if(strcasecmp(argv[i], "-v") == 0) {
            args->verbose_init = true;
        }
        else if(strcasecmp(argv[i], "-p") == 0) {
            
            if(i + 1 >= argc) {
                args->print_help = true;
                fprintf(stderr, "Value expected after flag \"-p\".\n");
                return -1;
            }

            if(verify_int(argv[i + 1]) != 0) {
                args->print_help = true;
                fprintf(stderr, "Value for flag \"-p\" expected to be int type.\n");
                return -2;
            }

            args->selected_port = atoi(argv[i + 1]);
            i++;
        }
        else if(strcasecmp(argv[i], "-sidLn") == 0) {
            
            if(i + 1 >= argc) {
                args->print_help = true;
                fprintf(stderr, "Value expected after flag \"-sidLn\".\n");
                return -1;
            }

            if(verify_int(argv[i + 1]) != 0) {
                args->print_help = true;
                fprintf(stderr, "Value for flag \"-sidLn\" expected to be int type.\n");
                return -2;
            }

            args->session_id_length = atoi(argv[i + 1]);
            i++;

        }
        else if(strcasecmp(argv[i], "-sidLfs") == 0) {

            if(i + 1 >= argc) {
                args->print_help = true;
                fprintf(stderr, "Value expected after flag \"-sidLfs\".\n");
                return -1;
            }

            if(verify_int(argv[i + 1]) != 0) {
                args->print_help = true;
                fprintf(stderr, "Value for flag \"-sidLfs\" expected to be int type.\n");
                return -2;
            }

            args->session_id_lifespan = atoi(argv[i + 1]);
            i++;
        }
        else if(strcasecmp(argv[i], "-NAS") == 0) {

            if(i + 1 >= argc) {
                args->print_help = true;
                fprintf(stderr, "Value expected after flag \"-NAS\".\n");
                return -1;
            }

            //trzeba przerobić ip:port na osobno ip i port
            char ip[INET_ADDRSTRLEN];

            char* div = strchr(argv[i + 1], ':');

            if(div == NULL) {
                args->print_help = true;
                fprintf(stderr, "Value for flag \"-NAS\" expected to be formatted as {ip}:{port}.\n");
                return -3;
            }

            size_t ip_len = div - argv[i + 1];
            if(ip_len >= sizeof(ip)) {
                args->print_help = true;
                fprintf(stderr, "Ip for NAS malformed.\n");
                return -4; //ip jest zbyt długie
            }

            memcpy(ip, argv[i + 1], ip_len);
            ip[ip_len] = '\0';

            char* port_str = div + 1;

            if(verify_int(port_str) != 0) {
                args->print_help = true;
                fprintf(stderr, "Value \'%s\' describing port for \"-NAS\" {ip}:{port} flag expected to be of type int.\n", port_str);
                return -5;
            }

            int port = atoi(port_str);

            inet_pton(AF_INET, ip, &(args->NAS_add.sin_addr));
            args->NAS_add.sin_port = htons(port);
            i++;
        }

        else{
            fprintf(stderr, "Unknown argument or flag \"%s\".\n", argv[i]);
            return -1;
        }

    }

    return 0;


}


/**
 * @brief Simple function fo checking if str value can be represented by an int
 * 
 * @param int_str char pointer supposedly representing string
 * 
 * @return 0 if str can be represented by int, -1 if not.
 * 
 */
int verify_int(char* int_str) {
    int i = 0;
    size_t len = strlen(int_str);

    while(i < len) {
        if(!isdigit(*(int_str + i))) {
            return -1;
        }
        ++i;
    }
    return 0;
}