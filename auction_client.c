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
#include <sys/stat.h>
#include "client_connections.h"
#include "file_handling.h"

// soulindo -> Password

// User credentials (they can be used in many requests to the server)
char userID[7], userPasswd[9];

void userLogout() {
    char buffer[128];

    // Check if the user is logged in
    if (!strcmp(userID, "") || !strcmp(userPasswd, "")) {
        printf("user not logged in\n");
        return;
    }
    
    sprintf(buffer, "LOU %s %s\n", userID, userPasswd);
    // Send the logout command to the server
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
            
        // Clear the user credentials
    } else if (!strcmp(buffer + 4, "NOK\n")) {
        printf("user not logged in\n");
    } else if (!strcmp(buffer + 4, "UNR\n")) {
        printf("unknown user\n");
    } else {
        printf("Logout: Invalid response from server.\n");
        return;
    }

    // If the function didn't return before, then the user is now successfully
    // unregistered/logged out. Let's wipe the credentials from our client as
    // they are not required anymore.
    memset(userID, 0, sizeof userID);
    memset(userPasswd, 0, sizeof userPasswd);
}

/***
 * Splits the prompt into a list of prompt arguments (similarly to argv in
 * the main function)
 * 
 * @param prompt The prompt string
 * @param prompt_args A list of strings to where the propmt arguments are going to
 * be written to
 * @return The number of arguments 
*/
int promptToArgsList(char* prompt, char prompt_args[][128]) {
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
void clientLogin(int arg_count, char args[][128]) {
    int scanf_success;
    int istId, passwdLen, status;
    char buffer[128], aux[10];

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

int exitScript() {
    if (strcmp(userID, "") || strcmp(userPasswd, "")) {
        printf("User not logged out\n");
        return 0;
    }
    else {
        printf("Exiting...\n");
        return 1;
    }
}

/***
 * Logs out and unregisters a user from the database.
 * The user credentials are stored the global variables called userId and userPasswd. 
 * 
 * @param arg_count The number of arguments of the prompt
*/
void clientUnregister(int arg_count) {
    char buffer[128], aux[16];
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
    char buffer[8192], aux[16];
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

    if (!strcmp(userID, "")) {
        printf("No user is logged in.\n");
        return;
    }

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

void closeAuction(int arg_count, char arg_values[][128]) {
    if (arg_count != 2) {
        printf("Close auction: Wrong arguments given.\n\t>close <auction_id>\n");
        return;
    }

    if (!strcmp(userID, "")) {
        printf("No user is logged in.\n");
        return;
    }

    char * auction_id = arg_values[1];

    char buffer[128];
    memset(buffer, 0, sizeof buffer);

    sprintf(buffer, "CLS %s %s %s\n", userID, userPasswd, auction_id);

    setup_TCP();
    tcp_connect();

    tcp_send(buffer, strlen(buffer));

    memset(buffer, 0, sizeof buffer);

    tcp_receive(buffer, sizeof buffer);

    TCP_free();

    printf("%s", buffer);

    char aux[4];
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

    memset(buffer, 0, 128);

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
        char fname[64];
        char fsize[16];

        // Get the name of the file
        memset(buffer, 0, 128);
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
        memset(buffer, 0, 128);
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

        receiveFile(fname, atoi(fsize));

        memset(buffer, 0, sizeof buffer);
        tcp_receive(buffer, 1);
        if (strcmp(buffer, "\n")) {
            printf("Invalid response from server\n");
            TCP_free();
            return;
        }

        TCP_free();
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
 * Lists all auctions that the logged in user (with login credentials in the
 * global variables userId and userPasswd) has created.
 * 
 * @param arg_count The number of arguments of the prompt
 */
void listMyAuctions(int arg_count) {
    char buffer[8192], aux[16];
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


void makeBid(int arg_count, char arg_vals[][128]) {
    if (arg_count != 3) {
        printf("Make bid: Wrong arguments given.\n\t>bid <auction_id> <value>\n");
        return;
    }

    if (!strcmp(userID, "")) {
        printf("No user is logged in.\n");
        return;
    }

    char * auction_id = arg_vals[1];
    char * value = arg_vals[2];

    char buffer[128];
    memset(buffer, 0, sizeof buffer);

    sprintf(buffer, "BID %s %s %s %s\n", userID, userPasswd, auction_id, value);

    setup_TCP();
    tcp_connect();

    printf("%s", buffer);
    tcp_send(buffer, strlen(buffer));

    memset(buffer, 0, sizeof buffer);

    tcp_receive(buffer, sizeof buffer);

    TCP_free();

    char aux[4];
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

/***
 * Lists all auctions that the logged in user (with login credentials in the
 * global variables userId and userPasswd) has created.
 * 
 * @param arg_count The number of arguments of the prompt
 */
void myBids(int arg_count) {
    char buffer[8192];

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

    char aux[4];
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

void showRecord(int argc, char argv[][128]) {

    void handleAuctions(char *auctions) {
        char c = 'a';
        int i = 0, j= 0;
        char buffer[1024];

        memset(buffer, 0, sizeof buffer);

        // Read auction
        printf("Auction:\n");
        while (1) {
            c = auctions[i];
            if (c == 'B' || c == '\0' || c == 'E') {
                break;
            }
            buffer[j] = c;
            i++;
            j++;
        }
        printf("%s\n", buffer);

        memset(buffer, 0, sizeof buffer);

        if (c == 'B') {
            // Read bids
            
            i+= 2;
            j= 0;

            printf("Bids:\n");
            int repeat = 1;
            while (repeat) {
                repeat = 0;
                memset(buffer, 0, sizeof buffer);
                while (1) {
                    c = auctions[i];
                    if (c == 'B') {
                        repeat = 1;
                        break;
                    }
                    if (c == 'E' || c == '\0') {
                        break;
                    }
                    buffer[j] = c;
                    i++;
                    j++;
                }
                printf("-> %s", buffer);
            }
            printf("\n");
        }

        if (c == 'E') {
            i += 2;
            j = 0;

            printf("End:\n");

            memset(buffer, 0, sizeof buffer);
            while (1) {
                c = auctions[i];
                if (c == '\0' || c == '\n') {
                    i++;
                    break;
                }
                buffer[j] = c;
                i++;
                j++;
            }
            printf("%s", buffer);
            printf("\n");
        }

        return;

    }

    if (argc != 2 || strlen(argv[1]) != 3) {
        printf("Wrong arguments given.\n\t> show_record <auction_id>\n\t> sr <auction_id>\n\t (auction_id: 3-digit number)\n");
        return;
    }

    char buffer[8192];
    memset(buffer, 0, sizeof buffer);

    sprintf(buffer, "SRC %s\n", argv[1]);

    udp_send(buffer);

    memset(buffer, 0, sizeof buffer);

    int status = udp_receive(buffer, sizeof buffer);
    if (status == -1) {
        printf("Show record: failed to receive response from server.\n");
        return;
    }

    char aux[4];
    memset(aux, 0, sizeof aux);
    strncpy(aux, buffer, 3);
    if (strcmp(aux, "RRC")) {
        printf("Show record: Invalid response from server.\n");
        return;
    }

    memset(aux, 0, sizeof aux);
    strncpy(aux, buffer + 4, 3);

    if (!strcmp(aux, "NOK")) {
        printf("Auction does not exist.\n");
        return;
    } else if (!strcmp(aux, "OK ")) {
        handleAuctions(buffer + 7);
    } else {
        printf("Show record: Invalid response from server.\n");
        return;
    }
}

int main(int argc, char *argv[]) {
    char prompt[512];
    char prompt_args[16][128];
    int prompt_args_count;

    // Set the parameters of the server according to the program's arguments
    setServerParameters(argc, argv);
    setup_UDP();

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
    UDP_free();
}

