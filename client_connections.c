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

int socket_setup(char* ip, char* port) {
    int errcode;
    struct addrinfo udp_hints, tcp_hints;

    // UDP socket
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (udp_fd == -1) return -1;
    
    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_INET;      // IPv4
    udp_hints.ai_socktype = SOCK_DGRAM; // UDP socket
    
    errcode = getaddrinfo(ip, port, &udp_hints, &udp_info);
    if (errcode != 0) return -1;

    // TCP socket
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (tcp_fd == -1) return -1;

    memset(&tcp_hints, 0, sizeof tcp_hints);
    tcp_hints.ai_family = AF_INET;      // IPv4
    tcp_hints.ai_socktype = SOCK_STREAM; // UDP socket
    
    errcode = getaddrinfo("tejo.tecnico.ulisboa.pt", port, &tcp_hints, &tcp_info);
    if (errcode != 0) return -1;

    return 0;
}

/***
 * Frees up the socket information structures. This function should be used
 * as the program closes.
*/
void socket_free() {
    freeaddrinfo(udp_info);
    freeaddrinfo(tcp_info);
}

int udp_request(char* req, char* res, int max_len) {
		socklen_t addrlen;
		struct sockaddr_in addr;
    int n;
    n = sendto(udp_fd, req, strlen(req), 0, udp_info->ai_addr, udp_info->ai_addrlen);
    if (n == -1) return -1;

		addrlen = sizeof(addr);

		n = recvfrom(udp_fd, res, max_len, 0, (struct sockaddr *)&addr, &addrlen);
		if (n == -1) return -1;

    return 0;
}


/***
 * Sends a message using the TCp connection protocol, with the parameters
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
int tcp_send(char* message, char* dest, int max_len) {
    int n;

    //! This works fine the first time, but after that, the connect function
    //! throws an error with errno set to "106 - connection already established"
    //? I think the idea is to use write only, and set the SIGPIPE signal to
    //? reconnect to the socket, instead of abruptly end the program.
    //? However, I don't know where exactly should i put the connect function
    n=connect(tcp_fd,tcp_info->ai_addr,tcp_info->ai_addrlen);
    if(n==-1) {
        printf("%d\n", errno);
    return -1;
    };

    n=write(tcp_fd, message, strlen(message));
    if(n==-1) return -1;

		n=read(tcp_fd,dest,max_len);
    if(n==-1) return -1;


    return 0;
}