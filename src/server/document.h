#ifndef DOCUMENT_H
#define DOCUMENT_H

typedef struct document Document;

int find_doc_index(const char *key);

void handle_add(char *args[], const char *client_fifo);

void handle_consult(const char *key, const char *client_fifo);

void handle_delete(const char *key, const char *client_fifo);

void handle_lines(const char *key, const char *keyword, const char *client_fifo);

void handle_search(const char *keyword, const char *client_fifo);

#endif