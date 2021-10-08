CC=gcc

COMMON_CFLAGS=-ggdb -Wall
CLIENT_CFLAGS=$(COMMON_CFLAGS)
SERVER_CFLAGS=$(COMMON_CFLAGS)

LDFLAGS=


.SILENT: help
.PHONY: all help clean

all: client server

help:
	echo "Available targets:"
	echo "- all"
	echo "- server"
	echo "- client"

clean:
	rm -f *.o client server

# ------------------------------
# Common:
# ------------------------------
COMMON_OBJS=netpack.o logging.o

netpack.o: netpack.c
	$(CC) $(COMMON_CFLAGS) -c $< -o $@

logging.o: logging.c
	$(CC) $(COMMON_CFLAGS) -c $< -o $@

# ------------------------------
# Client:
# ------------------------------
CLIENT_OBJS=client.o $(COMMON_OBJS)

client: $(CLIENT_OBJS)
	$(CC) $(CLIENT_CFLAGS) -o $@ $(CLIENT_OBJS)

client.o: client.c
	$(CC) $(CLIENT_CFLAGS) -c $< -o $@

# ------------------------------
# Server:
# ------------------------------
SERVER_OBJS=$(COMMON_OBJS) server.o runner.o connection.o

server: $(SERVER_OBJS)
	$(CC) $(SERVER_CFLAGS) $(SERVER_OBJS) -o $@

server.o: server.c connection.c logging.c
	$(CC) $(SERVER_CFLAGS) -c $< -o $@

runner.o: runner.c logging.c
	$(CC) $(SERVER_CFLAGS) -c $< -o $@

conenction.o: connection.c runner.c logging.c
	$(CC) $(SERVER_CFLAGS) -c $< -o $@


