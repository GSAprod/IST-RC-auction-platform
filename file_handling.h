#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "client_connections.h"
#include "utils.h"


/***
 * Check if the file exists and is not empty.
 * 
 * @param filename The name of the file to check.
 * @return 0 if the file does not exist or is empty, otherwise the size of the file.
*/
int checkAssetFile(char * filename);

/***
 * Sends a file through a TCP connection to the default socket (using the tcp_send
 * function).
 * 
 * @param filename The path of the file to send
 * @param fsize The size of the file in question
 * @return 0 if the file is sent successfully, -1 otherwise
*/
int sendFile(char * filename, long fsize);

/**
 * @brief Receives a file from client and saves it to disk.
 * 
 * This function receives a file from a client using the default tcp socket connection
 * and saves it to disk.
 * The file is received in chunks and written to the specified file.
 * 
 * @param filename The name of the file to be saved.
 * @param fsize The size of the file to be received.
 * @param socket_fd The file descriptor of the socket connection.
 * @return 0 if the file is received and saved successfully, -1 otherwise.
 */
int receiveFile(char * filename, long fsize);

/***
 * Loads a file from the disk and sends into the client using the TCP socket.
 * 
 * @param fd The descriptor of the file to be read
 * @param fsize The file size
 * @param socket_fd The socket to send the file data
 * @return 0 if the file is sent successfully, -1 otherwise
*/
int serverSendFile(int fd, long fsize, int socket_fd);

/**
 * @brief Receives a file from client and saves it to disk.
 * 
 * This function receives a file from a client using a tcp socket connection and saves it to disk.
 * The file is received in chunks and written to the specified file.
 * 
 * @param filename The name of the file to be saved.
 * @param fsize The size of the file to be received.
 * @param socket_fd The file descriptor of the socket connection.
 * @return 0 if the file is received and saved successfully, -1 otherwise.
 */
int ServerReceiveFile(char * filename, long fsize, int socket_fd);

#endif
