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

#define CACHE_SIZE 10  // temos que mudar para enviar como argumento
typedef struct {
    char key[MAX_KEY];      
    char value[MAX_VALUE];  
    int used;               
} CacheEntry;

CacheEntry cache[CACHE_SIZE];
int cache_count = 0;


//função de porcurar na cache
char* get_from_cache(const char* key) {
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(cache[i].key, key) == 0) {
            cache[i].used++;
            return cache[i].value;
        }
    }
    return NULL;  
}

void add_to_cache(const char* key, const char* value) {
    if (cache_count < cache_size) {
        
        strcpy(cache[cache_count].key, key);
        strcpy(cache[cache_count].value, value);
        cache[cache_count].used = 1;
        cache_count++;
    } else {
        
        int min_index = 0;
        for (int i = 1; i < cache_count; i++) {
            if (cache[i].used < cache[min_index].used) {
                min_index = i;
            }
        }
        strcpy(cache[min_index].key, key);
        strcpy(cache[min_index].value, value);
        cache[min_index].used = 1;
    }
}



int main() {
    unlink(SERVER_FIFO);
    if (mkfifo(SERVER_FIFO, 0666) < 0) {
        perror("Erro ao criar FIFO do servidor");
        exit(1);
    }
    printf("Servidor aguardando no FIFO...\n");

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
        char *client_fifo = args[i-1];

        if (strcmp(op, "ADD") == 0 && i == 6) {
            handle_add(args, client_fifo);
        } else if (strcmp(op, "CONSULT") == 0 && i == 3) {
            handle_consult(args[1], client_fifo);
        } else if (strcmp(op, "DELETE") == 0 && i == 3) {
            handle_delete(args[1], client_fifo);
        } else if (strcmp(op, "LINES") == 0 && i == 4) {
            handle_lines(args[1], args[2], client_fifo);
        } else if (strcmp(op, "SEARCH") == 0 && i == 3) {
            handle_search(args[1], client_fifo);
        } else {
            send_response(client_fifo, "ERROR|Comando inválido ou mal formatado.\n");
        }
    }

    return 0;
}
