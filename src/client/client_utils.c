#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "client_utils.h"

void send_request(const char *message, const char *client_fifo) {
    int server_fd;
    while ((server_fd = open("../pipes/server_fifo", O_WRONLY)) < 0) {
        if (errno == ENOENT) {
            usleep(100000); // Espera de 100ms
        } else {
            perror("Erro ao abrir FIFO do servidor");
            exit(1);
        }
    }

    write(server_fd, message, strlen(message));
    close(server_fd);

    int client_fd = open(client_fifo, O_RDONLY);
    if (client_fd < 0) {
        perror("Erro ao abrir FIFO do cliente");
        exit(1);
    }

    char buffer[512];
    int n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("Erro ao ler resposta do servidor");
    }
    if (n >= 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    close(client_fd);
}