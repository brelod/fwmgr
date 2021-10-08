CC=gcc
CFLAGS=-ggdb -Wall
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
	$(CC) $(CFLAGS) -c $< -o $@

logging.o: logging.c
	$(CC) $(CLAGS) -c $< -o $@

# ------------------------------
# Client:
# ------------------------------
CLIENT_OBJS=client.o $(COMMON_OBJS)

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS)

client.o: client.c
	$(CC) $(CFLAGS) -c $< -o $@

# ------------------------------
# Server:
# ------------------------------
server: main.o runner.o connection.o logging.o server.o
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

main.o: main.c logging.c
	$(CC) $(CFLAGS) -c $< -o $@

runner.o: runner.c logging.c
	$(CC) $(CFLAGS) -c $< -o $@

conenction.o: connection.c runner.c logging.c
	$(CC) $(CFLAGS) -c $< -o $@

server.o: server.c connection.c logging.c
	$(CC) $(CFLAGS) -c $< -o $@

