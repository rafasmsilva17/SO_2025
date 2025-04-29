CC=gcc
CFLAGS=-Wall -Wextra

CLIENT_SRC=client/dclient.c client/client_utils.c
SERVER_SRC=server/dserver.c server/document.c server/server_utils.c

CLIENT_HDR = client/client_utils.h
SERVER_HDR = server/server_utils.h server/document.h

CLIENT_BIN=bin/dclient
SERVER_BIN=bin/dserver

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC) $(CLIENT_HDR)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRC)

$(SERVER_BIN): $(SERVER_SRC) $(SERVER_HDR)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC)

clean:
	rm -rf bin pipes/client_*_fifo
