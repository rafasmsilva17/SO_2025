CC=gcc
CFLAGS=-Wall -Wextra

CLIENT_SRC=client/dclient.c client/client_utils.c
SERVER_SRC=server/dserver.c server/document.c server/server_utils.c server/cache.c

CLIENT_HDR = client/client_utils.h
SERVER_HDR = server/server_utils.h server/document.h server/cache.h

CLIENT_BIN=bin/dclient
SERVER_BIN=bin/dserver

SCRIPTS_DIR=scripts_teste

all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC) $(CLIENT_HDR)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRC)

$(SERVER_BIN): $(SERVER_SRC) $(SERVER_HDR)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC)

clean:
	rm -rf bin pipes/client_*_fifo resultados_teste/resultados_paralelo.txt resultados_teste/resultados_cache.txt

test_cache: $(SERVER_BIN) $(CLIENT_BIN)
	@echo "Executando teste de cache..."
	chmod +x scripts_teste/teste_cache.sh
	scripts_teste/teste_cache.sh

test_paralelo: $(SERVER_BIN) $(CLIENT_BIN)
	@echo "Executando teste de paralelismo..."
	chmod +x scripts_teste/teste_paralelo.sh
	scripts_teste/teste_paralelo.sh

