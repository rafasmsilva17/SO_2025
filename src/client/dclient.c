#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>  // Incluído para resolver o erro com 'errno'
#include "client_utils.h"

#define MAX_MSG 512

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: dclient -a/-c/-d/-l/-s ...\n");
        exit(1);
    }

    pid_t pid = getpid();
    char client_fifo[64];
    snprintf(client_fifo, sizeof(client_fifo), "../pipes/client_%d_fifo", pid);
    mkfifo(client_fifo, 0666);

    char message[MAX_MSG] = {0};

    if (strcmp(argv[1], "-a") == 0 && argc == 6) {
        if (access(argv[5], F_OK) != 0) {
            fprintf(stderr, "Erro: O ficheiro '%s' não existe.\n", argv[5]);
            unlink(client_fifo);
            exit(1);
        }

        snprintf(message, sizeof(message), "ADD|%s|%s|%s|%s|%s",
                 argv[2], argv[3], argv[4], argv[5], client_fifo);
    }
    else if (strcmp(argv[1], "-c") == 0 && argc == 3) {
        snprintf(message, sizeof(message), "CONSULT|%s|%s", argv[2], client_fifo);
    }
    else if (strcmp(argv[1], "-d") == 0 && argc == 3) {
        snprintf(message, sizeof(message), "DELETE|%s|%s", argv[2], client_fifo);
    }
    else if (strcmp(argv[1], "-l") == 0 && argc == 4) {
        snprintf(message, sizeof(message), "LINES|%s|%s|%s", argv[2], argv[3], client_fifo);
    }
    else if (strcmp(argv[1], "-s") == 0 && argc == 3) {
        snprintf(message, sizeof(message), "SEARCH|%s|%s", argv[2], client_fifo);
    }
    else {
        fprintf(stderr, "Comando inválido ou argumentos insuficientes.\n");
        unlink(client_fifo);
        exit(1);
    }

    send_request(message, client_fifo);

    unlink(client_fifo);
    return 0;
}
