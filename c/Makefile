CC=gcc
CFLAGS=-ggdb -Wall
LDFLAGS=-lpthread

# TODO - should be configurable from here
# server addr, port
# socket timeout

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

# ================================================================================
# Common:
# ================================================================================
COMMON=netpack.o logging.o
netpack.o: netpack.c
logging.o: logging.c

# ================================================================================
# Client:
# ================================================================================
client: client.o $(COMMON)
client.o: client.c

# ================================================================================
# Server:
# ================================================================================
server: server.o runner.o connection.o threadpool.o queue.o $(COMMON)
server.o: server.c connection.c logging.c
runner.o: runner.c logging.c
conenction.o: connection.c runner.c logging.c
threadpool.o: threadpool.c queue.c
queue.o: queue.c


