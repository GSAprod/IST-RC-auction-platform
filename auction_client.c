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
    int istId;
    int passwdLen;

    // Argument verification
    if(arg_count != 3) {
        printf("Login: Wrong arguments given.\nlogin UID password\n");
        return;
    }

    // Verify if the istID is a 6 digit number
    scanf_success = sscanf(args[1], "%d", &istId);
    if (scanf_success != 1 || strlen(args[1]) != 6) {
        printf("Login: Wrong arguments given.\nlogin UID password\n");
        return;
    }
    
    // Verify if the student password is an 8 character alphanumeric string
    passwdLen = strlen(args[2]);
    if (passwdLen != 8) {
        printf("Login: Wrong arguments given.\nlogin UID password\n");
        return;
    }
    for(int i = 0; i < passwdLen; i++) {
        char c = args[2][i];
        if (!isalpha(c) && !isdigit(c)) {
            printf("Login: Wrong arguments given.\nlogin UID password\n");
            return;
        }
    }

    //* Now connect to the server and send the credentials.
    //* Receive the arguments and act accordingly
}

int main(int argc, char *argv[]) {
    char prompt[512];
    char prompt_args[16][128];
    int prompt_args_count;

    // Set the parameters of the server according to the program's arguments
    setServerParameters(argc, argv);
    
    //? Might need a while loop from here on
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
        // TODO Logout function
    } else if (!strcmp(prompt_args[0], "unregister")) {
        // TODO Unregister function
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
        // TODO List function
    } else if (!strcmp(prompt_args[0], "show_asset") || !strcmp(prompt_args[0], "sa")) {
        // TODO Show_asset function
    } else if (!strcmp(prompt_args[0], "bid") || !strcmp(prompt_args[0], "b")) {
        // TODO My_auctions function
    } else if (!strcmp(prompt_args[0], "show_record") || !strcmp(prompt_args[0], "sr")) {
        // TODO My_auctions function
    } else {
        printf("Command not found.\n");
    }

    printf("finish\n");
}