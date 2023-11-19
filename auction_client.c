#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SERVER_IP "localhost"
#define DEFAULT_SERVER_PORT 58057   // 58000 + Group #57

/***
 * Sets the server ip and port parameters with the arguments given
 * when the program is executed.
 * 
 * @param argc Number of arguments given when the program
 * @param argv List of arguments given when the program is executed
 * @param ip The variable where the server IP is written to
 * @param port The variable where the server port is written to
 * 
 * @note The function forces program exiting if the arguments are invalid
*/
void setServerParameters(int argc, char *argv[], char ip[], int *port) {
    int ip_used = 0, port_used = 0; // These variables control if a param is used twice

    strcpy(ip, DEFAULT_SERVER_IP);
    *port = DEFAULT_SERVER_PORT;

    // Check if there is a correct number of arguments
    if (argc % 2 == 0 || argc > 5) {
        printf("Wrong arguments given.\n%s [-n ASIP] [-p ASport]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for(int i = 1; i < argc; i += 2) {      // Check for the IP argument
        if(!strcmp(argv[i], "-n")) {
            if(ip_used == 1) {
                printf("Wrong arguments given.\n%s [-n ASIP] [-p ASport]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            strcpy(ip, argv[i + 1]);
            ip_used = 1;
        } else if (!strcmp(argv[i], "-p")) { // Check for a port argument
            *port = atoi(argv[i + 1]);

            // If the port is invalid, throw an error
            if(port_used == 1 || *port <= 0 || *port > 65535) {
                printf("Wrong arguments given.\n%s [-n ASIP] [-p ASport]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            port_used = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    char server_ip[256];
    int server_port;

    // Set the parameters of the server according to the program's arguments
    setServerParameters(argc, argv, server_ip, &server_port);    
    printf("%s\n", server_ip);
    printf("%d\n", server_port);
}