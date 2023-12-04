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
#include "client_connections.h"

// User credentials (they can be used in many requests to the server)
char userID[6], userPasswd[8];

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
    token = strtok(NULL, " ");

    if (!strcmp(token, "OK\n")) {
        printf("successful logout\n");
    } else if (!strcmp(token, "NOK\n")) {
        printf("user not logged in\n");
    } else if (!strcmp(token, "UNR\n")) {
        printf("unknown user\n");
        return;
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
 * Lists all auctions of the server and their respective status.
 * 
 * @param arg_count The number of arguments of the prompt
 */
void listAuctions(int arg_count) {
    char buffer[8192], aux[16];
    char *token;
    int status, auction_num, status_num, has_entries = 0;

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
        // Print the list of auctions
        token = strtok(buffer + 7, " ");
        while (token != NULL) {
            // Check if the next substring is a 3-digit auction number 
            status = sscanf(token, "%03d", &auction_num);
            if (status == EOF) {
                printf("List all auctions: Invalid response from server.\n");
                return;
            }

            // Get the next substring and check if it's the status number.
            token = strtok(NULL, " ");
            if(token == NULL) {
                printf("List all auctions: Invalid response from server.\n");
                return;
            }
            status = sscanf(token, "%01d", &status_num);
            if (status == EOF) {
                printf("List all auctions: Invalid response from server.\n");
                return;
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
            printf("No auctions have yet been started.\n");
        }
    } else {
        // If a different status (other than OK and NOK) is sent by the server,
        // send an error
        printf("List all auctions: failed to receive response from server.\n");
        return;
    }
}

int main(int argc, char *argv[]) {
    char prompt[512];
    char prompt_args[16][128];
    int prompt_args_count;

    // Set the parameters of the server according to the program's arguments
    setServerParameters(argc, argv);
    socket_setup();

    memset(userID, 0, sizeof userID);
    memset(userPasswd, 0, sizeof userPasswd);
    
    while (1) {
        printf("> ");
        fgets(prompt, sizeof prompt, stdin);
        prompt_args_count = promptToArgsList(prompt, prompt_args);

        //? From here on, we should issue the routines for each operation.
        //? We can define these routines like this: void login(int arg_count, char args[][128])
        //? Do not forget to check if the arguments (and count) are correct
        //! This if chain supposedly works, but it hasn't been tested (the rest was)
        if(!strcmp(prompt_args[0], "login")) {
            clientLogin(prompt_args_count, prompt_args);
        } else if (!strcmp(prompt_args[0], "logout")) {
            userLogout();
        } else if (!strcmp(prompt_args[0], "unregister")) {
            clientUnregister(prompt_args_count);
        } else if (!strcmp(prompt_args[0], "exit")) {
            // TODO Exit function
        } else if (!strcmp(prompt_args[0], "open")) {
            // TODO Open function
        } else if (!strcmp(prompt_args[0], "close")) {
            // TODO Close function
        } else if (!strcmp(prompt_args[0], "myauctions") || !strcmp(prompt_args[0], "ma")) {
            // TODO My_auctions function
        } else if (!strcmp(prompt_args[0], "mybids") || !strcmp(prompt_args[0], "mb")) {
            // TODO My_bids function
        } else if (!strcmp(prompt_args[0], "list") || !strcmp(prompt_args[0], "l")) {
            listAuctions(prompt_args_count);
        } else if (!strcmp(prompt_args[0], "show_asset") || !strcmp(prompt_args[0], "sa")) {
            // TODO Show_asset function
        } else if (!strcmp(prompt_args[0], "bid") || !strcmp(prompt_args[0], "b")) {
            // TODO My_auctions function
        } else if (!strcmp(prompt_args[0], "show_record") || !strcmp(prompt_args[0], "sr")) {
            // TODO My_auctions function
        } else {
            printf("Command not found.\n");
        }

    }
        printf("finish\n");
        socket_free();
}

