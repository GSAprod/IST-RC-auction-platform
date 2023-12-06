#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DEFAULT_PORT "58057"

char port[8];
int is_mode_verbose = 0;

void set_program_parameters(int argc, char* argv[]) {
    int port_int;

    strcpy(port, DEFAULT_PORT);

    switch(argc) {
        // No additional arguments given
        case 1:
            break;
        case 2:
            // If there are only two args, then the second one MUST be the verbose
            // mode
            if(!strcmp(argv[1], "-v")) {
                is_mode_verbose = 1;
                break;
            } else {
                printf("Wrong arguments given.\n\t%s [-p ASport] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            
        case 3:
            // If there are only three args, the second one MUST be the port argument,
            // followed by the desired port number
            port_int = atoi(argv[2]);
            if(strcmp(argv[1], "-p") || port_int <= 0 || port_int > 65535) {
                printf("Wrong arguments given.\n\t%s [-p ASport] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            strcpy(port, argv[2]);
            break;
        case 4:
            // If there are 4 arguments, we must check for these two circumstances:
            // ./AS -v -p ASport
            // ./AS -p ASport -v
            if(!strcmp(argv[1], "-v") && !strcmp(argv[2], "-p") &&
               atoi(argv[3]) > 0 && atoi(argv[3]) <= 65535) {
                is_mode_verbose = 1;
                strcpy(port, argv[3]);
            } else if (!strcmp(argv[3], "-v") && !strcmp(argv[1], "-p") &&
               atoi(argv[2]) > 0 && atoi(argv[2]) <= 65535) {
                is_mode_verbose = 1;
                strcpy(port, argv[2]);
            } else {
                // We could just use the print from the default case here,
                // but due to compiler warnings (more specifically -Wimplicit-fallthrough),
                // we decided to add this else clause.
                printf("Wrong arguments given.\n\t%s [-p ASport] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            printf("Wrong arguments given.\n\t%s [-p ASport] [-v]\n", argv[0]);
            exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    set_program_parameters(argc, argv);

    printf("finish\n");
}