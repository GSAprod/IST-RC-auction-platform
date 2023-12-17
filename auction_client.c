#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include "client_connections.h"
#include "file_handling.h"

// soulindo -> Password

// User credentials (they can be used in many requests to the server)
char userID[UID_SIZE], userPasswd[PASSWORD_SIZE];

/***
 * Splits the prompt into a list of prompt arguments (similarly to argv in
 * the main function)
 * 
 * @param prompt The prompt string
 * @param prompt_args A list of strings to where the propmt arguments are going to
 * be written to
 * @return The number of arguments 
*/
int promptToArgsList(char* prompt, char prompt_args[][MEDIUM_BUFFER]) {
    int argCount = 0;

    // Split the string prompt until the first space character
    char *token = strtok(prompt, " ");
    while(token != NULL) {
        // Check if the last tokenized string doesn't just contain the newline character
        // (this may happen if a space is put after the prompt)
        if(strcmp("\n", token)) {
            sscanf(token, "%s", prompt_args[argCount]);
            argCount++;
        }
        // Split the remaining string
        token = strtok(NULL, " ");
    }
    return argCount;
}

/***
 * Login to the auction server using the credentials provided.
 * The function sends a message to the AS using the UDP protocol with the
 * user credentials provided, and displays the result of the request, which
 * can be a successful login, a failed login, and a new user being generated.
 * 
 * @param arg_count The number of arguments given to the command
 * @param args A list of arguments given to the function. The list should have
 * 3 arguments: the name of the command, the istID and a password.
*/
void clientLogin(int arg_count, char args[][MEDIUM_BUFFER]) {
    int scanf_success;
    int istId, passwdLen, status;
    char buffer[MEDIUM_BUFFER], aux[SMALL_BUFFER];

    // Argument verification
    if(arg_count != 3) {
        printf("Login: Wrong arguments given.\n\tlogin UID password\n");
        return;
    }

    // Verify if the istID is a 6 digit number
    scanf_success = sscanf(args[1], "%d", &istId);
    if (scanf_success != 1 || strlen(args[1]) != 6) {
        printf("Login: Wrong arguments given.\n\tlogin UID password\n");
        return;
    }
    
    // Verify if the student password is an 8 character alphanumeric string
    passwdLen = strlen(args[2]);
    if (passwdLen != 8) {
        printf("Login: Wrong arguments given.\n\tlogin UID password\n");
        return;
    }
    for(int i = 0; i < passwdLen; i++) {
        char c = args[2][i];
        if (!isalpha(c) && !isdigit(c)) {
            printf("Login: Wrong arguments given.\n\tlogin UID password\n");
            return;
        }
    }

    // Generate the string for sending the login request to the server, and
    // send it using the UDP protocol
    sprintf(buffer, "LIN %06d %s\n", istId, args[2]);
    status = udp_send(buffer);
    if (status == -1) {
        printf("Login: failed to send request");
        return;
    }

    // Check if the response type of the server is RLI
    memset(buffer, 0, sizeof buffer);
    memset(aux, 0, sizeof aux);
    status = udp_receive(buffer, sizeof buffer);
    if (status == -1) {
        printf("Login: failed to receive response from server.\n");
        return;
    }
    strncpy(aux, buffer, 4);
    if(strcmp(aux, "RLI ")) {
        printf("Login: failed to receive response from server.\n");
        return;
    }
    
    // Check the status response sent by the server and inform the user
    strncpy(aux, buffer+4, 4);
    if (!strcmp(aux, "NOK\n")) {
        printf("Invalid user credentials.\n");
        return;
    } else if (!strcmp(aux, "OK\n")) {
        printf("User successfully logged in.\n");
        
    } else if (!strcmp(aux, "REG\n")) {
        printf("No user with matching ID. Registered new user.\n");
        
    } else {
        printf("Login: failed to receive response from server.\n");
        return;
    }

    // If the function didn't return before, then the user is now successfully
    // logged in. Let's store the credentials in order to send other requests
    // that require authentication.
    strcpy(userID, args[1]);
    strcpy(userPasswd, args[2]);
    return;
}

/***
 * Logout from the server using the credentials stored while logging in.
 * The function checks if there are user credentials stored, and if so, sends
 * a UDP request to log out the user from the server.
 * The user credentials are stored the global variables called userId and userPasswd. 
*/
void userLogout() {
    char buffer[MEDIUM_BUFFER];

    // Check if the user is logged in
    if (!strcmp(userID, "") || !strcmp(userPasswd, "")) {
        printf("user not logged in\n");
        return;
    }
    
    // Send the logout command to the server
    sprintf(buffer, "LOU %s %s\n", userID, userPasswd);
    udp_send(buffer);

    memset(buffer, 0, sizeof buffer);

    // Receive the response from the server
    udp_receive(buffer, sizeof buffer);

    // Check if the response is correct
    char * token = strtok(buffer, " ");

    if (strcmp(token, "RLO")) {
        printf("Logout: Invalid response from server.\n");
        return;
    }

    if (!strcmp(buffer + 4, "OK\n")) {
        printf("successful logout\n");

    } else if (!strcmp(buffer + 4, "NOK\n")) {
        printf("user not logged in\n");
    } else if (!strcmp(buffer + 4, "UNR\n")) {
        printf("unknown user\n");
    } else {
        printf("Logout: Invalid response from server.\n");
        return;
    }

    // If the function didn't return before, then the user is now successfully
    // unregistered/logged out. Let's wipe the credentials from our program as
    // they are not required anymore.
    memset(userID, 0, sizeof userID);
    memset(userPasswd, 0, sizeof userPasswd);
}

/***
 * Logs out and unregisters a user from the database.
 * The function checks if there are credentails stored, and if so, sends a request
 * to the server to unregister the client.
 * The user credentials are stored the global variables called userId and userPasswd. 
 * 
 * @param arg_count The number of arguments of the prompt
*/
void clientUnregister(int arg_count) {
    char buffer[MEDIUM_BUFFER], aux[SMALL_BUFFER];
    int status;

    if (arg_count != 1) {
        printf("Unregister: Wrong arguments given.\n\tunregister\n");
        return;
    }

    // If the client hasn't logged in, the user credentials are not stored in
    // the client app. Therefore, we don't even need to query the server because
    // we have no credentials to even send the login request
    if (!strcmp(userID, "") || !strcmp(userPasswd, "")) {
        printf("User is not logged in.\n");
        return;
    }

    // Generate the string for sending the unregister request to the server, 
    // and send it using the UDP protocol
    sprintf(buffer, "UNR %s %s\n", userID, userPasswd);
    status = udp_send(buffer);
    if (status == -1) {
        printf("Unregister: failed to send request");
        return;
    }

    // Check if the response type of the server is RUR
    memset(buffer, 0, sizeof buffer);
    status = udp_receive(buffer, sizeof buffer);
    if (status == -1) {
        printf("Unregister: failed to receive response from server.\n");
        return;
    }
    strncpy(aux, buffer, 4);
    if(strcmp(aux, "RUR ")) {
        printf("Login: failed to receive response from server.\n");
        return;
    }

    // Check the status response sent by the server and inform the user
    strncpy(aux, buffer+4, 4);
    if (!strcmp(aux, "NOK\n")) {
        printf("User is not logged in.\n");

    } else if (!strcmp(aux, "OK\n")) {
        printf("User successfully unregistered.\n");
        
    } else if (!strcmp(aux, "UNR\n")) {
        printf("User not registered.\n");
        
    } else {
        printf("Unregister: failed to receive response from server.\n");
        return;
    }

    // If the function didn't return before, then the user is now successfully
    // unregistered/logged out. Let's wipe the credentials from our client as
    // they are not required anymore.
    memset(userID, 0, sizeof userID);
    memset(userPasswd, 0, sizeof userPasswd);
}

/***
 * Checks if the user is logged out and terminates the program after closing the
 * TCP and UDP connections.
 * 
 * @return 1 if the user is logged out, 0 otherwise
*/
int exitScript() {
    if (strcmp(userID, "") || strcmp(userPasswd, "")) {
        printf("User not logged out\n");
        return 0;
    }
    else {
        printf("Exiting...\n");
        TCP_free();
        UDP_free();
        return 1;
    }
}

/***
 * Prints a list of auctions based on a string formatted like the following:
 * AID state[ AID state]*\\n
 * 
 * @param auctionListStr The string with the list of auctions
 * @return 0 if the entire list could be printed;
 * -1 if the string is invalid;
 * -2 if there are no auctions.
*/
int printAuctions(char* auctionListStr) {
    char *token;
    int status, auction_num, status_num, has_entries;

    // Print the list of auctions
    token = strtok(auctionListStr, " ");
    while (token != NULL) {
        // Check if the next substring is a 3-digit auction number 
        status = sscanf(token, "%03d", &auction_num);
        if (status == EOF) {
            return -1;
        }

        // Get the next substring and check if it's the status number.
        token = strtok(NULL, " ");
        if(token == NULL) {
            return -1;
        }
        status = sscanf(token, "%01d", &status_num);
        if (status == EOF) {
            return -1;
        }

        // Print the table's header, if it hasn't been printed before
        if(!has_entries) {
            has_entries = 1;
            printf("AUCTION\tSTATUS\n");
        } 

        // Print the auction using the auction_num and status_num
        printf("%03d\t%s\n", auction_num, status_num == 1 ? "active" : "not active");

        token = strtok(NULL, " ");
    }

    // If no auctions have been started, show the message to the user
    if (!has_entries) {
        return -2;
    }

    return 0;
}

/***
 * Lists all auctions of the server and their respective status.
 * 
 * @param arg_count The number of arguments of the prompt
 */
void listAllAuctions(int arg_count) {
    char buffer[LIST_BUFFER], aux[SMALL_BUFFER];
    int status;

    if (arg_count != 1) {
        printf("List all auctions: Wrong arguments given.\n\tlist\n\tl\n");
        return;
    }

    // Generate the string for sending the auction listing request to the server, 
    // and send it using the UDP protocol
    sprintf(buffer, "LST\n");
    status = udp_send(buffer);
    if (status == -1) {
        printf("List all auctions: failed to send request");
        return;
    }

    // Check if the response type of the server is RLS
    memset(buffer, 0, sizeof buffer);
    memset(aux, 0, sizeof aux);
    status = udp_receive(buffer, sizeof buffer);
    if (status == -1) {
        printf("List all auctions: failed to receive response from server.\n");
        return;
    }
    strncpy(aux, buffer, 4);
    if (strcmp(aux, "RLS ")) {
        printf("List all auctions: failed to receive response from server.\n");
        return;
    }

    memset(aux, 0, sizeof aux);
    strncpy(aux, buffer+4, 3);
    if (!strcmp(aux, "NOK")) {
        printf("No auctions have yet been started.\n");
        return;
    } else if (!strcmp(aux, "OK ")) {
        // Print a table with all auctions and their respective status
        status = printAuctions(buffer + 7);
        if (status == -1) {
            printf("List all auctions: Invalid response from server.\n");
        } else if (status == -2) {
            printf("No auctions have yet been started.\n");
        }
    } else {
        // If a different status (other than OK and NOK) is sent by the server,
        // send an error
        printf("List all auctions: failed to receive response from server.\n");
        return;
    }
}

/***
 * Lists all auctions that the logged in user (with login credentials in the
 * global variables userId and userPasswd) has created.
 * 
 * @param arg_count The number of arguments of the prompt
 */
void listMyAuctions(int arg_count) {
    char buffer[LIST_BUFFER], aux[SMALL_BUFFER];
    int status;

    if (arg_count != 1) {
        printf("List all auctions: Wrong arguments given.\n\tlist\n\tl\n");
        return;
    }

    if (!strcmp(userID, "")) {
        printf("No user is logged in.\n");
        return;
    }

    // Generate the string for sending the auction listing request to the server, 
    // and send it using the UDP protocol
    sprintf(buffer, "LMA %s\n", userID);
    status = udp_send(buffer);
    if (status == -1) {
        printf("List my auctions: failed to send request");
        return;
    }

    // Check if the response type of the server is RMA
    memset(buffer, 0, sizeof buffer);
    memset(aux, 0, sizeof aux);
    status = udp_receive(buffer, sizeof buffer);
    if (status == -1) {
        printf("List my auctions: failed to receive response from server.\n");
        return;
    }
    strncpy(aux, buffer, 4);
    if (strcmp(aux, "RMA ")) {
        printf("List my auctions: failed to receive response from server.\n");
        return;
    }

    memset(aux, 0, sizeof aux);
    strncpy(aux, buffer+4, 3);
    if (!strcmp(aux, "NOK")) {
        printf("You have no ongoing auctions.\n");
        return;
    } else if (!strcmp(aux,"NLG")) {
        printf("No user is logged in.\n");
        return;
    } else if (!strcmp(aux, "OK ")) {
        //! This part has not yet been tested!
        // Print a table with all auctions and their respective status
        status = printAuctions(buffer + 7);
        if (status == -1) {
            printf("List all auctions: Invalid response from server.\n");
        } else if (status == -2) {
            printf("No auctions have yet been started.\n");
        }
    } else {
        // If a different status (other than OK and NOK) is sent by the server,
        // send an error
        printf("List all auctions: failed to receive response from server.\n");
        return;
    }
}

/***
 * Lists all auctions that the logged in user (with login credentials in the
 * global variables userId and userPasswd) has created.
 * 
 * @param arg_count The number of arguments of the prompt
 */
void myBids(int arg_count) {
    char buffer[LIST_BUFFER];

    if (arg_count != 1) {
        printf("List my bids: Wrong arguments given.\n\t> mybids\n\t> mb\n");
        return;
    }

    if (!strcmp(userID, "")) {
        printf("No user is logged in.\n");
        return;
    }

    sprintf(buffer, "LMB %s\n", userID);
    
    udp_send(buffer);

    memset(buffer, 0, sizeof buffer);

    int status = udp_receive(buffer, sizeof buffer);

    if (status == -1) {
        printf("My bids: failed to receive response from server.\n");
        return;
    }

    char aux[SMALL_BUFFER];
    memset(aux, 0, sizeof aux);

    strncpy(aux, buffer, 3);

    if (strcmp(aux, "RMB")) {
        printf("My bids: Invalid response from server.\n");
        return;
    }

    memset(aux, 0, sizeof aux);
    strncpy(aux, buffer + 4, 3);

    if (!strcmp(aux, "NOK")) {
        printf("User has no ongoing bids.\n");
        return;
    } else if (!strcmp(aux, "NLG")) {
        printf("No user is logged in.\n");
        return;
    } else if (!strcmp(aux, "OK ")) {
        int i = printAuctions(buffer + 7);

        if (i == -1) {
            printf("Invalid response from server.\n");
        } else if (i == -2) {
            printf("User is not logged in.\n");
        }
    }
    else {
        printf("My bids: Invalid response from server.\n");
        return;
    }
}

/***
 * Receives a substring related to the info of an auction,
 * taken from the show_record response, and formats it appropriately.
 * 
 * @param in_str The string to read the auction info
 * @param out_str The pointer to where the formatted text should be written
 * @return 0 if the string given is valid, -1 otherwise
*/
int aux_formatAuctionInfo(char* in_str, char* out_str) {
    char* ptr = in_str;
    char buffer[256];
    int bytesRead = 7;

    // Read host UID
    strcpy(out_str, "Auction record:\n================\nStarted by    | ");
    int i = 0;
    while(1) {
        if (*ptr == ' ') { buffer[i] = '\0'; ptr++; break; }
        if (!isdigit(*ptr)) return -1;

        buffer[i++] = *ptr;
        ptr++;
    }
    if (i != 6) return -1;
    strcat(out_str, buffer);
    memset(buffer, 0, sizeof buffer);

    // Read auction name
    strcat(out_str, "\nAuction name  | ");
    i = 0;
    while(1) {
        if (*ptr == ' ') { buffer[i] = '\0'; ptr++; break; }
        if ((!isalnum(*ptr) && *ptr != '.' && *ptr != '-' && *ptr != '_') || i >= 24) return -1;

        buffer[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    strcat(out_str, buffer);
    memset(buffer, 0, sizeof buffer);
    bytesRead += i + 1;
    
    // Read name of asset
    strcat(out_str, "\nName of asset | ");
    i = 0;
    while(1) {
        if (*ptr == ' ') { buffer[i] = '\0'; ptr++; break; }
        if ((!isalnum(*ptr) && *ptr != '.' && *ptr != '-' && *ptr != '_') || i >= 24) return -1;

        buffer[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    strcat(out_str, buffer);
    memset(buffer, 0, sizeof buffer);
    bytesRead += i + 1;

    // Read start value
    strcat(out_str, "\nStart value   | ");
    i = 0;
    while(1) {
        if (*ptr == ' ') { buffer[i] = '\0'; ptr++; break; }
        if (!isdigit(*ptr) || i >= 6) return -1;

        buffer[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    strcat(out_str, buffer);
    memset(buffer, 0, sizeof buffer);
    bytesRead += i + 1;

    // Read start date time
    strcat(out_str, "\nStarted at    | ");
    i = 0;
    int time_separator = 0;
    while(1) {
        if (*ptr == ' ') { 
            if (time_separator == 1) {
                buffer[i] = '\0'; 
                ptr++; 
                break;
            } else time_separator = 1;
        }
        if (*ptr == '\n' || *ptr == '\0') return -1;

        buffer[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    strcat(out_str, buffer);
    memset(buffer, 0, sizeof buffer);
    bytesRead += i + 1;

    // Read time active
    strcat(out_str, "\nDuration      | ");
    i = 0;
    while(1) {
        if (*ptr == ' ' || *ptr == '\n') { buffer[i] = '\0'; break; }
        if (!isdigit(*ptr) || i >= 5) return -1;

        buffer[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    strcat(out_str, buffer);
    memset(buffer, 0, sizeof buffer);
    strcat(out_str, "\n");
    bytesRead += i;

    return bytesRead;
}

/***
 * Receives a substring related to the info of a bid,
 * taken from the show_record response, and formats it appropriately.
 * 
 * @param in_str The string to read the bid info
 * @param out_str The pointer to where the formatted text should be written
 * @return 0 if the string given is valid, -1 otherwise
*/
int aux_formatAuctionBid(char* in_str, char* out_str) {
    char* ptr = in_str;
    char bidderID[UID_SIZE], bid_value[VALUE_SIZE], bid_date[DATETIME_SIZE], bid_seconds[TIMEPASSED_SIZE];
    int bytesRead = 7;

    // Read bidder UID
    int i = 0;
    while(1) {
        if (*ptr == ' ') { bidderID[i] = '\0'; ptr++; break; }
        if (!isdigit(*ptr) || i >= 6) return -1;

        bidderID[i++] = *ptr;
        ptr++;
    }
    if (i != 6) return -1;

    // Read bid value
    i = 0;
    while(1) {
        if (*ptr == ' ') { bid_value[i] = '\0'; ptr++; break; }
        if (!isdigit(*ptr) || i >= 6) return -1;

        bid_value[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    bytesRead += i + 1;

    // Read bid date and time
    i = 0;
    int time_separator = 0;
    while(1) {
        if (*ptr == ' ') { 
            if (time_separator == 1) {
                bid_date[i] = '\0'; 
                ptr++; 
                break;
            } else time_separator = 1;
        }
        if (*ptr == '\n' || *ptr == '\0') return -1;

        bid_date[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    bytesRead += i + 1;

    // Read the number of seconds elapsed since the beggining of the auction
    // until the bid was placed
    i = 0;
    while(1) {
        if (*ptr == ' ' || *ptr == '\n') { bid_seconds[i] = '\0'; break; }
        if (!isdigit(*ptr) || i >= 5) return -1;

        bid_seconds[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    bytesRead += i;

    sprintf(out_str, "%s      %6s  %19s   %5s\n", bidderID, bid_value, bid_date, bid_seconds);
    return bytesRead;
}

/***
 * Receives a substring related to the end information of an auction,
 * taken from the show_record response, and formats it appropriately.
 * 
 * @param in_str The string to read the auction end info
 * @param out_str The pointer to where the formatted text should be written
 * @return 0 if the string given is valid, -1 otherwise
*/
int aux_formatAuctionEnd(char* in_str, char* out_str) {
    char* ptr = in_str;
    char end_date[DATETIME_SIZE], end_seconds[TIMEPASSED_SIZE];
    int bytesRead = 0;

    // Read bid date and time
    int i = 0;
    int time_separator = 0;
    while(1) {
        if (*ptr == ' ') { 
            if (time_separator == 1) {
                end_date[i] = '\0'; 
                ptr++; 
                break;
            } else time_separator = 1;
        }
        if (*ptr == '\n' || *ptr == '\0') return -1;

        end_date[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    bytesRead += i + 1;

    // Read the number of seconds elapsed between the beggining and end 
    // of the auction
    i = 0;
    while(1) {
        if (*ptr == '\n') { end_seconds[i] = '\0'; break; }
        if (!isdigit(*ptr) || i >= 5) return -1;

        end_seconds[i++] = *ptr;
        ptr++;
    }
    if (i == 0) return -1;
    bytesRead += i;

    sprintf(out_str, "\nAuction ended at %s (lasted %s seconds).\n", end_date, end_seconds);
    return bytesRead;
}

/***
 * Checks if the show_record response is formatted correctly, and formats it and
 * prints it onto the terminal.
 * 
 * @param auctions The show_record response received from the server
*/
void handleAuctions(char *auctions) {
    int status = 0, shift = 0;
    char buffer[LARGE_BUFFER];

    memset(buffer, 0, sizeof buffer);

    // Read auction
    status = aux_formatAuctionInfo(auctions, buffer);
    if (status == -1) return;

    write(1, buffer, strlen(buffer));
    printf("\n");
    shift += status;

    memset(buffer, 0, sizeof buffer);

    if (auctions[shift] != ' ')
        return;

    shift++;
    if (auctions[shift] == 'B') {
        printf("List of bids:\nBIDDER ID    VALUE  DATE/TIME            SECOND\n");
        
        while(auctions[shift] == 'B' && auctions[shift + 1] == ' ') {
            status = aux_formatAuctionBid(auctions + shift + 2, buffer);
            if (status == -1)
                return;

            write(1, buffer, strlen(buffer));
            memset(buffer, 0, sizeof buffer);
            shift += status + 2;

            if (auctions[shift] != ' ') return;
            shift += 1;
        }
    }

    if (auctions[shift] == 'E' && auctions[shift + 1] == ' ') {
        status = aux_formatAuctionEnd(auctions + shift + 2, buffer);
        if (status == -1)
            return;

        write(1, buffer, strlen(buffer));
    }

    return;
}

/***
 * Fetches the info about an auction, its last 50 bids, and whether the auction has
 * ended or not.
 * After checking if the arguments are correct, the function sends a request via
 * UDP to the server, asking for the info of a certain auction.
 * 
 * @param argc The number of arguments for verification purposes
 * @param argv The arguments given, containing the id of the auction we want
 * to get the info from 
*/
void showRecord(int argc, char argv[][128]) {

    // Verify if the function's arguments are correct
    if (argc != 2 || strlen(argv[1]) != 3 || atoi(argv[1]) == 0) {
        printf("Wrong arguments given.\n\t> show_record <auction_id>\n\t> sr <auction_id>\n\t (auction_id: 3-digit number)\n");
        return;
    }

    // Send the TCP request to the server
    char buffer[LIST_BUFFER];
    memset(buffer, 0, sizeof buffer);
    sprintf(buffer, "SRC %s\n", argv[1]);
    udp_send(buffer);

    // Receive the response from the server and start verifying and printing all
    // arguments
    memset(buffer, 0, sizeof buffer);
    int status = udp_receive(buffer, sizeof buffer);
    if (status == -1) {
        printf("Show record: failed to receive response from server.\n");
        return;
    }

    char aux[SMALL_BUFFER];
    char *ptr = buffer;
    memset(aux, 0, sizeof aux);
    strncpy(aux, ptr, 4);
    if (strcmp(aux, "RRC ")) {
        printf("Show record: Invalid response from server.\n");
        return;
    }

    ptr += 4;
    memset(aux, 0, sizeof aux);
    strncpy(aux, ptr, 3);

    if (!strcmp(aux, "NOK")) {
        if (*(buffer + 7) != '\n') {
            printf("Show record: Invalid response from server.\n");
        } else {
            printf("Auction does not exist.\n");
        }
        return;
    } else if (!strcmp(aux, "OK ")) {
        handleAuctions(buffer + 7);
    } else {
        printf("Show record: Invalid response from server.\n");
        return;
    }
}

/***
 * Opens an auction on the server.
 * After checking if the arguments are correct, this function sends a request
 * using TCP to open a new auction, with the given parameters and asset file.
 * 
 * @param arg_count The number of arguments given (used for verification)
 * @param arg_values The arguments given, such as the name of the auction, the
 * name of the asset, and the start value and duration.
*/
void openAuction(int arg_count, char arg_values[][128]) {
    char buffer[256];
    char * name = arg_values[1];
    char * fname = arg_values[2];
    char * start_value = arg_values[3];
    char * time_active = arg_values[4];
    int fsize;

    memset(buffer, 0, sizeof buffer);

    if (arg_count != 5) {
        printf("Open auction: Wrong arguments given.\n\t>open <asset> <asset_fname> <start_value> <time_active>\n");
        return;
    }

    // Check if there are any user credentials stored
    if (!strcmp(userID, "")) {
        printf("No user is logged in.\n");
        return;
    }

    // Check if the asset exists and is not empty
    fsize = checkAssetFile(fname);
    if (fsize <= 0) {
        printf("Asset file does not exist or is empty.\n");
        return;
    }

    setup_TCP();
    tcp_connect();

    //Sends the beginning of the TCP message
    sprintf(buffer, "OPA %s %s %s %s %s %s %d ", userID, userPasswd , name, start_value , time_active, fname, fsize);
    tcp_send(buffer, strlen(buffer));
    
    //Sends the file
    if (sendFile(fname, fsize)) {
        printf("Open auction: failed to send file.\n");
        return;
    }

    tcp_send("\n", 1);
    

    memset(buffer, 0, sizeof buffer);

    // Receives and verifies the response given by the server
    tcp_receive(buffer, sizeof buffer);

    printf("%s", buffer);
    char *token = strtok(buffer, " ");

    if (strcmp(token, "ROA")) {
        printf("Open auction: Invalid response from server.\n");
        TCP_free();
        return;
    }

    token = strtok(NULL, " ");

    if (!strcmp(token, "NOK\n")) {
        printf("Could not open auction.\n");
    }
    else if (!strcmp(token, "NLG\n")) {
        printf("No user is logged in.\n");
    }
    else if (!strcmp(token, "OK")) {
        token = strtok(NULL, " ");
        printf("Auction %s opened with ID %s.\n", name, token);
    }
    else {
        printf("Open auction: Invalid response from server.\n");
    }

    TCP_free();

    return;
}

/***
 * Requests the server to close a certain auction, specified by its AID.
 * 
 * @param arg_count The number of arguments given (for verification purposes)
 * @param arg_values The arguments given, containing the ID of the auction
*/
void closeAuction(int arg_count, char arg_values[][128]) {
    if (arg_count != 2) {
        printf("Close auction: Wrong arguments given.\n\t>close <auction_id>\n");
        return;
    }

    // Check if there are any user credentials stored
    if (!strcmp(userID, "")) {
        printf("No user is logged in.\n");
        return;
    }

    char * auction_id = arg_values[1];

    // Sends the close request to the server
    char buffer[128];
    memset(buffer, 0, sizeof buffer);

    sprintf(buffer, "CLS %s %s %s\n", userID, userPasswd, auction_id);

    setup_TCP();
    tcp_connect();

    tcp_send(buffer, strlen(buffer));

    memset(buffer, 0, sizeof buffer);

    // Receives the response and verifies if the format is correct
    tcp_receive(buffer, sizeof buffer);

    TCP_free();

    printf("%s", buffer);

    char aux[SMALL_BUFFER];
    memset(aux, 0, sizeof aux);

    strncpy(aux, buffer, 3);

    if (strcmp(aux, "RCL")) {
        printf("Close auction: Invalid response from server.\n");
        return;
    }

    printf("%s", buffer + 4);

    if (!strcmp(buffer + 4, "OK\n")) {
        printf("Auction successfully closed.\n");
        return;
    }
    else if (!strcmp(buffer + 4, "NLG\n")) {
        printf("User is not logged in.\n");
    }
    else if (!strcmp(buffer + 4, "EAU\n")) {
        printf("Auction does not exist.\n");
    }
    else if (!strcmp(buffer + 4, "EOW\n")) {
        printf("Auction not owned by user.\n");
    }
    else if (!strcmp(buffer + 4, "END\n")) {
        printf("Auction already closed.\n");
    } else {
        printf("Close auction: Invalid response from server.\n");
    }
}

/***
 * Requests the download of an asset from a particular auction.
 * 
 * @param arg_count The number of arguments given (for verification purposes)
 * @param arg_values The arguments given, containing the ID of the auction
*/
void showAsset(int arg_count, char arg_values[][128]) {
    char buffer[128], *ptr;
    int status, char_count;

    if (arg_count != 2) {
        printf("Show asset: Wrong arguments given.\n\t>show_asset <asset_id>\n");
        return;
    }

    char * asset_id = arg_values[1];

    memset(buffer, 0, sizeof buffer);

    sprintf(buffer, "SAS %s\n", asset_id);

    setup_TCP();
    tcp_connect();

    tcp_send(buffer, strlen(buffer));

    memset(buffer, 0, MEDIUM_BUFFER);

    // Check if the response type is RSA
    status = tcp_receive(buffer, 4);
    if (status != 4 || strcmp(buffer, "RSA ")) {
        printf("Show asset: Invalid response from server.\n");
        return;
    }
    memset(buffer, 0, sizeof buffer);
    
    // Get the status code
    status = tcp_receive(buffer, 3);
    if(status != 3) {
        printf("Show asset: Invalid response from server.\n");
        TCP_free();
        return;
    }

    if (!strcmp(buffer, "OK ")) {
        char fname[FILENAME_SIZE];
        char fsize[FILE_SIZE_STR];

        // Get the name of the file
        memset(buffer, 0, MEDIUM_BUFFER);
        ptr = buffer;
        char_count = 0;
        while((status = tcp_receive(ptr, 1)) > 0) {
            if (*ptr == ' ') { *ptr = '\0'; break; }
            if (!isalpha(*ptr) && *ptr != '-' && *ptr != '_'
                    && *ptr != '.') { status = -1; break; }

            char_count++;
            if (char_count == 25) { status = -1; break; }
            ptr++;
        }
        if (status == -1)  {
            printf("Show asset: Invalid response from server.\n");
            TCP_free();
            return;
        }
        strcpy(fname, buffer);

        // Get the size of the file
        memset(buffer, 0, MEDIUM_BUFFER);
        ptr = buffer;
        char_count = 0;
        while((status = tcp_receive(ptr, 1)) > 0) {
            if (*ptr == ' ') { *ptr = '\0'; break; }
            if (!isdigit(*ptr)) { status = -1; break; }

            char_count++;
            if (char_count == 9) { status = -1; break; }
            ptr++;
        }
        if (status == -1)  {
            printf("Show asset: Invalid response from server.\n");
            TCP_free();
            return;
        }
        strcpy(fsize, buffer);

        // Receive the contents of the file
        receiveFile(fname, atoi(fsize));

        memset(buffer, 0, sizeof buffer);
        tcp_receive(buffer, 1);
        if (strcmp(buffer, "\n")) {
            printf("Invalid response from server\n");
            TCP_free();
            return;
        }

        TCP_free();

        printf("Asset downloaded: %s (%s bytes)\n", fname, fsize);
        return;
    } else if (!strcmp(buffer, "NOK")) {
        printf("Show asset: There was a problem receiving the file.\n");
        TCP_free();
        return;
    }


    TCP_free();

    printf("Show asset: Invalid response from server.\n");
    return;
}

/***
 * Requests the server for a bid on a certain auction.
 * 
 * @param arg_count The number of arguments given (for verification purposes)
 * @param arg_values The arguments given, containing the ID of the auction and
 * the value to bid
*/
void makeBid(int arg_count, char arg_vals[][128]) {
    if (arg_count != 3) {
        printf("Make bid: Wrong arguments given.\n\t>bid <auction_id> <value>\n");
        return;
    }

    // Check if there are any user credentials stored
    if (!strcmp(userID, "")) {
        printf("No user is logged in.\n");
        return;
    }

    char * auction_id = arg_vals[1];
    char * value = arg_vals[2];

    // Sends the bidding request to the server
    char buffer[128];
    memset(buffer, 0, sizeof buffer);

    sprintf(buffer, "BID %s %s %s %s\n", userID, userPasswd, auction_id, value);

    setup_TCP();
    tcp_connect();

    printf("%s", buffer);
    tcp_send(buffer, strlen(buffer));

    memset(buffer, 0, sizeof buffer);

    // Receives the response and verifies if the format is correct
    tcp_receive(buffer, sizeof buffer);

    TCP_free();

    char aux[SMALL_BUFFER];
    memset(aux, 0, sizeof aux);

    strncpy(aux, buffer, 3);

    if (strcmp(aux, "RBD")) {
        printf("Make bid: Invalid response from server.\n");
        return;
    }

    if (!strcmp(buffer + 4, "ACC\n")) {
        printf("Bid accepted.\n");
    }
    else if (!strcmp(buffer + 4, "NLG\n")) {
        printf("User is not logged in.\n");
    }
    else if (!strcmp(buffer + 4, "NOK\n")) {
        printf("Auction %s is not active\n", auction_id);
    }
    else if (!strcmp(buffer + 4, "REF\n")) {
        printf("Auction rejected. There has already been placed a larger bid\n");
    }
    else if (!strcmp(buffer + 4, "ILG\n")) {
        printf("You can't bid your own auction\n");
    }
    else {
        printf("%s", buffer);
        printf("Invalid response from server.\n");
    }
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        exitScript();
    }
    exitScript();
}

int main(int argc, char *argv[]) {
    char prompt[MAX_PROMPT_SIZE];
    char prompt_args[MAX_PROMPT_NUMBER][MEDIUM_BUFFER];
    int prompt_args_count;

    // Set the parameters of the server according to the program's arguments
    setServerParameters(argc, argv);
    setup_UDP();

    // Set the signal handler for SIGINT
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("Error setting signal handler for SIGINT.\n");
        return -1;
    }

    memset(userID, 0, sizeof userID);
    memset(userPasswd, 0, sizeof userPasswd);
    
    while (1) {
        printf("> ");
        fgets(prompt, sizeof prompt, stdin);
        prompt_args_count = promptToArgsList(prompt, prompt_args);

        if(!strcmp(prompt_args[0], "login")) {
            clientLogin(prompt_args_count, prompt_args);
        } else if (!strcmp(prompt_args[0], "logout")) {
            userLogout();
        } else if (!strcmp(prompt_args[0], "unregister")) {
            clientUnregister(prompt_args_count);
        } else if (!strcmp(prompt_args[0], "exit")) {
            if (exitScript()) {
                break;
            }
        } else if (!strcmp(prompt_args[0], "open")) {
            openAuction(prompt_args_count, prompt_args);
        } else if (!strcmp(prompt_args[0], "close")) {
            closeAuction(prompt_args_count, prompt_args);
        } else if (!strcmp(prompt_args[0], "myauctions") || !strcmp(prompt_args[0], "ma")) {
            listMyAuctions(prompt_args_count);
        } else if (!strcmp(prompt_args[0], "mybids") || !strcmp(prompt_args[0], "mb")) {
            myBids(prompt_args_count);
        } else if (!strcmp(prompt_args[0], "list") || !strcmp(prompt_args[0], "l")) {
            listAllAuctions(prompt_args_count);
        } else if (!strcmp(prompt_args[0], "show_asset") || !strcmp(prompt_args[0], "sa")) {
            showAsset(prompt_args_count, prompt_args);
        } else if (!strcmp(prompt_args[0], "bid") || !strcmp(prompt_args[0], "b")) {
            makeBid(prompt_args_count, prompt_args);
        } else if (!strcmp(prompt_args[0], "show_record") || !strcmp(prompt_args[0], "sr")) {
            showRecord(prompt_args_count, prompt_args);
        } else {
            printf("Command not found.\n");
        }

    }
    printf("finish\n");
}

