CC=gcc
CFLAGS=-ggdb -Wall
LDFLAGS=-lpthread

HOST = "127.0.0.1"
PORT = 5555
THREADS = 1
QUEUE_SIZE = 1

LOGGING += netpack.o
LOGGING += client.o
LOGGING += server.o runner.o connection.o threadpool.o queue.o
$(LOGGING): CFLAGS += -DLOGGING

CLIENT = client
SERVER = server

# TODO: Error codes


.SILENT: help
.PHONY: all help clean

all: $(CLIENT) $(SERVER)

help:
	echo "Available targets:"
	echo "- all"
	echo "- $(CLIENT)"
	echo "- $(SERVER)"

clean:
	rm -f *.o $(CLIENT) $(SERVER)

# ================================================================================
# Common:
# ================================================================================
COMMON=netpack.o logging.o

netpack.o: netpack.c
logging.o: logging.c

# ================================================================================
# Client:
# ================================================================================

$(CLIENT): client.o $(COMMON)
client.o: CFLAGS += -DHOST='$(HOST)' -DPORT=$(PORT)
client.o: client.c

# ================================================================================
# Server:
# ================================================================================

$(SERVER): server.o runner.o connection.o threadpool.o queue.o $(COMMON)
server.o: CFLAGS += -DHOST='$(HOST)' -DPORT=$(PORT) -DTHREADS=$(THREADS) -DQUEUE_SIZE=$(QUEUE_SIZE)
server.o: server.c connection.c logging.c
runner.o: runner.c logging.c
conenction.o: connection.c runner.c logging.c
threadpool.o: threadpool.c queue.c
queue.o: queue.c


