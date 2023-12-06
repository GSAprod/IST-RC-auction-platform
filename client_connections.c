#include "client_connections.h"

int udp_fd, tcp_fd;
struct addrinfo *udp_info, *tcp_info; 

char server_ip[256];
char server_port[16];


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
void setServerParameters(int argc, char *argv[]) {
    int port_int;
    int ip_used = 0, port_used = 0; // These variables control if a param is used twice

    strcpy(server_ip, DEFAULT_SERVER_IP);
    strcpy(server_port, DEFAULT_SERVER_PORT);

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
            strcpy(server_ip, argv[i + 1]);
            ip_used = 1;
        } else if (!strcmp(argv[i], "-p")) { // Check for a port argument
            port_int = atoi(argv[i + 1]);

            // If the port is invalid, throw an error
            if(port_used == 1 || port_int <= 0 || port_int > 65535) {
                printf("Wrong arguments given.\n%s [-n ASIP] [-p ASport]\n", argv[0]);
                exit(EXIT_FAILURE);
            }

            strcpy(server_port, argv[i + 1]);
            port_used = 1;
        }
    }
}


/***
 * Sets up the UDP socket connections by setting the global descriptor udp_fd, 
 * and the addrinfo struct udp_info.
 * 
 * @return 0 if the setup is successful, -1 otherwise
*/
int setup_UDP() {
    int errcode;
    struct addrinfo udp_hints;

    // UDP socket
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (udp_fd == -1) return -1;
    
    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_INET;      // IPv4
    udp_hints.ai_socktype = SOCK_DGRAM; // UDP socket
    
    errcode = getaddrinfo(server_ip, server_port, &udp_hints, &udp_info);
    if (errcode != 0) return -1;

    return 0;
}

/***
 * Sets up the TCP socket connections by setting the global descriptor tcp_fd, 
 * and the addrinfo struct tcp_info.
 * 
 * @return 0 if the setup is successful, -1 otherwise
*/
int setup_TCP() {
    int errcode;
    struct addrinfo tcp_hints;

    // TCP socket
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (tcp_fd == -1) return -1;

    memset(&tcp_hints, 0, sizeof tcp_hints);
    tcp_hints.ai_family = AF_INET;      // IPv4
    tcp_hints.ai_socktype = SOCK_STREAM; // UDP socket
    
    errcode = getaddrinfo(server_ip, server_port, &tcp_hints, &tcp_info);
    if (errcode != 0) return -1;

    return 0;
}

/***
 * Frees up the UDP socket information structure. This function should be used
 * as the program closes.
*/
void UDP_free() {
    freeaddrinfo(udp_info);
}

/***
 * Frees up the TCP socket information structures. This function should be used
 * as the program closes.
*/
void TCP_free() {
    freeaddrinfo(tcp_info);
}


/***
 * Sends a message using the UDP connection protocol, with the parameters
 * established in the variables udp_fd and udp_info.
 * 
 * @param message The message to be sent
 * 
 * @return 0 if the mesage was sent, -1 if an error occurs 
 * while sending the message
*/
int udp_send(char* message) {
    int n;
    n = sendto(udp_fd, message, strlen(message), 0, udp_info->ai_addr, udp_info->ai_addrlen);
    if (n == -1) return -1;

    return 0;
}

/***
 * Reads a certain number of bytes from the UDP socket.
 * 
 * @param dest The string where the received message is stored
 * @param max_len The maximum number of characters to be read
 * @note The number of characters received may be smaller than specified
 * 
 * @return 0 if the message is received correctly, -1 otherwise
*/
int udp_receive(char* dest, int max_len) {
    socklen_t addrlen;
    struct sockaddr_in addr;
    int n;

    addrlen = sizeof(addr);
    n = recvfrom(udp_fd, dest, max_len, 0,
                 (struct sockaddr *)&addr, &addrlen);
    if (n == -1) return -1;

    return 0;
}

/***
 * Creates a connection onto the TCP socket
 * 
 * @return 0 if the connection is established, -1 if there was an error
 * connecting
*/
int tcp_connect() {
    int n;
    n=connect(tcp_fd, tcp_info->ai_addr, tcp_info->ai_addrlen);
    if(n==-1) return -1;

    return 0;
}

/***
 * Sends a message using the TCp connection protocol, with the parameters
 * established in the variables udp_fd and udp_info.
 * 
 * @param message The message to be sent
 * @param message_len The length of the message that is sent
 * 
 * @return 0 if the mesage was sent, -1 if an error occurs 
 * while sending the message
*/
int tcp_send(char* message, int message_len) {
    int bytesLeft, bytesWritten;
    char* ptr = message;

    bytesLeft=message_len;
    while(bytesLeft > 0) {
        bytesWritten = write(tcp_fd, ptr, bytesLeft);
        if(bytesWritten <= 0) return -1;
        bytesLeft -= bytesWritten;
        ptr += bytesWritten;
    }

    return 0;
}

/***
 * Reads a certain number of bytes from the TCP socket.
 * 
 * @param dest The string where the received message is stored
 * @param max_len The maximum number of characters to be read
 * @note The number of characters received may be smaller than specified
 * 
 * @return the length of the string received if the message is received correctly, -1 otherwise
*/
int tcp_receive(char* dest, int max_len) {
    int bytesLeft, bytesRead;
    char* ptr = dest;

    bytesLeft = max_len;
    while(bytesLeft > 0) {
        bytesRead = read(tcp_fd, ptr, bytesLeft);
        if(bytesRead == -1) {
            if (errno == ECONNRESET) return 0;
            return -1;
        }
        else if (bytesRead == 0) break;
        bytesLeft -= bytesRead;
        ptr += bytesRead;
    }

    return max_len - bytesLeft;
}

/***
 * Closes the TCP connection.
 * 
 * @returns 0 if the connection is closed properly, other
 * values otherwise
*/
int tcp_close() {
    return close(tcp_fd);
}