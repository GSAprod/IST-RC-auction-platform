#include "file_handling.h"

int checkAssetFile(char * filename) {
	struct stat filestat;
	int ret_stat = stat(filename, &filestat);

	if (ret_stat == -1) {
		return 0;
	}

	if (filestat.st_size == 0) {
		return -1;
	}

	printf("File size: %ld\n", filestat.st_size);

	return filestat.st_size;
}

int sendFile(char * filename, long fsize) {
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		return -1;
	}
	while (fsize > 0) {
		char buffer[512];
		int read_size = read(fd, buffer, fsize > 512 ? 512 : fsize);
		if (read_size == -1) {
			return -1;
		}
		fsize -= read_size;
		tcp_send(buffer, read_size);
	}
	
	close(fd);

	return 0;
}

int receiveFile(char * filename, long fsize, char * beginning_bytes, int beginning_bytes_size) {
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd == -1) {
		return -1;
	}

	while (beginning_bytes_size > 0) {
		int written_size = write(fd,beginning_bytes, beginning_bytes_size);
		if (written_size == -1) {
			return -1;
		}
		beginning_bytes_size -= written_size;
		fsize -= written_size;
	}

	while (fsize > 0) {
		char buffer[512];
		int read_size = tcp_receive(buffer, fsize > 512 ? 512 : fsize);
		printf("read_size: %d\n", read_size);
		if (read_size == -1) {
			return -1;
		}
		fsize -= read_size;
		write(fd, buffer, read_size);
	}
	
	close(fd);

	return 0;
}

int ServerReceiveFile(char * filename, long fsize, int socket_fd, char * beginning_bytes, int beginning_bytes_size) {
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd == -1) {
		return -1;
	}

	while (beginning_bytes_size > 0) {
		int written_size = write(fd,beginning_bytes, beginning_bytes_size);
		if (written_size == -1) {
			return -1;
		}
		beginning_bytes_size -= written_size;
		fsize -= written_size;
	}

	while (fsize > 0) {
		char buffer[512];
		int read_size = server_tcp_receive(socket_fd,buffer, fsize > 512 ? 512 : fsize);
		printf("read_size: %d\n", read_size);
		if (read_size == -1) {
			return -1;
		}
		fsize -= read_size;
		write(fd, buffer, read_size);
	}
	
	close(fd);

	return 0;
}