#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SERVER_FIFO "../pipes/server_fifo"
#define MAX_MSG 512
#define MAX_DOCS 100

typedef struct {
    char key[16];
    char title[200];
    char authors[200];
    char year[5];
    char path[64];
    int active;
} Document;

Document docs[MAX_DOCS];
int doc_count = 0;

void send_response(const char *client_fifo, const char *message) {
    int fd = open(client_fifo, O_WRONLY);
    if (fd >= 0) {
        write(fd, message, strlen(message));
        close(fd);
    }
}

char* generate_key() {
    static int next_id = 1;
    static char key[16];
    snprintf(key, sizeof(key), "doc%d", next_id++);
    return key;
}

int find_doc_index(const char *key) {
    for (int i = 0; i < doc_count; ++i) {
        if (docs[i].active && strcmp(docs[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

void handle_add(char *args[], const char *client_fifo) {
    if (doc_count >= MAX_DOCS) {
        send_response(client_fifo, "ERROR|Limite de documentos atingido.\n");
        return;
    }

    // Verifica se o ficheiro existe na diretoria base
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "../docs/%s", args[4]);
    printf("DEBUG: full_path = %s\n", full_path);
    if (access(full_path, F_OK) != 0) {
        send_response(client_fifo, "ERROR|Ficheiro não encontrado na diretoria base.\n");
        return;
    }

    // Gera chave e armazena meta-informação
    char *key = generate_key();
    strcpy(docs[doc_count].key, key);
    strcpy(docs[doc_count].title, args[1]);
    strcpy(docs[doc_count].authors, args[2]);
    strcpy(docs[doc_count].year, args[3]);
    strcpy(docs[doc_count].path, args[4]);  // só o caminho relativo
    docs[doc_count].active = 1;
    doc_count++;

    char response[64];
    snprintf(response, sizeof(response), "OK|key=%s\n", key);
    send_response(client_fifo, response);
}


void handle_consult(const char *key, const char *client_fifo) {
    int idx = find_doc_index(key);
    if (idx == -1) {
        send_response(client_fifo, "ERROR|Documento não encontrado.\n");
        return;
    }

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "OK|%s|%s|%s|%s\n",
             docs[idx].title, docs[idx].authors, docs[idx].year, docs[idx].path);
    send_response(client_fifo, buffer);
}

void handle_delete(const char *key, const char *client_fifo) {
    int idx = find_doc_index(key);
    if (idx == -1) {
        send_response(client_fifo, "ERROR|Documento não encontrado.\n");
        return;
    }

    docs[idx].active = 0;
    send_response(client_fifo, "OK|Documento removido com sucesso.\n");
}

void handle_lines(const char *key, const char *keyword, const char *client_fifo) {
    int idx = find_doc_index(key);
    if (idx == -1) {
        send_response(client_fifo, "ERROR|Documento não encontrado.\n");
        return;
    }

    int pipefd[2];
    pipe(pipefd);
    pid_t pid = fork();

    if (pid == 0) {
        close(pipefd[0]); // fechar leitura
        dup2(pipefd[1], STDOUT_FILENO);
        execlp("grep", "grep", "-c", keyword, docs[idx].path, NULL);
        perror("execlp grep");
        exit(1);
    } else {
        close(pipefd[1]); // fechar escrita
        wait(NULL);
        char buffer[64];
        int n = read(pipefd[0], buffer, sizeof(buffer)-1);
        buffer[n] = '\0';
        close(pipefd[0]);

        char response[128];
        snprintf(response, sizeof(response), "OK|Linhas encontradas: %s", buffer);
        send_response(client_fifo, response);
    }
}

void handle_search(const char *keyword, const char *client_fifo) {
    char result[512] = "OK|";
    int found = 0;

    for (int i = 0; i < doc_count; ++i) {
        if (!docs[i].active) continue;

        int pid = fork();
        if (pid == 0) {
            execlp("grep", "grep", "-q", keyword, docs[i].path, NULL);
            exit(1);
        }
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            strcat(result, docs[i].key);
            strcat(result, ";");
            found = 1;
        }
    }

    if (!found) {
        send_response(client_fifo, "OK|Nenhum documento encontrado.\n");
    } else {
        strcat(result, "\n");
        send_response(client_fifo, result);
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
