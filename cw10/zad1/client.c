#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <zconf.h>
#include <pthread.h>
#include <signal.h>

#define MAX_MESSAGE_LENGTH 256

int socket_fd;
int running = 1;

void on_server_disconnected(){
    printf("server disconnected\n");
    running = 0;
    exit(0);
}

void handle_exit() {
    shutdown(socket_fd, SHUT_RDWR);
}

void listener(){
    char buffer[MAX_MESSAGE_LENGTH];
    char *ping = "ping";
    char *disconnecting = "disconnecting\n";
    int bytes;
    while (0 < (bytes = read(socket_fd, buffer, MAX_MESSAGE_LENGTH))) {
        buffer[bytes] = '\0';
        if (strcmp(buffer, ping) == 0) {
            write(socket_fd, ping, strlen(ping));
        } else if(strcmp(buffer, disconnecting) == 0) {
            on_server_disconnected();
        } else {
            printf("%s", buffer);
        }
    }
    on_server_disconnected();
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }
    signal(SIGPIPE, on_server_disconnected);
    signal(SIGINT, handle_exit);
    atexit(handle_exit);
    char *connection_type = argv[2];
    char *socket_path = argv[3];

    if (strcmp(connection_type, "local") == 0) {
        socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, socket_path);
        connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    }
    else if (strcmp(connection_type, "remote") == 0) {
        struct addrinfo* res;
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        getaddrinfo("localhost", socket_path, &hints, &res);
        socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        connect(socket_fd, res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);
    } else {
        fprintf(stderr, "wrong connection type\n");
        return 1;
    }

    pthread_t t;
    pthread_create(&t, NULL, (void *) listener, NULL);

    char buff[MAX_MESSAGE_LENGTH];
    while (running){
        printf("\nType command:\n");
        fgets(buff, MAX_MESSAGE_LENGTH, stdin);
        write(socket_fd, buff, strlen(buff));
    }
}