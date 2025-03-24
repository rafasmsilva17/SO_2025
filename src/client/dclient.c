#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define PIPE_SC "../../pipes/server_fifo"
#define PIPE_CL "../../pipes/client_fifo"
#define BUFFER_SIZE 512

int main() {
    char cm[BUFFER_SIZE];  // Allocate a buffer
    while(1){
        scanf("%s", cm);  // Read input into the buffer

        int fd_write = open(PIPE_SC, O_WRONLY);
        if (fd_write == -1) {
            perror("Erro ao abrir FIFO para escrita");
            exit(1);
        }

        write(fd_write, cm, strlen(cm) + 1);  // Use strlen() + 1 to send null-terminated string
        close(fd_write);

            // Verifica se o FIFO CLIENTE já existe
        if (access(PIPE_CL, F_OK) == -1) {
            // FIFO não existe, então cria
            if (mkfifo(PIPE_CL, 0666) == -1) {
                perror("Erro ao criar FIFO");
                exit(1);
            }
            printf("FIFO criado: %s\n", PIPE_CL);
        } else {
            printf("FIFO já existe.\n");
        }
           // Open FIFO for reading (wait for server response)
        int fd_read = open(PIPE_CL, O_RDONLY);
        if (fd_read == -1) {
            perror("Erro ao abrir FIFO para leitura");
            exit(1);
        }

        char response[BUFFER_SIZE];
        int bytes_read = read(fd_read, response, BUFFER_SIZE);
        if (bytes_read > 0) {
            response[bytes_read] = '\0';
            printf("Server response: %s\n", response);
        }

        close(fd_read);
    }
    return 0;
}