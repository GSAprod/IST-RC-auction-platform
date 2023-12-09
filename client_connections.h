#ifndef CLIENT_CONNECTIONS_H
#define CLIENT_CONNECTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define DEFAULT_SERVER_IP "localhost"
#define DEFAULT_SERVER_PORT "58057"   // 58000 + Group #57

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
 * Sets up the UDP socket connections by setting the global descriptor udp_fd, 
 * and the addrinfo struct udp_info.
 * 
 * @return 0 if the setup is successful, -1 otherwise
*/
int setup_UDP();

/***
 * Sets up the UDP socket connections for the server by setting the global descriptor udp_fd.
 * and the addrinfo struct udp_info.
 * 
 * @param port The port the server is using for receiving messages
 * @return udp_fd if the setup is successful, -1 otherwise
*/
int server_setup_UDP(char* port);

/***
 * Sets up the TCP socket connections by setting the global descriptor tcp_fd, 
 * and the addrinfo struct tcp_info.
 * 
 * @return 0 if the setup is successful, -1 otherwise
*/
int setup_TCP();

/***
 * Sets up the TCP socket connections for a server by setting 
 * the global descriptor tcp_fd, and the addrinfo struct tcp_info.
 * 
 * @return tcp_fd if the setup is successful, -1 otherwise
*/
int server_setup_TCP(char* port);

/***
 * Frees up the UDP socket information structure. This function should be used
 * as the program closes.
*/
void UDP_free();

/***
 * Frees up the TCP socket information structures. This function should be used
 * as the program closes.
*/
void TCP_free();

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
 * Sends a message using the UDP connection protocol to a specific address, 
 * using the socket established in the variable udp_fd. 
 * 
 * @param message The message to be sent
 * @param to_addr The address where the message should be sent to
 * @param to_addr_len The length of the address where the message is sent
 * 
 * @return 0 if the mesage was sent, -1 if an error occurs 
 * while sending the message
*/
int server_udp_send(char* message, struct sockaddr* to_addr, socklen_t to_addr_len);

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
 * Reads a certain number of bytes from the UDP socket and stores the address of the sender.
 * 
 * @param dest The string where the received message is stored
 * @param max_len The maximum number of characters to be read
 * @param from_addr The struct where the address of the sender is stored
 * @param from_addr_len The struct where the length of the address of the sender is stored
 * @note The number of characters received may be smaller than specified
 * 
 * @return 0 if the message is received correctly, -1 otherwise
*/
int server_udp_receive(char* dest, int max_len, struct sockaddr* from_addr,
        socklen_t* from_addr_len);

/***
 * Creates a connection onto the TCP socket
 * 
 * @return 0 if the connection is established, -1 if there was an error
 * connecting
*/
int tcp_connect();

/***
 * Accepts a TCP connection and returns the corresponding socket.
 * 
 * @return the file descriptor of the socket if the connection
 * is successful, -1 otherwise
*/
int server_tcp_accept();

/***
 * Closes a TCP socket connection. This function is only used for abstraction purposes.
 * 
 * @param socket_fd The file descriptor of the socket that will be closed
*/
void server_tcp_close(int socket_fd);

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
int tcp_send(char* message, int message_len);

/***
 * Sends a message using the TCP connection protocol, to a certain socket
 * described in its corresponding file descriptor
 * 
 * @param socket_fd The file descriptor of the socket where the message is sent to
 * @param message The message to be sent
 * @param message_len The length of the message that is sent
 * 
 * @return 0 if the mesage was sent, -1 if an error occurs 
 * while sending the message
*/
int server_tcp_send(int socket_fd, char* message, int message_len);

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

/***
 * Reads a certain number of bytes from the TCP socket specified in the corresponding
 * file descriptor.
 * 
 * @param socket_fd The socket where the message is received
 * @param dest The string where the received message is stored
 * @param max_len The maximum number of characters to be read
 * @note The number of characters received may be smaller than specified
 * 
 * @return the length of the string received if the message is received correctly, -1 otherwise
*/
int server_tcp_receive(int socket_fd, char* dest, int max_len);

#endif