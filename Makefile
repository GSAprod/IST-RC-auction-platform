CC=gcc
CFLAGS=-Wall -Wextra -g

all:
	$(CC) $(CFLAGS) utils.c file_handling.c client_connections.c auction_client.c -o auction_client
  
	$(CC) $(CFLAGS) utils.c file_handling.c database_handling.c client_connections.c auction_server.c -o auction_server
clean:
	rm ./auction_client ./auction_server