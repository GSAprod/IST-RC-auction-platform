CC=gcc
CFLAGS=-Wall -Wextra -g

all:
	$(CC) $(CFLAGS) utils.c file_handling.c client_connections.c auction_client.c -o client/user
  
	$(CC) $(CFLAGS) utils.c file_handling.c database_handling.c client_connections.c auction_server.c -o ASDIR/AS
clean:
	rm ./client/user ./ASDIR/AS