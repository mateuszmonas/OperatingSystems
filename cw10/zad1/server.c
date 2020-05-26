#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <zconf.h>
#include <netdb.h>

#define MAX_BACKLOG 10

int init_local_socket(char *socket_path) {
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);
    unlink(socket_path);
    bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(socket_fd, MAX_BACKLOG);
    return socket_fd;
}

int init_network_socket(char* port_number) {
    struct addrinfo *res;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, port_number, NULL, &res);
    int socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(socket_fd, res->ai_addr, res->ai_addrlen);
    listen(socket_fd, MAX_BACKLOG);
    freeaddrinfo(res);
    return socket_fd;
}

int main(int argc, char **argv){
    if (argc < 3) {
        perror("not enough arguments");
    }
    char* port_number = argv[1];
    char* socket_path = argv[2];

    int local_socket;
    int network_socket;
}