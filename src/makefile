CC=gcc
CFLAGS=-Wall -Wextra

CLIENT_SRC=client/dclient.c
SERVER_SRC=server/dserver.c

CLIENT_BIN=bin/dclient
SERVER_BIN=bin/dserver

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER_BIN): $(SERVER_SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf bin pipes/client_*_fifo
