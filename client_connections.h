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

int udp_fd, tcp_fd;
struct addrinfo *udp_info, *tcp_info; 

/***
 * Sets up the TCP and UDP socket connections by setting, respectively,
 * the global descriptors udp_fd and tcp_fd, and the addrinfo structs udp_info and
 * tcp_info.
 * 
 * @param ip A string containing the auction server's IP
 * @param port A string containing the auction server's active port number
 * 
 * @return 0 if the setup is successful, -1 otherwise
*/
int socket_setup(char* ip, char* port);

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
 * @param dest The string where the received message is stored
 * @param max_len The maximum number of characters to be read
 * @note The number of characters received may be smaller than specified
 * 
 * @return 0 if the mesage was sent, -1 if an error occurs 
 * while sending the message
*/
int udp_request(char* req, char* res, int max_len);


/***
 * Sends a message using the TCp connection protocol, with the parameters
 * established in the variables udp_fd and udp_info.
 * 
 * @param message The message to be sent
 * @param dest The string where the received message is stored
 * @param max_len The maximum number of characters to be read
 * 
 * @return 0 if the mesage was sent, -1 if an error occurs 
 * while sending the message
*/
int tcp_send(char* message, char* dest, int max_len);


#endif