CC=gcc
CFLAGS=-Wall -Wextra

all:
	$(CC) $(CFLAGS) auction_client.c -o auction_client
	#$(CC) $(CFLAGS) auction_server.c -o auction_server

clean:
	rm ./auction_client ./auction_server