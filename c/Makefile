CC=gcc
CFLAGS=-ggdb -Wall
LDFLAGS=

SOURCES=main.c logging.c server.c connection.c runner.c
OBJECTS=$(patsubst %.c,%.o, $(SOURCES))


exe: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

main.o: main.c logging.c
	$(CC) $(CFLAGS) -c $< -o $@

runner.o: runner.c logging.c
	$(CC) $(CFLAGS) -c $< -o $@

conenction.o: connection.c runner.c logging.c
	$(CC) $(CFLAGS) -c $< -o $@

server.o: server.c connection.c logging.c
	$(CC) $(CFLAGS) -c $< -o $@

logging.o: logging.c
	$(CC) $(CFLAGS) -c $< -o $@

client: client.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o exe client
