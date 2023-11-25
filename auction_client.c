#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DEFAULT_SERVER_IP "localhost"
#define DEFAULT_SERVER_PORT 58057   // 58000 + Group #57

//! Não faço a menor ideia de como abstrair isto, mas por agora fica aqui como var global
int fd;
struct addrinfo *res;

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

int sendUDPRequest(char * request, char * response) {
    int n;
    struct sockaddr_in addr;
    socklen_t addrlen;

    n = strlen(request);

    while (n > 0) {
        n -= sendto(fd, request, n, 0, res->ai_addr, res->ai_addrlen);

        if (n == -1) {
            exit(1);
        }
    }

    addrlen = sizeof(addr);

    while (n = n=recvfrom(fd,response,128,0, (struct sockaddr*)&addr,&addrlen));

    if(n==-1) /*error*/ exit(1);

    return;
}

int main(int argc, char *argv[]) {
    char server_ip[256];
    int server_port;
    char prompt[512];
    char prompt_args[16][128];
    int prompt_args_count;

    // Set the parameters of the server according to the program's arguments
    setServerParameters(argc, argv, server_ip, &server_port);

    fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
    if(fd==-1) /*error*/exit(1);

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_DGRAM; //UDP socket

    errcode=getaddrinfo(server_ip,server_port,&hints,&res);
    if(errcode!=0) /*error*/ exit(1);
    
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