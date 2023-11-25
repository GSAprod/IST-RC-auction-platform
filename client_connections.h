#ifndef CLIENT_CONNECTIONS_H
#define CLIENT_CONNECTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define DEFAULT_SERVER_IP "localhost"
#define DEFAULT_SERVER_PORT 58057   // 58000 + Group #57

int udp_fd, tcp_fd;
struct addrinfo *udp_info, *tcp_info; 

char server_ip[256];
int server_port;


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
void setServerParameters(int argc, char *argv[]);


/***
 * Sets up the TCP and UDP socket connections by setting, respectively,
 * the global descriptors udp_fd and tcp_fd, and the addrinfo structs udp_info and
 * tcp_info.
 * 
 * @return 0 if the setup is successful, -1 otherwise
*/
int socket_setup();

/***
 * Frees up the socket information structures. This function should be used
 * as the program closes.
*/
void socket_free();

/***
 * Sends a message using the UDP connection protocol, with the parameters
 * established in the variables udp_fd and udp_info.
 * 
 * @param message The message to be sent
 * 
 * @return 0 if the mesage was sent, -1 if an error occurs 
 * while sending the message
*/
int udp_send(char* message);

/***
 * Reads a certain number of bytes from the UDP socket.
 * 
 * @param dest The string where the received message is stored
 * @param max_len The maximum number of characters to be read
 * @note The number of characters received may be smaller than specified
 * 
 * @return 0 if the message is received correctly, -1 otherwise
*/
int udp_receive(char* dest, int max_len);

/***
 * Sends a message using the TCp connection protocol, with the parameters
 * established in the variables udp_fd and udp_info.
 * 
 * @param message The message to be sent
 * 
 * @return 0 if the mesage was sent, -1 if an error occurs 
 * while sending the message
*/
int tcp_send(char* message);

/***
 * Reads a certain number of bytes from the TCP socket.
 * 
 * @param dest The string where the received message is stored
 * @param max_len The maximum number of characters to be read
 * @note The number of characters received may be smaller than specified
 * 
 * @return 0 if the message is received correctly, -1 otherwise
*/
int tcp_receive(char* dest, int max_len);

#endif