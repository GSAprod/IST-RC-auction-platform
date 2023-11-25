#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "client_connections.h"

#define DEFAULT_SERVER_IP "localhost"
#define DEFAULT_SERVER_PORT 58057   // 58000 + Group #57

int logout(char* username, char* password) {
    char buffer[128];
    
    sprintf(buffer, "LOU %s %s\n", username, password);
    // Send the logout command to the server
    udp_send(buffer);

    // Receive the response from the server
    udp_receive(buffer, sizeof buffer);

    // Check if the response is correct
    char * token = strtok(buffer, " ");

    if (strcmp(token, "RLO")) {
        printf("Error: Invalid response from server.\n");
        return -1;
    }
    token = strtok(NULL, " ");

    if (!strcmp(token, "OK")) {
        printf("successful logout\n");
        return 0;
    } else if (!strcmp(token, "NOK")) {
        printf("user not logged in\n");
        return 1;
    } else if (!strcmp(token, "UNR")) {
        printf("unknown user\n");
        return 1;
    }

    return -1;
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
        // TODO Login function
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