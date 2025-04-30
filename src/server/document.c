#include "document.h"
#include "server_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#define SERVER_FIFO "../pipes/server_fifo"
#define MAX_DOCS 100000


typedef struct document{
    char key[16];
    char title[200];
    char authors[200];
    char year[5];
    char path[64];
    int active;
} Document;

Document docs[MAX_DOCS];
int doc_count = 0;

Document* get_all_documents() {
    return docs;
}

// Retorna número total de documentos
int get_total_documents() {
    return doc_count;
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

    char *key = generate_key();
    strcpy(docs[doc_count].key, key);
    strcpy(docs[doc_count].title, args[1]);
    strcpy(docs[doc_count].authors, args[2]);
    strcpy(docs[doc_count].year, args[3]);
    strcpy(docs[doc_count].path, args[4]);
    docs[doc_count].active = 1;
    doc_count++;

    int key_number;
    sscanf(key, "doc%d", &key_number);  // Extrai apenas o número

    char response[64];
    snprintf(response, sizeof(response), "Document %d indexed\n", key_number);
    send_response(client_fifo, response);
}

void handle_consult(const char *key, const char *client_fifo) {
    int idx = find_doc_index(key);
    if (idx == -1) {
        send_response(client_fifo, "ERROR|Documento não encontrado.\n");
        return;
    }

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Title: %s\nAuthors: %s\nYear: %sPath: %s\n",
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
    char response [64];
    snprintf(response, sizeof(response), "Index entry %d deleted\n", idx);
    send_response(client_fifo, response);
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
        snprintf(response, sizeof(response), "%s", buffer);
        send_response(client_fifo, response);
    }
}

void handle_search(char *keyword, char *nr_processes_str, const char *client_fifo) {
    int max_procs = atoi(nr_processes_str);
    if (max_procs <= 0) {
        send_response(client_fifo, "Número de processos inválido.\n");
        return;
    }

    Document *documents = get_all_documents();
    int total = get_total_documents();

    int active_procs = 0;
    int pipe_fds[2];
    pipe(pipe_fds); // pipe para recolher os resultados dos filhos

    for (int i = 0; i < total; ++i) {
        if (!documents[i].active) continue;

        while (active_procs >= max_procs) {
            wait(NULL);
            active_procs--;
        }

        pid_t pid = fork();
        if (pid == 0) {
            // filho
            close(pipe_fds[0]);

            int grep_pipe[2];
            pipe(grep_pipe);

            pid_t grep_pid = fork();
            if (grep_pid == 0) {
                // neto executa grep
                dup2(grep_pipe[1], STDOUT_FILENO);
                close(grep_pipe[0]);
                close(grep_pipe[1]);
                execlp("grep", "grep", "-q", keyword, documents[i].path, NULL);
                exit(2); // erro ao executar grep
            }

            close(grep_pipe[1]);
            int status;
            waitpid(grep_pid, &status, 0);

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                // documento contém a keyword
                int doc_num;
                sscanf(documents[i].key, "doc%d", &doc_num);
                char msg[64];
                snprintf(msg, sizeof(msg), "%d\n", doc_num);
                write(pipe_fds[1], msg, strlen(msg));
            }
            close(grep_pipe[0]);
            close(pipe_fds[1]);
            exit(0);
        } else if (pid > 0) {
            active_procs++;
        } else {
            perror("fork");
        }
    }

    while (active_procs-- > 0) {
        wait(NULL);
    }

    // recolher resultados
    close(pipe_fds[1]);
    char result[2048] = "";
    char line[64];
    int first = 1;
    strcat(result, "[");

    int n;
    while ((n = read(pipe_fds[0], line, sizeof(line) - 1)) > 0) {
        line[n] = '\0';
        int doc_id = atoi(line);
        char temp[32];
        if (!first) {
            strcat(result, ", ");
        }
        snprintf(temp, sizeof(temp), "%d", doc_id);
        strcat(result, temp);
        first = 0;
    }
    close(pipe_fds[0]);
    strcat(result, "]\n");

    send_response(client_fifo, result);
}
