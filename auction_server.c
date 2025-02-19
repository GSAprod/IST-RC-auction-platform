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

char port[PORT_SIZE];

void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nSIGINT received. Shutting down server.\n");
        UDP_free();
        TCP_free();
        exit(EXIT_SUCCESS);
    }
}

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
                set_mode_verbose();
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
                set_mode_verbose();
                strcpy(port, argv[3]);
            } else if (!strcmp(argv[3], "-v") && !strcmp(argv[1], "-p") &&
               atoi(argv[2]) > 0 && atoi(argv[2]) <= 65535) {
                set_mode_verbose();
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
 * Checks if the string given corresponds to a 3-digit auction ID.
 * 
 * @param string The string to check
 * @return 1 if the string given has the format of an auction ID, 0 otherwise
*/
int verify_format_AID(char* string) {
    if (strlen(string) != 3)
        return 0;

    for(int i = 0; i < 3; i++) {
        char c = string[i];
        if (!isdigit(c)) return 0;
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
    char userID[UID_SIZE], userPasswd[PASSWORD_SIZE];
    int status;

    strtok(message, " ");    // This only gets the "LIN " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, " ");
    if (!verify_format_id(token)) {
        if (get_mode_verbose()) printf("Invalid UDP request made to server.\n");
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userID, token);

    // Get the user password and verify if it's an 8-digit number
    token = strtok(NULL, "\n");
    if (!verify_format_password(token)) {
        if (get_mode_verbose()) printf("Invalid UDP request made to server.\n");
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userPasswd, token);

    status = Login(userID, userPasswd);
    if (status == -2) {
        // In this case, there would be an error when a login attempt is made.
        if (get_mode_verbose()) 
            printf("Login: A error occurred when user %s was logging in.\n", userID);
        server_udp_send("ERR\n", to_addr, to_addr_len);

    } else if (status == -1) {
        // In this case, the user and password don't match
        if (get_mode_verbose()) 
            printf("Login: Incorrect credentials given - user %s\n", userID);
        server_udp_send("RLI NOK\n", to_addr, to_addr_len);

    } else if (status == 0) {
        // In this case, a new user is created in the database
        if (get_mode_verbose()) 
            printf("Login: New user with ID %s has been registered.\n", userID);
        server_udp_send("RLI REG\n", to_addr, to_addr_len);

    } else if (status == 1) {
        // In this case, the login is successful
        if (get_mode_verbose()) 
            printf("Login: User %s has logged in\n", userID);
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
    char userID[UID_SIZE], userPasswd[PASSWORD_SIZE];
    int status;

    strtok(message, " ");    // This only gets the "LOU " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, " ");
    if (!verify_format_id(token)) {
        if (get_mode_verbose()) printf("Invalid UDP request made to server.\n");
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userID, token);

    // Get the user password and verify if it's an 8-digit number
    token = strtok(NULL, "\n");
    if (!verify_format_password(token)) {
        if (get_mode_verbose()) printf("Invalid UDP request made to server.\n");
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userPasswd, token);

    // Check if the user is logged in before logging out
    status = CheckUserLogged(userID, userPasswd);
    switch (status) {
        case -2:
            if (get_mode_verbose()) 
                printf("Logout: Invalid password or password error for user %s\n", userID);
            server_udp_send("ERR\n", to_addr, to_addr_len);
            break;
        case -1:
            if (get_mode_verbose()) 
                printf("Logout: User %s not registered in database\n", userID);
            server_udp_send("RLO UNR\n", to_addr, to_addr_len);
            break;
        case 0:
            if (get_mode_verbose()) 
                printf("Logout: User %s is not logged in\n", userID);
            server_udp_send("RLO NOK\n", to_addr, to_addr_len);
            break;
        case 1:
            status = Logout(userID);
            if (status != 1) {
                if (get_mode_verbose()) 
                    printf("Logout: User %s is not logged in\n", userID);
                server_udp_send("RLO NOK\n", to_addr, to_addr_len);
                break;
            } else {
                if (get_mode_verbose()) 
                    printf("Logout: User %s has logged out\n", userID);
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
    char userID[UID_SIZE], userPasswd[PASSWORD_SIZE];
    int status;

    strtok(message, " ");    // This only gets the "UNR " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, " ");
    if (!verify_format_id(token)) {
        if (get_mode_verbose()) printf("Invalid UDP request made to server.\n");
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userID, token);

    // Get the user password and verify if it's an 8-digit number
    token = strtok(NULL, "\n");
    if (!verify_format_password(token)) {
        if (get_mode_verbose()) printf("Invalid UDP request made to server.\n");
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userPasswd, token);

    // Check if the user is logged in before logging out
    status = CheckUserLogged(userID, userPasswd);
    switch (status) {
        case -2:
            if (get_mode_verbose()) 
                printf("Unregister: Invalid password or password error for user %s\n", userID);
            server_udp_send("ERR\n", to_addr, to_addr_len);
            break;
        case -1:
            if (get_mode_verbose()) 
                printf("Unregister: User %s not registered in database\n", userID);
            server_udp_send("RUR UNR\n", to_addr, to_addr_len);
            break;
        case 0:
            if (get_mode_verbose()) 
                printf("Unregister: User %s is not logged in\n", userID);
            server_udp_send("RUR NOK\n", to_addr, to_addr_len);
            break;
        case 1:
            status = Unregister(userID);
            if (status != 0) {
                if (get_mode_verbose()) 
                    printf("Unregister: User %s is not logged in\n", userID);
                server_udp_send("RUR NOK\n", to_addr, to_addr_len);
                break;
            } else {
                if (get_mode_verbose()) 
                    printf("Unregister: Unregistered user %s\n", userID);
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
    char userID[UID_SIZE], response[LIST_BUFFER];
    int num_auctions;

    strtok(message, " ");    // This only gets the "LMA " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, "\n");
    if (!verify_format_id(token)) {
        if (get_mode_verbose()) printf("Invalid UDP request made to server.\n");
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }
    strcpy(userID, token);

    // Gets the number of auctions and saves the auctions in the auction_list
    struct AUCTIONLIST * auction_list = NULL;
    num_auctions = GetAuctionsListByUser(userID, &auction_list);
    if (num_auctions == -1) {
        if (get_mode_verbose()) printf("List my auctions: User %s is not logged in.\n", userID);
        server_udp_send("RMA NLG\n", to_addr, to_addr_len);
        return;
    }
    if (num_auctions == 0) {
        if (get_mode_verbose()) printf("List my auctions: User %s has no ongoing auctions.\n", userID);
        server_udp_send("RMA NOK\n", to_addr, to_addr_len);
        free(auction_list);
        return;
    }

    strcpy(response, "RMA OK");
    ptr = response + 6;
    char aux[SMALL_BUFFER];
    for(int i = 0; i < num_auctions; i++) {
        // Adds the auction info to the response
        sprintf(aux, " %s %d", auction_list[i].AID, auction_list[i].active);
        strcpy(ptr, aux);
        ptr += strlen(aux);
        memset(aux, 0, sizeof aux);
    }
    strcpy(ptr, "\n");

    free(auction_list);

    if (get_mode_verbose()) 
        printf("My auctions: User %s requested created auctions (%d created).\n", userID, num_auctions);

    server_udp_send(response, to_addr, to_addr_len);
}

/***
 * Sends back a list of auctions that a certain user has bidded
 * 
 * @param message The UDP request that contains the info necessary for the login.
 * It should have this format: 'LMB UID'
 * @param to_addr The address where the UDP message should be sent to
 * @param to_addr_len The length of the address where the UDP message is sent
*/
void list_mybids_handling(char* message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    char* token, *ptr;
    char userID[UID_SIZE], response[LIST_BUFFER];
    int num_auctions;
    struct AUCTIONLIST * bid_list = NULL;

    strtok(message, " ");    // This only gets the "LMB " string

    // Get the user ID and verify if it's a 6-digit number
    token = strtok(NULL, "\n");
    if (!verify_format_id(token)) {
        if (get_mode_verbose()) printf("Invalid UDP request made to server.\n");
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }

    strcpy(userID, token);

    // Gets the number of auctions by user and saves the auctions in the auction_list
    num_auctions = GetAuctionsListByUserBidded(userID, &bid_list);
    if (num_auctions == -1) {
        if (get_mode_verbose()) printf("List my bids: User %s is not logged in.\n", userID);
        server_udp_send("RMB NLG\n", to_addr, to_addr_len);
        return;
    }
    if (num_auctions == 0) {
        if (get_mode_verbose()) printf("List my bids: User %s has no BIDS.\n", userID);
        server_udp_send("RMB NOK\n", to_addr, to_addr_len);
        return;
    }

    strcpy(response, "RMB OK");
    ptr = response + 6;
    char aux[SMALL_BUFFER];
    for(int i = 0; i < num_auctions; i++) {
        // Adds the auction info to the response
        sprintf(aux, " %s %d", bid_list[i].AID, bid_list[i].active);
        strcpy(ptr, aux);
        ptr += strlen(aux);
    }
    free(bid_list);
    strcpy(ptr, "\n");

    if (get_mode_verbose()) 
        printf("My bids: User %s requested bidded auctions (%d created).\n", userID, num_auctions);

    server_udp_send(response, to_addr, to_addr_len);
}

/***
 * Sends back a list of all auctions
 * 
 * @param message The UDP request that contains the info necessary for the login.
 * It should have this format: 'LST'
 * @param to_addr The address where the UDP message should be sent to
 * @param to_addr_len The length of the address where the UDP message is sent
*/
void list_auctions_handling(char * message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    char *ptr;
    char response[LIST_BUFFER];
    int num_auctions;
    struct AUCTIONLIST * auction_list = NULL;

    strtok(message, " ");    // This only gets the "UNR " string

    // Gets the number of auctions and saves the auctions in the auction_list
    num_auctions = GetAuctionsList(&auction_list);
    if (get_mode_verbose()) printf("num_auctions: %d\n", num_auctions);
    if (num_auctions == -1) {
        if (get_mode_verbose()) printf("List auctions: User is not logged in.\n");
        server_udp_send("RLS NLG\n", to_addr, to_addr_len);
        return;
    }
    if (num_auctions == 0) {
        if (get_mode_verbose()) printf("List auctions: There are no ongoing auctions.\n");
        server_udp_send("RLS NOK\n", to_addr, to_addr_len);
        return;
    }

    strcpy(response, "RLS OK");
    ptr = response + 6;
    char aux[SMALL_BUFFER];
    // Writes the auctions to the response
    for(int i = 0; i < num_auctions; i++) {
        sprintf(aux, " %s %d", auction_list[i].AID, auction_list[i].active);
        strcpy(ptr, aux);
        ptr += strlen(aux);
    }
    free(auction_list);
    strcpy(ptr, "\n");

    if (get_mode_verbose()) 
        printf("List all: Received request. %d auctions returned.\n", num_auctions);

    server_udp_send(response, to_addr, to_addr_len);
}  

/***
 * Sends back the information of an auction, if it exists
 * 
 * @param message The UDP request that contains the info necessary for the login.
 * It should have this format: 'SRC AID'
 * @param to_addr The address where the UDP message should be sent to
 * @param to_addr_len The length of the address where the UDP message is sent
*/
void show_record_handling(char * message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    char response[LIST_BUFFER];
    char * res_ptr = response;
    memset(response, 0, sizeof(response));

    strtok(message, " ");    // This only gets the "SRC " string

    // Get the auction ID and verify if it's a 3-digit number
    char AID[AID_SIZE];
    strcpy(AID, strtok(NULL, "\n"));
    if (!verify_format_AID(AID)) {
        if (get_mode_verbose()) printf("Invalid UDP request made to server.\n");
        server_udp_send("ERR\n", to_addr, to_addr_len);
        return;
    }

    strcpy(res_ptr, "RRC OK ");
    res_ptr += 7;

    // Get the auction info and writes it to the response
    if (GetAuctionInfo(AID, res_ptr) < 0) {
        if (get_mode_verbose()) printf("Show record: Auction %s does not exist.\n", AID);
        server_udp_send("RRC NOK\n", to_addr, to_addr_len);
        return;
    }

    // Adds a newline to the end of the response
    response[strlen(response)] = '\n';

    if (get_mode_verbose()) 
        printf("Show record: Requested record of auction #%s.\n", AID);

    server_udp_send(response, to_addr, to_addr_len);
}

/***
 * Opens an auction, if the user is logged in and the information given is valid.
 * Copies the file given in the TCP request to the server
 * 
 * @param socket_fd The socket where the parameters must be read
*/
void open_auction_handling(int socket_fd) {
    char buffer[SMALL_BUFFER];
    char UID[UID_SIZE], password[PASSWORD_SIZE], name[FILENAME_SIZE], start_value[VALUE_SIZE], timea_active[TIMEPASSED_SIZE], Fname[FILENAME_SIZE], Fsize[FILE_SIZE_STR];
    char res[SMALL_BUFFER];

    memset(buffer, 0, sizeof buffer);
    memset(UID, 0, sizeof UID);
    memset(password, 0, sizeof password);
    memset(name, 0, sizeof name);
    memset(start_value, 0, sizeof start_value);
    memset(timea_active, 0, sizeof timea_active);
    memset(Fname, 0, sizeof Fname);
    memset(Fsize, 0, sizeof Fsize);

   // Variables for parsing
    char currentField[SMALL_BUFFER];
    int currentFieldIndex = 0;

    // Receive bytes from the message
    memset(currentField, 0, sizeof(currentField));

    int read_size = 0;
    int leave = 0;

    memset(buffer, 0, sizeof(buffer));

    while (!leave && (read_size = server_tcp_receive(socket_fd, buffer, 1)) > 0) {
        if (buffer[0] == ' ') {
            // Space encountered, process the current field
            switch (currentFieldIndex) {
                case 0: strcpy(UID, currentField); break;
                case 1: strcpy(password, currentField); break;
                case 2: strcpy(name, currentField); break;
                case 3: strcpy(start_value, currentField); break;
                case 4: strcpy(timea_active, currentField); break;
                case 5: strcpy(Fname, currentField); break;
                case 6: strcpy(Fsize, currentField); leave = 1; break;
            }
            
            // Move to the next field
            memset(currentField, 0, sizeof(currentField));
            currentFieldIndex++;
        } else {
            // Append the current character to the current field
            strncat(currentField, buffer, 1);
        }
    }

    if (read_size == -1) {
        if (get_mode_verbose())
            printf("Failed to receive TCP message. Closing connection.\n");
        return;
    }

    if (get_mode_verbose()) {
        printf("Open auction: User %s is trying to open an auction.\n", UID);
        printf("Open auction: Name: %s\n", name);
        printf("Open auction: Start value: %s\n", start_value);
        printf("Open auction: Time active: %s\n", timea_active);
        printf("Open auction: File name: %s\n", Fname);
        printf("Open auction: File size: %s\n", Fsize);
    }

    if (!verify_format_id(UID) || !verify_format_password(password)) {
        if (get_mode_verbose()) {
            printf("Invalid TCP request made to server.\n");
            printf("ERRORS IN:\nver_UID: %d\nver_aid: %d\nver_pass: %d\nver_time: %d\nver_size: %d\n", verify_format_id(UID), verify_format_id(start_value), verify_format_password(password), verify_format_AID(timea_active), verify_format_AID(Fsize));
        }
        server_tcp_send(socket_fd, "ROA ERR\n", 8);
        return;
    }

    // Check if the message is valid
    if (CheckUserLogged(UID, password) != 1) {
        sprintf(res, "ROA NLG\n");
        server_tcp_send(socket_fd, res, strlen(res));
        return;
    }

    char start_time[DATETIME_SIZE];
    time_t now;

    // Get the current time and the time str 
    time(&now);
    timeToString(now, start_time);

    // Creates the auction in the DB. If it is successful, it returns the AID
    int AID = CreateAuction(UID, name, Fname, start_value, timea_active, start_time, now, Fsize, socket_fd);

    if (AID <= 0) {
        sprintf(res, "ROA NOK\n");
        server_tcp_send(socket_fd, res, strlen(res));
        return;
    }

    // Sends the AID to the client
    sprintf(res, "ROA OK %03d\n", AID);
    server_tcp_send(socket_fd, res, strlen(res));

    if (get_mode_verbose()) 
        printf("Open auction: Assigned auction number %d.\n", AID);

    return;

}


/***
 * Closes an auction, if it exists and the user is the owner of the auction
 * 
 * @param socket_fd The socket where the parameters must be read
*/
void close_auction_handling(int socket_fd) {
    char buffer[SMALL_BUFFER];

    // Receive the remaining bytes of the message
    memset(buffer, 0, sizeof buffer);

    char * ptr = buffer;

    //reads byte by byte until it finds a \n
    while (server_tcp_receive(socket_fd, ptr, 1) > 0) {
        if (get_mode_verbose()) printf("ptr: %c\n", *ptr);
        if (*ptr == '\n') break;
        ptr++;
    }
    
    // Check if the message is valid
    char UID[UID_SIZE], AID[AID_SIZE], password[PASSWORD_SIZE];
    sscanf(buffer, "%s %s %s\n", UID, password, AID);

    if (get_mode_verbose()) {
        printf("Close auction: User %s is trying to close an auction.\n", UID);
        printf("Close auction: Auction ID: %s\n", AID);
    }

    if (!verify_format_id(UID) || !verify_format_password(password) || !verify_format_AID(AID)) {
        if (get_mode_verbose()) {
            printf("Invalid TCP request made to server.\n");
            printf("ERRORS IN:\nver_UID: %d\nver_aid: %d\nver_pass: %d\n", verify_format_id(UID), verify_format_id(AID), verify_format_password(password));
        }
        server_tcp_send(socket_fd, "ERR\n", 4);
        return;
    }

    // Check if the user is logged in
    int status = CheckUserLogged(UID, password);
    if (status <= 0) {
        if (get_mode_verbose())
            printf("Close auction: User %s is not logged in.\n", UID);
        server_tcp_send(socket_fd, "RCL NLG\n", 8);
        return;
    }

    // Tries to close the auction
    status = CloseAuction(AID, UID);
    switch (status) {
        case 0:
            if (get_mode_verbose())
                printf("Close auction: Auction %s has been closed.\n", AID);
            server_tcp_send(socket_fd, "RCL OK\n", 7);
            return;
        case -1:
            if (get_mode_verbose())
                printf("Close auction: Auction does not exist - %s.\n", AID);
            server_tcp_send(socket_fd, "RCL EAU\n", 8);
            return;
        case -2:
            if (get_mode_verbose())
                printf("Close auction: User %s is not the auction owner.\n", UID);
            server_tcp_send(socket_fd, "RCL EOW\n", 8);
            return;
        case -3:
            if (get_mode_verbose())
                printf("Close auction: Auction %s has already ended.\n", AID);
            server_tcp_send(socket_fd, "RCL END\n", 8);
            return;
    }

    if(get_mode_verbose())
        printf("Close Auction: Auction is now closed.\n");

    server_tcp_send(socket_fd, "RCL ERR\n", 8);
    return;
}

/***
 * Fetches an asset from the auction specified in the TCP request, and
 * sends a response containing the file data.
 * 
 * @param seocket_fd The socket that contains the auction number
*/
void show_asset_handling(int socket_fd) {
    char aid[AID_SIZE], *ptr;
    char buffer[SMALL_BUFFER];

    memset(aid, 0, sizeof aid);
    ptr = aid;

    int i = 0;
    while(server_tcp_receive(socket_fd, ptr, 1) > 0 && i <= AID_SIZE) {
        if (*ptr == '\n') {
            *ptr = '\0';
            break;
        }
        i++;
        ptr++;
    }

    // Verify if AID is valid
    if(!verify_format_AID(aid) || i == AID_SIZE) {
        if (get_mode_verbose()) printf("Invalid TCP request made to server.\n");
        memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "ERR\n");
        server_tcp_send(socket_fd, buffer, strlen(buffer));
        return;
    }

    // Starts interaction with the database
    if(ShowAsset(aid, socket_fd) == -1) {
        memset(buffer, 0, sizeof buffer);
        strcpy(buffer, "RSA NOK\n");
        server_tcp_send(socket_fd, buffer, strlen(buffer));
        return;
    }
}

/***
 * Creates a new bid using the paramters specified in the TCP request.
 * 
 * @param socket_fd The socket where the parameters must be read
*/
void bid_handling(int socket_fd) {
    char userID[UID_SIZE], userPasswd[PASSWORD_SIZE], auctionID[AID_SIZE], auctionValue[VALUE_SIZE];
    char* ptr;

    // Check if the userID is valid
    ptr = userID;
    memset(userID, 0, sizeof userID);
    int i = 0;
    while(server_tcp_receive(socket_fd, ptr, 1) > 0 && i < 7) {
        if (*ptr == ' ') { *ptr = '\0'; break; }
        if (*ptr == '\n') { i = 7; break; }

        i++;
        ptr++;
    }
    if(i == 7 || !verify_format_id(userID)) {
        memset(userID, 0, sizeof userID);
        strcpy(userID, "ERR\n");
        server_tcp_send(socket_fd, userID, strlen(userID));
        return;
    }

    // Check if the userPasswd is valid
    ptr = userPasswd;
    memset(userPasswd, 0, sizeof userPasswd);
    i = 0;
    while(server_tcp_receive(socket_fd, ptr, 1) > 0 && i < 9) {
        if (*ptr == ' ') { *ptr = '\0'; break; }
        if (*ptr == '\n') { i = 9; break; }
        
        i++;
        ptr++;
    }
    if(i == 9 || !verify_format_password(userPasswd)) {
        memset(userPasswd, 0, sizeof userPasswd);
        strcpy(userPasswd, "ERR\n");
        server_tcp_send(socket_fd, userPasswd, strlen(userPasswd));
        return;
    }

    // Check if the auctionID is valid
    ptr = auctionID;
    memset(auctionID, 0, sizeof auctionID);
    i = 0;
    while(server_tcp_receive(socket_fd, ptr, 1) > 0 && i < 4) {
        if (*ptr == ' ') { *ptr = '\0'; break; }
        if (*ptr == '\n') { i = 4; break; }
        
        i++;
        ptr++;
    }
    if(i == 4 || !verify_format_AID(auctionID)) {
        memset(auctionID, 0, sizeof auctionID);
        server_tcp_send(socket_fd, "ERR\n", 4);
        return;
    }

    // Check if the bid value is valid
    ptr = auctionValue;
    memset(auctionValue, 0, sizeof auctionValue);
    i = 0;
    for (i = 0; i < 17; i++, ptr++) {
        if(i == 17 || server_tcp_receive(socket_fd, ptr, 1) <= 0) {
            memset(auctionValue, 0, sizeof auctionValue);
            server_tcp_send(socket_fd, "ERR\n", 4);
            return;
        }

        if(*ptr == '\n') { 
            if (i == 0) {
                memset(auctionValue, 0, sizeof auctionValue);
                server_tcp_send(socket_fd, "ERR\n", 4);
                return;
            } else { *ptr = '\0'; break; }
        }

        if (!isdigit(*ptr)) {
            memset(auctionValue, 0, sizeof auctionValue);
            server_tcp_send(socket_fd, "ERR\n", 4);
            return;
        }
    }

    if (get_mode_verbose()) 
        printf("Bid: User %s has requested a bid of value %s for auction %s\n", 
                userID, auctionValue, auctionID);

    // Tries to create the bid in the db
    int status = Bid(auctionID, userID, auctionValue);
    switch (status)
    {
    case 0:
        // Bid was successful
        server_tcp_send(socket_fd, "RBD ACC\n", 8);
        break;
    case -1:
        // Auction has ended
        if (get_mode_verbose()) printf("Bid: Request rejected - auction is closed\n");
        server_tcp_send(socket_fd, "RBD NOK\n", 8);
        break;
    case -2:
        // Highest bid is higher than the one given
        if (get_mode_verbose()) printf("Bid: Request rejected - there is a higher bid\n");
        server_tcp_send(socket_fd, "RBD REF\n", 8);
        break;
    case -3:
        // User is the owner of the auction
        if (get_mode_verbose()) printf("Bid: Request rejected - the owner cannot bid in its own auction\n");
        server_tcp_send(socket_fd, "RBD ILG\n", 8);
        break;
    case -4:
        // User is not logged in
        if (get_mode_verbose()) printf("Bid: Request rejected - no user logged in\n");
        server_tcp_send(socket_fd, "RBD NLG\n", 8);
        break;
    default:
        if (get_mode_verbose()) printf("Bid: Request rejected - internal error\n");
        server_tcp_send(socket_fd, "RBD ERR\n", 8);
        break;
    }

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
    char buffer[MEDIUM_BUFFER], message_type[MESSAGE_TYPE_SIZE];
    struct sockaddr sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    memset(buffer, 0, sizeof buffer);
    status = server_udp_receive(buffer, sizeof(buffer), &sender_addr, &sender_addr_len);
    if (status == -1) {
        if (get_mode_verbose()) printf("Failed to receive request.\n");
        return -1;   // Go to the next mesage
    }

    memset(message_type, 0, sizeof message_type);
    // Get the first 4 characters of the message to determine its type
    strncpy(message_type, buffer, 4);

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
        show_record_handling(buffer, &sender_addr, sender_addr_len);
    } else {
        if(get_mode_verbose())
            printf("Invalid UDP request made to server.\n");
        
        // The message is invalid in this case.
        // Send response ERR (invalid request).
        status = server_udp_send("ERR\n", &sender_addr, sender_addr_len);
        if (status == -1) return -1;
    }

    return 0;
}

/***
 * Receives a TCP message, processes it and sends its corresponding response
 * depending on the type of message.
 * 
 * @return 0 if the request is processed correctly, -1 if there was an error
 * processing the request  
*/
int handle_tcp_request() {
    int socket_fd, status;
    char buffer[TINY_BUFFER];
    struct timeval timeout;

    // Accept the new connection to the new socket
    socket_fd = server_tcp_accept();
    if (socket_fd == -1) {
        if (get_mode_verbose())
            printf("Failed to accept TCP connection.\n");
        return -1;
    }

    // Set a timeout in case the socket is stuck on read/write
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
            sizeof timeout) < 0 ||
            setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout,
            sizeof timeout) < 0) {
        if (get_mode_verbose())
            printf("Failed to establish TCP socket timeout.\n");
        server_tcp_close(socket_fd);
        return -1;
    }

    // Receive the first 4 characters of a TCP message and compare it
    // against the list of actions available
    memset(buffer, 0, sizeof buffer);
    status = server_tcp_receive(socket_fd, buffer, 4);
    if (status == -1) {
        if (get_mode_verbose())
            printf("Failed to receive TCP message. Closing connection.\n");
        server_tcp_close(socket_fd);
        return -1;
    }

    if(!strcmp(buffer, "OPA ")) {
        open_auction_handling(socket_fd);
    } else if(!strcmp(buffer, "CLS ")) {
        close_auction_handling(socket_fd);
    } else if(!strcmp(buffer, "SAS ")) {
        show_asset_handling(socket_fd);
    } else if(!strcmp(buffer, "BID ")) {
        bid_handling(socket_fd);
    } else {
        // Send an error response using TCP
        status = server_tcp_send(socket_fd, "ERR\n", 4);
        if (status == -1 && get_mode_verbose()) {
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
    struct sigaction act;

    // Define verbose mode and port number according to the arguments used
    // to run this program
    set_program_parameters(argc, argv);
    InitDatabase();

    //Handle sigint
    signal(SIGINT, sig_handler);

    //Handle sigpipe
    memset(&act, 0, sizeof act);
    act.sa_handler=SIG_IGN;
    if(sigaction(SIGPIPE, &act, NULL) == -1) {
        printf("Failed to set SISPIPE behaviour.\n");
        return -1;
    }

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
        timeout.tv_sec = TIMEOUT;     // Set a timeout for every select function

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