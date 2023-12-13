#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "client_connections.h"
#include "utils.h"


/*
 * Check if the file exists and is not empty.
 * 
 * @param filename The name of the file to check.
 * @return 0 if the file does not exist or is empty, otherwise the size of the file.
*/
int checkAssetFile(char * filename);

int sendFile(char * filename, long fsize);

int serverSendFile(int fd, long fsize, int socket_fd);

int receiveFile(char * filename, long fsize, char * beginning_bytes, int beginning_bytes_size);

int ServerReceiveFile(char * filename, long fsize, int socket_fd);

#endif
