#include "client_connections.h"

int udp_fd, tcp_fd;
struct addrinfo *udp_info, *tcp_info; 

char server_ip[256];
char server_port[16];


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


int setup_UDP() {
    int errcode;
    struct addrinfo udp_hints;
    struct timeval timeout;

    // UDP socket
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (udp_fd == -1) return -1;
    
    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_INET;      // IPv4
    udp_hints.ai_socktype = SOCK_DGRAM; // UDP socket
    
    errcode = getaddrinfo(server_ip, server_port, &udp_hints, &udp_info);
    if (errcode != 0) return -1;

    // Set a timeout in case the socket is stuck on read/write
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    if(setsockopt(udp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
            sizeof timeout) < 0 ||
            setsockopt(udp_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout,
            sizeof timeout) < 0) {
        if (get_mode_verbose())
            printf("Failed to establish TCP socket timeout.\n");
        return -1;
    }

    return 0;
}

int server_setup_UDP(char* port) {
    int errcode;
    struct addrinfo udp_hints;
    
    if(atoi(port) <= 0 || atoi(port) > 65535) return -1;

    // UDP socket
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd == -1) return -1;

    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_INET;      // IPv4
    udp_hints.ai_socktype = SOCK_DGRAM; // UDP socket
    udp_hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo(NULL, port, &udp_hints, &udp_info);
    if (errcode != 0) return -1;

    errcode = bind(udp_fd, udp_info->ai_addr, udp_info->ai_addrlen);
    if (errcode == -1) return -1;

    return udp_fd;
}

int setup_TCP() {
    int errcode;
    struct addrinfo tcp_hints;
    struct timeval timeout;

    // TCP socket
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (tcp_fd == -1) return -1;

    memset(&tcp_hints, 0, sizeof tcp_hints);
    tcp_hints.ai_family = AF_INET;      // IPv4
    tcp_hints.ai_socktype = SOCK_STREAM; // UDP socket
    
    errcode = getaddrinfo(server_ip, server_port, &tcp_hints, &tcp_info);
    if (errcode != 0) return -1;

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    if(setsockopt(tcp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
            sizeof timeout) < 0 ||
            setsockopt(tcp_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout,
            sizeof timeout) < 0) {
        if (get_mode_verbose())
            printf("Failed to establish TCP socket timeout.\n");
        server_tcp_close(tcp_fd);
        return -1;
    }

    return 0;
}

int server_setup_TCP(char* port) {
    int errcode;
    struct addrinfo tcp_hints;

    // TCP socket
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (tcp_fd == -1) return -1;

    memset(&tcp_hints, 0, sizeof tcp_hints);
    tcp_hints.ai_family = AF_INET;      // IPv4
    tcp_hints.ai_socktype = SOCK_STREAM; // UDP socket
    tcp_hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo(NULL, port, &tcp_hints, &tcp_info);
    if (errcode != 0) return -1;

    errcode = bind(tcp_fd, tcp_info->ai_addr, tcp_info->ai_addrlen);
    if (errcode == -1) return -1;

    if (listen(tcp_fd, 5) == -1) return -1;

    return tcp_fd;
}

void UDP_free() {
    close(udp_fd);
    freeaddrinfo(udp_info);
}

void TCP_free() {
    close(tcp_fd);
    freeaddrinfo(tcp_info);
}


int udp_send(char* message) {
    int n;
    n = sendto(udp_fd, message, strlen(message), 0, udp_info->ai_addr, udp_info->ai_addrlen);
    if (n == -1) return -1;

    return 0;
}

int server_udp_send(char* message, struct sockaddr* to_addr, socklen_t to_addr_len) {
    int n;
    n = sendto(udp_fd, message, strlen(message), 0, to_addr, to_addr_len);
    if (n == -1) return -1;

    return 0;
}

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

int server_udp_receive(char* dest, int max_len, struct sockaddr* from_addr,
        socklen_t* from_addr_len) {
    int n;

    n = recvfrom(udp_fd, dest, max_len, 0,
                 from_addr, from_addr_len);
    if (n == -1) return -1;

    return 0;
}

int tcp_connect() {
    int n;
    n=connect(tcp_fd, tcp_info->ai_addr, tcp_info->ai_addrlen);
    if(n==-1) return -1;

    return 0;
}

int server_tcp_accept() {
    return accept(tcp_fd, NULL, NULL);
}

void server_tcp_close(int socket_fd) {
    close(socket_fd);
}

int tcp_send(char* message, int message_len) {
    // The only difference between this function and the server_tcp_send
    // function is in the socket that is used to send the message.
    // Since the client already pre-defines the socket, we can just call
    // the server function with this socket to obtain the same response
    return server_tcp_send(tcp_fd, message, message_len);
}

int server_tcp_send(int socket_fd, char* message, int message_len) {
    int bytesLeft, bytesWritten;
    char* ptr = message;

    bytesLeft=message_len;
    while(bytesLeft > 0) {
        bytesWritten = write(socket_fd, ptr, bytesLeft);
        if(bytesWritten <= 0) return -1;
        bytesLeft -= bytesWritten;
        ptr += bytesWritten;
    }

    return 0;
}

int tcp_receive(char* dest, int max_len) {
    // The only difference between this function and the server_tcp_receive
    // function is in the socket that is used to receive the message.
    // Since the client already pre-defines the socket, we can just call
    // the server function with this socket to obtain the same response
    return server_tcp_receive(tcp_fd, dest, max_len);
}

int server_tcp_receive(int socket_fd, char* dest, int max_len) {
    int bytesLeft, bytesRead;
    char* ptr = dest;

    bytesLeft = max_len;
    while(bytesLeft > 0) {
        bytesRead = read(socket_fd, ptr, bytesLeft);
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