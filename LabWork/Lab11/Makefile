all: client server

CFLAGS += -Wall -g

client: client.c common.h
	$(CC) $(LDFLAGS) $(CFLAGS) client.c -o client

server: server.c common.h
	$(CC) $(LDFLAGS) $(CFLAGS) server.c -o server

clean:
	rm -rf client server
