#include "server_utils.h"

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
