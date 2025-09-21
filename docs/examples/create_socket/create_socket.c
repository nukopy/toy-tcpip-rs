#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // AF_INET 等
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int create_socket(void) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    return fd;
}

int main(void) {
    printf("Start creating sockets...\n");

    // IPv4/TCP ソケットを作る（プロトコルは 0 で自動選択）
    // fds: file descriptors
    // fd: file descriptor
    // create array of file descriptors
    int fds[2];
    for (int i = 0; i < 2; i++) {
        fds[i] = create_socket();
        if (fds[i] == -1) {
            perror("Failed to create socket");
            return 1;
        }
        printf("socket fd%d = %d\n", i, fds[i]);
    }

    // close all sockets
    for (int i = 0; i < 2; i++) {
        close(fds[i]);
    }
    printf("All sockets closed.\n");

    return 0;
}
