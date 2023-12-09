#include <sys/types.h>
#include <sys/time.h>
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
#include <ctype.h>
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

/***
 * Logs in a user, or registers a brand new user if it doesn't exist in the database
 * 
 * @param message The UDP request that contains the info necessary for the login.
 * It should have this format: 'LIN UID password'
 * @param to_addr The address where the UDP message should be sent to
 * @param to_addr_len The length of the address where the UDP message is sent
*/
void login_handling(char* message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    char* token;
    char userID[7], userPasswd[9];
    int status;

    strtok(message, " ");    // This only gets the "LIN " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, " ");
    if (strlen(token) != 6) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    for(int i = 0; i < 6; i++) {
        char c = token[i];
        if (!isdigit(c)) {
            if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
            //? Same here
            server_udp_send("ERR\n", to_addr, to_addr_len);
            return;
        }
    }
    strcpy(userID, token);

    // Get the user password and verify if it's an 8-digit number
    token = strtok(NULL, "\n");
    if (strlen(token) != 8) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    for(int i = 0; i < 8; i++) {
        char c = token[i];
        if (!isdigit(c) && !isalpha(c)) {
            if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
            //? Same here
            server_udp_send("ERR\n", to_addr, to_addr_len);
            return;
        }
    }
    strcpy(userPasswd, token);

    //* status = Login(userID, userPasswd);
    //! There is no return value for a new user (I'm using 2 in this case)
    status = 2;    // TODO Delete this line
    if (status == -1) {         //? I suppose -1 is when the credentials are incorrect
        if (is_mode_verbose) 
            printf("Login: Incorrect credentials given - user %s\n", userID);
        //? Same here
        server_udp_send("RLI NOK\n", to_addr, to_addr_len);
        return;
    } else if (status == 0 || status == 1) {
        if (is_mode_verbose) 
            printf("Login: User %s has logged in\n", userID);
        //? Same here
        server_udp_send("RLI OK\n", to_addr, to_addr_len);
        return;
    } else if (status == 2) {
        if (is_mode_verbose) 
            printf("Login: New user with ID %s has been registered.\n", userID);
        //? Same here
        server_udp_send("RLI REG\n", to_addr, to_addr_len);
        return;
    }

    return;
}

/***
 * Receives an UDP message, processes it and sends its corresponding response
 * depending on the type of message.
 * 
 * @return 0 if the request is processed correctly, -1 if there was an error
 * processing the request  
*/
int handle_udp_request() {
    int status;
    char buffer[256], message_type[5];
    struct sockaddr sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    memset(buffer, 0, sizeof buffer);
    status = server_udp_receive(buffer, 256, &sender_addr, &sender_addr_len);
    if (status == -1) {
        if (is_mode_verbose) printf("Failed to receive request.\n");
        return -1;   // Go to the next mesage
    }

    // Get the first 4 characters of the message to determine its type
    strncpy(message_type, buffer, 4);

    //? Each routine should have the following parameters:
    //? - the buffer that has be passed for analysing the message
    //? - the sender_addr to know where the reponse should be sent to
    //? - the sender_addr_len that complements the sender_addr
    //? eg. login_handling(char* message, struct sockaddr* to_addr, socklen_t to_addr_len)
    //? Check the "else" clause for an example of how the responses should be sent
    if(!strcmp(message_type, "LIN ")) {
        login_handling(buffer, &sender_addr, sender_addr_len);
    } else if (!strcmp(message_type, "LOU ")) {
        // TODO Logout handling routine
    } else if (!strcmp(message_type, "UNR ")) {
        // TODO Unregister handling routine
    } else if (!strcmp(message_type, "LMA ")) {
        // TODO List my auctions handling routine
    } else if (!strcmp(message_type, "LMB ")) {
        // TODO List my bids handling routine
    } else if (!strcmp(message_type, "LST\n")) {
        // TODO List all auctions handling routine
    } else if (!strcmp(message_type, "SRC ")) {
        // TODO Auction record handling routine
    } else {
        if(is_mode_verbose)
            printf("Invalid UDP request made to server.\n");
        
        // The message is invalid in this case.
        // Send response ERR (invalid request).
        status = server_udp_send("ERR\n", &sender_addr, sender_addr_len);
        if (status == -1) return -1;
    }

    return 0;
}

int handle_tcp_request() {
    int socket_fd, status;
    char buffer[8];
    struct timeval timeout;

    // Accept the new connection to the new socket
    socket_fd = server_tcp_accept();
    if (socket_fd == -1) {
        if (is_mode_verbose)
            printf("Failed to accept TCP connection.\n");
        return -1;
    }

    // Set a timeout in case the socket is stuck on read/write
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
            sizeof timeout) < 0 ||
            setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout,
            sizeof timeout) < 0) {
        if (is_mode_verbose)
            printf("Failed to establish TCP socket timeout.\n");
        server_tcp_close(socket_fd);
        return -1;
    }

    // Receive the first 4 characters of a TCP message and compare it
    // against the list of actions available
    memset(buffer, 0, sizeof buffer);
    status = server_tcp_receive(socket_fd, buffer, 4);
    if (status == -1) {
        if (is_mode_verbose)
            printf("Failed to receive TCP message. Closing connection.\n");
        server_tcp_close(socket_fd);
        return -1;
    }

    //? Each routine should have the following parameters:
    //? - the socket_fd that represents the socket connection 
    //? eg. login_handling(int socket_fd)
    //? To read the parameters of the message, use server_tcp_receive with
    //? the socket_fd passed into the routine.
    //? Check the "else" clause for an example of how the responses should be sent
    if(!strcmp(buffer, "OPA ")) {
        // TODO Open auction handling routine
    } else if(!strcmp(buffer, "CLS ")) {
        // TODO Close auction handling routine
    } else if(!strcmp(buffer, "SAS ")) {
        // TODO Show asset handling routine
    } else if(!strcmp(buffer, "BID ")) {
        // TODO Bid handling routine
    } else {
        // Send an error response using TCP
        status = server_tcp_send(socket_fd, "ERR\n", 4);
        if (status == -1 && is_mode_verbose) {
            printf("Failed to send TCP response. Closing connection.\n");
            server_tcp_close(socket_fd);
            return -1;
        }
    }

    server_tcp_close(socket_fd);

    return 0;
}


int main(int argc, char *argv[]) {
    int tcp_fd, udp_fd, out_fds;
    fd_set inputs, current_fds;
    struct timeval timeout;

    // Define verbose mode and port number according to the arguments used
    // to run this program
    set_program_parameters(argc, argv);

    // Set up both UDP and TCP connections
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

    FD_ZERO(&inputs);           // Clear input mask
    FD_SET(udp_fd, &inputs);    // Set UDP channel on for listening to activity
    FD_SET(tcp_fd, &inputs);    // Set TCP channel on for listening to activity

    while(1) {
        current_fds = inputs;   // Reload mask

        memset((void *) &timeout, 0, sizeof(timeout));
        timeout.tv_sec = 5;     // Set a timeout for every select function

        // Wait on select until a timeout has occurred
        out_fds = select(FD_SETSIZE, &current_fds, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) &timeout);

        switch(out_fds) {
            case 0:
                // Do nothing, no message was received in the last couple seconds.
                break;
            case -1:
                printf("Fatal error on select.\n");
                exit(EXIT_FAILURE);
            default:
                // If a message is received via UDP
                if(FD_ISSET(udp_fd, &current_fds)) {
                    handle_udp_request();
                }
                // If a message is received via TCP
                if(FD_ISSET(tcp_fd, &current_fds)) {
                    handle_tcp_request();
                }
        }
    }

    UDP_free();
    TCP_free();
}