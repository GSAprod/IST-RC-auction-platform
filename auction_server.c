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
#include "database_handling.h"
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
 * Checks if the string given corresponds to a 6-digit user ID.
 * 
 * @param string The string to check
 * @return 1 if the string given has the format of a user ID, 0 otherwise
*/
int verify_format_id(char* string) {
    if (strlen(string) != 6)
        return 0;

    for(int i = 0; i < 6; i++) {
        char c = string[i];
        if (!isdigit(c)) return 0;
    }

    return 1;
}

/***
 * Checks if the string given corresponds to an 8-digit alphanumeric password.
 * 
 * @param string The string to check
 * @return 1 if the string given has the format of a password, 0 otherwise
*/
int verify_format_password(char* string) {
    if (strlen(string) != 8)
        return 0;

    for(int i = 0; i < 8; i++) {
        char c = string[i];
        if (!isdigit(c) && !isalpha(c)) return 0;
    }

    return 1;
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
    if (!verify_format_id(token)) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userID, token);

    // Get the user password and verify if it's an 8-digit number
    token = strtok(NULL, "\n");
    if (!verify_format_password(token)) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userPasswd, token);

    status = Login(userID, userPasswd);
    if (status == -2) {
        // In this case, there would be an error when a login attempt is made.
        if (is_mode_verbose) 
            printf("Login: A error occurred when user %s was logging in.\n", userID);
        //? Same here
        server_udp_send("ERR\n", to_addr, to_addr_len);

    } else if (status == -1) {
        // In this case, the user and password don't match
        if (is_mode_verbose) 
            printf("Login: Incorrect credentials given - user %s\n", userID);
        //? Same here
        server_udp_send("RLI NOK\n", to_addr, to_addr_len);

    } else if (status == 0) {
        // In this case, a new user is created in the database
        if (is_mode_verbose) 
            printf("Login: New user with ID %s has been registered.\n", userID);
        //? Same here
        server_udp_send("RLI REG\n", to_addr, to_addr_len);

    } else if (status == 1) {
        // In this case, the login is successful
        if (is_mode_verbose) 
            printf("Login: User %s has logged in\n", userID);
        //? Same here
        server_udp_send("RLI OK\n", to_addr, to_addr_len);
    } 

    return;
}

/***
 * Logs out a user, if it exists in the database
 * 
 * @param message The UDP request that contains the info necessary for the logout.
 * It should have this format: 'LOU UID password'
 * @param to_addr The address where the UDP message should be sent to
 * @param to_addr_len The length of the address where the UDP message is sent
*/
void logout_handling(char* message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    char* token;
    char userID[7], userPasswd[9];
    int status;

    strtok(message, " ");    // This only gets the "LOU " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, " ");
    if (!verify_format_id(token)) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userID, token);

    // Get the user password and verify if it's an 8-digit number
    token = strtok(NULL, "\n");
    if (!verify_format_password(token)) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userPasswd, token);

    // Check if the user is logged in before logging out
    status = CheckUserLogged(userID, userPasswd);
    switch (status) {
        case -2:
            if (is_mode_verbose) 
                printf("Logout: Invalid password or password error for user %s\n", userID);
            //? Same here
            server_udp_send("ERR\n", to_addr, to_addr_len);
            break;
        case -1:
            if (is_mode_verbose) 
                printf("Logout: User %s not registered in database\n", userID);
            //? Same here
            server_udp_send("RLO UNR\n", to_addr, to_addr_len);
            break;
        case 0:
            if (is_mode_verbose) 
                printf("Logout: User %s is not logged in\n", userID);
            //? Same here
            server_udp_send("RLO NOK\n", to_addr, to_addr_len);
            break;
        case 1:
            status = Logout(userID);
            if (status != 1) {
                if (is_mode_verbose) 
                    printf("Logout: User %s is not logged in\n", userID);
                //? Same here
                server_udp_send("RLO NOK\n", to_addr, to_addr_len);
                break;
            } else {
                if (is_mode_verbose) 
                    printf("Logout: User %s has logged out\n", userID);
                //? Same here
                server_udp_send("RLO OK\n", to_addr, to_addr_len);
                break;
            }
    }
    
    return;
}

/***
 * Unregisters a user, if it exists in the database and is logged in
 * 
 * @param message The UDP request that contains the info necessary for unregistering.
 * It should have this format: 'UNR UID password'
 * @param to_addr The address where the UDP message should be sent to
 * @param to_addr_len The length of the address where the UDP message is sent
*/
void unregister_handling(char* message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    char* token;
    char userID[7], userPasswd[9];
    int status;

    strtok(message, " ");    // This only gets the "UNR " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, " ");
    if (!verify_format_id(token)) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userID, token);

    // Get the user password and verify if it's an 8-digit number
    token = strtok(NULL, "\n");
    if (!verify_format_password(token)) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userPasswd, token);

    // Check if the user is logged in before logging out
    status = CheckUserLogged(userID, userPasswd);
    switch (status) {
        case -2:
            if (is_mode_verbose) 
                printf("Unregister: Invalid password or password error for user %s\n", userID);
            //? Same here
            server_udp_send("ERR\n", to_addr, to_addr_len);
            break;
        case -1:
            if (is_mode_verbose) 
                printf("Unregister: User %s not registered in database\n", userID);
            //? Same here
            server_udp_send("RUR UNR\n", to_addr, to_addr_len);
            break;
        case 0:
            if (is_mode_verbose) 
                printf("Unregister: User %s is not logged in\n", userID);
            //? Same here
            server_udp_send("RUR NOK\n", to_addr, to_addr_len);
            break;
        case 1:
            status = Unregister(userID);
            if (status != 0) {
                if (is_mode_verbose) 
                    printf("Unregister: User %s is not logged in\n", userID);
                //? Same here
                server_udp_send("RUR NOK\n", to_addr, to_addr_len);
                break;
            } else {
                if (is_mode_verbose) 
                    printf("Unregister: Unregistered user %s\n", userID);
                //? Same here
                server_udp_send("RUR OK\n", to_addr, to_addr_len);
                break;
            }
    }
    return;
}

/***
 * Sends back a list of auctions that a certain user has created
 * 
 * @param message The UDP request that contains the info necessary for the login.
 * It should have this format: 'LMA UID'
 * @param to_addr The address where the UDP message should be sent to
 * @param to_addr_len The length of the address where the UDP message is sent
*/
void list_myauctions_handling(char* message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    char* token, *ptr;
    char userID[7], response[8192];
    int num_auctions;

    strtok(message, " ");    // This only gets the "UNR " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, " ");
    if (!verify_format_id(token)) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userID, token);

    struct AUCTIONLIST * auction_list;
    num_auctions = GetAuctionsListByUserBidded(userID, auction_list);
    if (num_auctions == -1) {
        if (is_mode_verbose) printf("List my auctions: User %s is not logged in.\n", userID);
        server_udp_send("RMA NLG\n", to_addr, to_addr_len);
        return;
    }
    if (num_auctions == 0) {
        if (is_mode_verbose) printf("List my auctions: User %s has no ongoing auctions.\n", userID);
        //? Same here
        server_udp_send("RMA NOK\n", to_addr, to_addr_len);
        free(auction_list);
        return;
    }

    strcpy(response, "RMA OK");
    ptr = response + 6;
    char aux[6];
    for(int i = 0; i < num_auctions; i++) {
        // TODO Fazer string
        sprintf(aux, " %s %d", auction_list[i].AID, auction_list[i].active);
        strcpy(ptr, aux);
        ptr += strlen(aux);
    }
    strcpy(ptr, "\n");

    free(auction_list);

    // TODO Enviar string
    server_udp_send(response, to_addr, to_addr_len);
}

void list_mybids_handling(char* message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    char* token, *ptr;
    char userID[7], response[8192];
    int num_auctions;
    struct AUCTIONLIST * bid_list;

    strtok(message, " ");    // This only gets the "UNR " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, " ");
    if (!verify_format_id(token)) {
        if (is_mode_verbose) printf("Invalid UDP request made to server.\n");
        //? Should we check if the status is -1? If so, what should we do?
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userID, token);

    // Check if the user is logged into the database

    num_auctions = GetAuctionsListByUserBidded(userID, bid_list);
    if (num_auctions == -1) {
        if (is_mode_verbose) printf("List my bids: User %s is not logged in.\n", userID);
        //? Same here
        server_udp_send("RMB NLG\n", to_addr, to_addr_len);
        return;
    }
    if (num_auctions == 0) {
        if (is_mode_verbose) printf("List my bids: User %s has no ongoing bids.\n", userID);
        //? Same here
        server_udp_send("RMB NOK\n", to_addr, to_addr_len);
        free(bid_list);
        return;
    }

    strcpy(response, "RMB OK");
    ptr = response + 6;
    char aux[6];
    for(int i = 0; i < num_auctions; i++) {
        // TODO Fazer string
        sprintf(aux, " %s %d", bid_list[i].AID, bid_list[i].active);
        strcpy(ptr, aux);
        ptr += strlen(aux);
    }
    strcpy(ptr, "\n");

    free(bid_list);

    // TODO Enviar string
    server_udp_send(response, to_addr, to_addr_len);
}

void list_auctions_handling(char * message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    char *ptr;
    char response[8192];
    int num_auctions;
    struct AUCTIONLIST * auction_list;

    strtok(message, " ");    // This only gets the "UNR " string

    num_auctions = GetAuctionsList(auction_list);
    if (num_auctions == 0) {
        if (is_mode_verbose) printf("List auctions: There are no auctions.\n");
        //? Same here
        server_udp_send("RLS NOK\n", to_addr, to_addr_len);
        free(auction_list);
        return;
    }

    strcpy(response, "RLS OK");
    ptr = response + 6;
    char aux[6];
    for(int i = 0; i < num_auctions; i++) {
        // TODO Fazer string
        sprintf(aux, " %s %d", auction_list[i].AID, auction_list[i].active);
        strcpy(ptr, aux);
        ptr += strlen(aux);
    }
    strcpy(ptr, "\n");

    free(auction_list);

    // TODO Enviar string
    server_udp_send(response, to_addr, to_addr_len);

}  

void close_auction_handling(int socket_fd) {
    char buffer[32];

    // Receive the remaining bytes of the message
    memset(buffer, 0, sizeof buffer);
    if (server_tcp_receive(socket_fd, buffer, 32) == -1) {
        if (is_mode_verbose)
            printf("Failed to receive TCP message. Closing connection.\n");
        server_tcp_close(socket_fd);
        return;
    }
    
    // Check if the message is valid
    char UID[6], AID[3], password[8];
    sscanf(buffer, "%s %s %s\n", UID, AID, password);

    if (!verify_format_id(UID) || !verify_format_id(AID) || !verify_format_password(password)) {
        if (is_mode_verbose)
            printf("Invalid TCP request made to server.\n");
        server_tcp_send(socket_fd, "ERR\n", 4);
        server_tcp_close(socket_fd);
        return;
    }

    // Check if the user is logged in
    int status = CheckUserLogged(UID, password);
    if (status <= 0) {
        if (is_mode_verbose)
            printf("Close auction: User %s is not logged in.\n", UID);
        server_tcp_send(socket_fd, "RCL NLG\n", 8);
        server_tcp_close(socket_fd);
        return;
    }

    /*
    status = CloseAuction(AID, UID);
    switch (status) {
        case 0:
            if (is_mode_verbose)
                printf("Close auction: Auction %s has been closed.\n", AID);
            server_tcp_send(socket_fd, "RCL OK\n", 7);
            server_tcp_close(socket_fd);
            return;
        case -1:
            if (is_mode_verbose)
                printf("Close auction: Auction does not exist - %s.\n", AID);
            server_tcp_send(socket_fd, "RCL EAU\n", 8);
            server_tcp_close(socket_fd);
            return;
        case -2:
            if (is_mode_verbose)
                printf("Close auction: User %s is not the auction owner.\n", UID);
            server_tcp_send(socket_fd, "RCL EOW\n", 8);
            server_tcp_close(socket_fd);
            return;
        case -3:
            if (is_mode_verbose)
                printf("Close auction: Auction %s has already ended.\n", AID);
            server_tcp_send(socket_fd, "RCL END\n", 8);
            server_tcp_close(socket_fd);
            return;
    }
    */



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
        logout_handling(buffer, &sender_addr, sender_addr_len);
    } else if (!strcmp(message_type, "UNR ")) {
        unregister_handling(buffer, &sender_addr, sender_addr_len);
    } else if (!strcmp(message_type, "LMA ")) {
        list_myauctions_handling(buffer, &sender_addr, sender_addr_len);
    } else if (!strcmp(message_type, "LMB ")) {
        list_mybids_handling(buffer, &sender_addr, sender_addr_len);
    } else if (!strcmp(message_type, "LST\n")) {
        list_auctions_handling(buffer, &sender_addr, sender_addr_len);
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
        close_auction_handling(socket_fd);
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
    InitDatabase();

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