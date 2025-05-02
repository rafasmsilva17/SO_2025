#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "server_utils.h"
#include "document.h"

#define MAX_MSG 512
#define SERVER_FIFO "../pipes/server_fifo"

int main() {
    // Inicialização do servidor
    unlink(SERVER_FIFO);
    if (mkfifo(SERVER_FIFO, 0666) < 0) {
        perror("Erro ao criar FIFO do servidor");
        exit(1);
    }

    // Persistência e cache
    //load_metadata_from_file();
    //init_cache(20); // cache de 20 entradas (ou configurable)

    printf("Servidor aguardando no FIFO...\n");
    int save = 1; //nao esta guardado no ficheiro
    char buffer[MAX_MSG];

    while (1) {
        int fd = open(SERVER_FIFO, O_RDONLY);
        if (fd < 0) {
            perror("Erro ao abrir FIFO do servidor");
            continue;
        }

        int n = read(fd, buffer, MAX_MSG - 1);
        if (n < 0) {
            perror("Erro ao ler do FIFO");
            close(fd);
            continue;
        }
        buffer[n] = '\0';
        close(fd);

        // parse mensagem
        char *args[8];
        int i = 0;
        char *token = strtok(buffer, "|");
        while (token && i < 8) {
            args[i++] = token;
            token = strtok(NULL, "|");
        }

        if (i < 2) continue; // inválido
        char *op = args[0];
        char *client_fifo = args[i - 1];

        if (strcmp(op, "-a") == 0 && i == 6) {
            handle_add(args, client_fifo);
        } else if (strcmp(op, "-c") == 0 && i == 3) {
            handle_consult(args[1], client_fifo);
        } else if (strcmp(op, "-d") == 0 && i == 3) {
            handle_delete(args[1], client_fifo);
        } else if (strcmp(op, "-l") == 0 && i == 4) {
            handle_lines(args[1], args[2], client_fifo);
        } else if (strcmp(op, "-s") == 0 && (i == 3 || i == 4)) {
            if (i == 3) {
                handle_search(args[1], NULL, client_fifo); // sem limite de processos
            } else {
                handle_search(args[1], args[2], client_fifo); // com limite
            }

        } else if (strcmp(op, "-f") == 0 && i == 2) {
            //save_metadata_to_file();
            save = 0; //guarda no ficheiro
            send_response(client_fifo, "Server is shuting down\n");
            break;
        } else {
            send_response(client_fifo, "ERROR|Comando inválido ou mal formatado.\n");
        }
    }
    /*
    // Encerramento
    if(save != 0){
        save_metadata_to_file();
    }
    free_cache();
    */
    return 0;
}
