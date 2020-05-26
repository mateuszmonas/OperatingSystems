#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <zconf.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "game.c"

#define MAX_BACKLOG 10
#define MAX_CLIENTS 10
#define MAX_EVENTS 10
#define MAX_MESSAGE_LENGTH 256

#define FREE_SLOT -1
#define OFFLINE 0
#define ONLINE 1

int running = 1;
int client_fds[MAX_CLIENTS];
int client_statuses[MAX_CLIENTS];
board_t boards[MAX_CLIENTS / 2];
int epoll_fd;
int network_socket;
int local_socket;
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
    getaddrinfo(NULL, port_number, &hints, &res);
    int socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(socket_fd, res->ai_addr, res->ai_addrlen);
    listen(socket_fd, MAX_BACKLOG);
    freeaddrinfo(res);
    return socket_fd;
}

int opponent_index(int client_index){
    if (client_index % 2 == 0) return client_index + 1;
    else return client_index - 1;
}

void disconnect_client(int client_index){
    char *response = "disconnecting\n";
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = client_fds[client_index];
    write(client_fds[client_index], response, strlen(response));
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fds[client_index], &event);
    shutdown(client_fds[client_index], SHUT_RDWR);
    client_fds[client_index] = FREE_SLOT;
    client_statuses[client_index] = OFFLINE;
    response = "opponent has left\n";
    if (client_fds[opponent_index(client_index)] != FREE_SLOT) {
        write(client_fds[opponent_index(client_index)], response, strlen(response));
    }
}

// -1 on error
//client index on success
int connect_client(struct epoll_event* epoll_event){
    int new_client = accept(epoll_event->data.fd, NULL, NULL);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = new_client;
    int j = 0;
    while (j < MAX_CLIENTS) {
        if (client_fds[j] == FREE_SLOT) {
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_client, &event);
            client_fds[j] = new_client;
            client_statuses[j] = ONLINE;
            return j;
        }
        j++;
    }
    write(new_client, "server is full\n", 15);
    shutdown(new_client, SHUT_RDWR);
    return -1;
}

void ping(){
    char *ping = "ping";
    while (running) {
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (client_fds[i] != FREE_SLOT) {
                if (client_statuses[i] == OFFLINE) {
                    disconnect_client(i);
                } else {
                    write(client_fds[i], ping, strlen(ping));
                    client_statuses[i] = OFFLINE;
                }
            }
        }
        sleep(5);
    }
}

void sigpipe_handle(){
}

void start_game(int client_index){
    char *response;
    boards[client_index / 2] = new_board();
    response = board_to_string(&boards[client_index / 2]);
    write(client_fds[client_index], response, strlen(response));
    write(client_fds[opponent_index(client_index)], response, strlen(response));
    free(response);
}

void move(int position, int client_index){
    char *response;
    if (opponent_index(client_index) == FREE_SLOT) {
        response = "you are not currently in game\n";
        write(client_fds[client_index], response, strlen(response));
    } else if (make_move(&boards[client_index / 2], position, client_index) == 0) {
        response = "your turn\n";
        write(client_fds[opponent_index(client_index)], response, strlen(response));
        response = board_to_string(&boards[client_index / 2]);
        write(client_fds[client_index], response, strlen(response));
        write(client_fds[opponent_index(client_index)], response, strlen(response));
        free(response);
        board_object bo = get_winner(&boards[client_index / 2]);
        if (boards[client_index / 2].o_move == 9 || bo != EMPTY) {
            if (bo == EMPTY) {
                response = "it's a draw";
            } else {
                response = bo == O ? "the winner is O" : "the winnder is X";
            }
            write(client_fds[client_index], response, strlen(response));
            write(client_fds[opponent_index(client_index)], response, strlen(response));
            start_game(client_index);
        }
    } else {
        response = "error\n";
        write(client_fds[client_index], response, strlen(response));
    }
}

void handle_exit(){
    char *msg = "disconnecting\n";
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        write(client_fds[i], msg, strlen(msg));
        shutdown(local_socket, SHUT_RDWR);
    }
    shutdown(local_socket, SHUT_RDWR);
    shutdown(network_socket, SHUT_RDWR);
}

int main(int argc, char **argv){
    if (argc < 3) {
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }
    atexit(handle_exit);
    signal(SIGPIPE, sigpipe_handle);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_fds[i] = FREE_SLOT;
        client_statuses[i] = OFFLINE;
    }

    char* port_number = argv[1];
    char* socket_path = argv[2];

    network_socket = init_network_socket(port_number);
    local_socket = init_local_socket(socket_path);
    pthread_t t;
    pthread_create(&t, NULL, (void *) ping, NULL);

    epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = local_socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, local_socket, &event);
    event.data.fd = network_socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, network_socket, &event);

    struct epoll_event events[MAX_EVENTS];

    char buffer[MAX_MESSAGE_LENGTH];
    char *response;
    while (running) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        printf("%d\n", event_count);
        for (int i = 0; i < event_count; ++i) {
            if (events[i].data.fd == local_socket || events[i].data.fd == network_socket) {
                int client_index;
                if ((client_index = connect_client(&events[i])) != -1) {
                    if (client_fds[opponent_index(client_index)] == FREE_SLOT) {
                        response = "waiting for opponent\n";
                        write(client_fds[client_index], response, strlen(response));
                    } else {
                        response = "found opponent\n";
                        write(client_fds[client_index], response, strlen(response));
                        write(client_fds[opponent_index(client_index)], response, strlen(response));
                        start_game(client_index);
                    }
                }
            } else {
                int bytes;
                int client_index = 0;
                while (client_index < MAX_CLIENTS && client_fds[client_index] != events[i].data.fd) {
                    client_index++;
                }
                if(0 < (bytes = read(events[i].data.fd, &buffer, MAX_MESSAGE_LENGTH))) {
                    buffer[bytes] = '\0';
                    char *command = strtok(buffer, " ");
                    if (strcmp(command, "ping") == 0) {
                        client_statuses[client_index] = ONLINE;
                    } else if (strcmp(command, "move") == 0) {
                        int position = (int) strtol(strtok(NULL, " "), NULL, 10);
                        move(position, client_index);
                    } else {
                        response = "unknown command\n";
                        write(events[i].data.fd, response, strlen(response));
                    }
                } else if (events[i].events == (EPOLLIN | EPOLLHUP)) {
                    disconnect_client(client_index);
                }
            }
        }
    }
}