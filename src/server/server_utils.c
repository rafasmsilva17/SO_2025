#include "server_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

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

