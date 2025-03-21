#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define PIPE_SC "pipes/server_fifo"
#define BUFFER_SIZE 512

void cleanup(int signum) {
    unlink(PIPE_SC);  // Remove FIFO file
    printf("\nFIFO removed. Server shutting down...\n");
    exit(0);
}

int main()
{
    char buffer [BUFFER_SIZE];

    signal(SIGINT,cleanup);
    
        // Verifica se o FIFO já existe
    if (access(PIPE_SC, F_OK) == -1) {
        // FIFO não existe, então cria
        if (mkfifo(PIPE_SC, 0666) == -1) {
            perror("Erro ao criar FIFO");
            exit(1);
        }
        printf("FIFO criado: %s\n", PIPE_SC);
    } else {
        printf("FIFO já existe.\n");
    }

    printf("Servidor pronto...\n");

    while(1){ //loop infinito para estar sempre à espera de comandos
        int fd = open(PIPE_SC,O_RDONLY);
        if(fd==-1){
            perror("Erro ao abrir FIFO\n");
            exit(1);
        } 

        int buffsize = read(fd, buffer, sizeof(buffer));
        if(buffsize>0){
            buffer[buffsize] = '\0'; //garantir que é null terminated
        } 
        printf("recebido: %s\n", buffer);
        close(fd);
    }
}
