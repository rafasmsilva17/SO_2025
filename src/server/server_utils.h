#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

void send_response(const char *client_fifo, const char *message);

char* generate_key();


#endif