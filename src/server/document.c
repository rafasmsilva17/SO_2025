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

void handle_search(char *keyword, char *nr_processes_str, const char *client_fifo)
 {
    char *keyword = args[2];
            int max_proc = atoi(args[3]);

            int num_docs = get_all_doc_paths(NULL); // Só para contar
            char **paths = malloc(sizeof(char*) * num_docs);
            get_all_doc_paths(paths);

            int per_proc = (num_docs + max_proc - 1) / max_proc;
            int pipefds[max_proc][2];
            for (int p = 0; p < max_proc; p++) pipe(pipefds[p]);

            for (int p = 0; p < max_proc; p++) {
                if (fork() == 0) {
                    dup2(pipefds[p][1], STDOUT_FILENO);
                    close(pipefds[p][0]);
                    close(pipefds[p][1]);

                    char *args_exec[per_proc + 4];
                    args_exec[0] = "grep";
                    args_exec[1] = "-l";
                    args_exec[2] = keyword;

                    int start = p * per_proc;
                    int j = 3;
                    for (int k = start; k < start + per_proc && k < num_docs; k++) {
                        args_exec[j++] = paths[k];
                    }
                    args_exec[j] = NULL;
                    execvp("grep", args_exec);
                    exit(1);
                }
                close(pipefds[p][1]);
            }

            char result[2048] = "";
            for (int p = 0; p < max_proc; p++) {
                char temp[512];
                int r = read(pipefds[p][0], temp, sizeof(temp) - 1);
                if (r > 0) {
                    temp[r] = '\0';
                    strcat(result, temp);
                }
                close(pipefds[p][0]);
                wait(NULL);
            }

            free(paths);
            send_response(client_fifo, result[0] ? result : "Nenhum resultado.\n");
}