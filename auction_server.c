#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "client_connections.h"
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
    int tcp_fd, udp_fd, out_fds;
    fd_set inputs, current_fds;
    struct timeval timeout;

    set_program_parameters(argc, argv);

    udp_fd = server_setup_UDP(port);
    if (udp_fd == -1) {
        printf("Failure setting up server.\n");
        exit(EXIT_FAILURE);
    }

    tcp_fd = server_setup_TCP(port);
    if (tcp_fd == -1) {
        printf("Failure setting up server.\n");
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&inputs);   // Clear input mask
    FD_SET(udp_fd, &inputs);     // Set UDP channel on for listening to activity

    while(1) {
        current_fds = inputs;   // Reload mask

        memset((void *) &timeout, 0, sizeof(timeout));
        timeout.tv_sec = 5;     // Set a timeout for every select function

        out_fds = select(FD_SETSIZE, &current_fds, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) &timeout);

        switch(out_fds) {
            case 0:
                printf("Timeout.\n");
                break;
            case -1:
                printf("Error on select.\n");
                exit(EXIT_FAILURE);
            default:
                if(FD_ISSET(udp_fd, &current_fds)) {
                    int status;
                    char buffer[256];
                    struct sockaddr sender_addr;
                    socklen_t sender_addr_len = sizeof(sender_addr);

                    memset(buffer, 0, sizeof buffer);
                    status = server_udp_receive(buffer, 256, &sender_addr, &sender_addr_len);
                    if (status == -1) exit(EXIT_FAILURE);
                    write(1, buffer, strlen(buffer));

                    status = server_udp_send("RLI OK\n", &sender_addr, sender_addr_len);
                    if (status == -1) {perror("jfdghfdgh"); exit(EXIT_FAILURE);}

                    //! Change this so the server can handle messages
                    break;
                }
                //? Add for TCP
        }
    }

    UDP_free();
    TCP_free();
}