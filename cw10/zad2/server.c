#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <zconf.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "../zad1/game.c"

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
struct sockaddr client_addresses[MAX_CLIENTS];
board_t boards[MAX_CLIENTS / 2];
int epoll_fd;
int network_socket;
int local_socket;
char *port_number;
char *socket_path;

int init_local_socket(char *socket_path) {
    int socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);
    bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    return socket_fd;
}

int init_network_socket(char* port_number) {
    struct addrinfo *res;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, port_number, &hints, &res);
    int socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(socket_fd, res->ai_addr, res->ai_addrlen);
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
    sendto(client_fds[client_index], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fds[client_index], &event);
    shutdown(client_fds[client_index], SHUT_RDWR);
    client_fds[client_index] = FREE_SLOT;
    client_statuses[client_index] = OFFLINE;
    response = "opponent has left\n";
    if (client_fds[opponent_index(client_index)] != FREE_SLOT) {
        sendto(client_fds[opponent_index(client_index)], response, strlen(response), 0, &client_addresses[opponent_index(client_index)], sizeof(client_addresses[opponent_index(client_index)]));
    }
}

// -1 on error
//client index on success
int connect_client(struct sockaddr *addr, int socket_fd){
    int j = 0;
    while (j < MAX_CLIENTS) {
        if (client_fds[j] == FREE_SLOT) {
            client_fds[j] = socket_fd;
            memcpy(&client_addresses[j], addr, sizeof(&addr));
            client_statuses[j] = ONLINE;
            return j;
        }
        j++;
    }
    sendto(socket_fd, "server is full\n", 15, 0, addr, sizeof(&addr));
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
                    sendto(client_fds[i], ping, strlen(ping), 0, &client_addresses[i], sizeof(client_addresses[i]));
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
    sendto(client_fds[client_index], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
    sendto(client_fds[opponent_index(client_index)], response, strlen(response), 0, &client_addresses[opponent_index(client_index)], sizeof(client_addresses[opponent_index(client_index)]));
    free(response);
}

void move(int position, int client_index){
    char *response;
    if (opponent_index(client_index) == FREE_SLOT) {
        response = "you are not currently in game\n";
        sendto(client_fds[client_index], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
    } else if (make_move(&boards[client_index / 2], position, client_index) == 0) {
        response = "your turn\n";
        sendto(client_fds[opponent_index(client_index)], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
        response = board_to_string(&boards[client_index / 2]);
        sendto(client_fds[client_index], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
        sendto(client_fds[opponent_index(client_index)], response, strlen(response), 0, &client_addresses[opponent_index(client_index)], sizeof(client_addresses[opponent_index(client_index)]));
        free(response);
        board_object bo = get_winner(&boards[client_index / 2]);
        if (boards[client_index / 2].o_move == 9 || bo != EMPTY) {
            if (bo == EMPTY) {
                response = "it's a draw";
            } else {
                response = bo == O ? "the winner is O" : "the winnder is X";
            }
            sendto(client_fds[client_index], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
            sendto(client_fds[opponent_index(client_index)], response, strlen(response), 0, &client_addresses[opponent_index(client_index)], sizeof(client_addresses[opponent_index(client_index)]));
            start_game(client_index);
        }
    } else {
        response = "error\n";
        sendto(client_fds[client_index], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
    }
}

void handle_exit(){
    char *msg = "disconnecting\n";
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        sendto(client_fds[i], msg, strlen(msg), 0, &client_addresses[i], sizeof(client_addresses[i]));
        shutdown(local_socket, SHUT_RDWR);
    }
    shutdown(local_socket, SHUT_RDWR);
    shutdown(network_socket, SHUT_RDWR);
    unlink(socket_path);
    exit(0);
}

int main(int argc, char **argv){
    if (argc < 3) {
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }
    atexit(handle_exit);
    signal(SIGINT, handle_exit);
    signal(SIGPIPE, sigpipe_handle);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_fds[i] = FREE_SLOT;
        client_statuses[i] = OFFLINE;
    }

    port_number = argv[1];
    socket_path = argv[2];

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
            int client_index = 0;
            struct sockaddr receive_address;
            socklen_t receive_address_length = sizeof(receive_address);
            recvfrom(events[i].data.fd, &buffer, MAX_MESSAGE_LENGTH, 0, &receive_address, &receive_address_length);
            char *command = strtok(buffer, " ");

            while (client_index < MAX_CLIENTS && client_addresses[client_index].sa_data != receive_address.sa_data) {
                client_index++;
            }
            if (strcmp(command, "join") == 0) {
                if(client_index < MAX_CLIENTS) {
                    response = "already connected\n";
                    sendto(client_fds[client_index], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
                }else if (-1 < (client_index = connect_client(&receive_address, events->data.fd))) {
                    if (client_fds[opponent_index(client_index)] == FREE_SLOT) {
                        response = "waiting for opponent\n";
                        sendto(client_fds[client_index], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
                    } else {
                        response = "found opponent\n";
                        sendto(client_fds[client_index], response, strlen(response), 0, &client_addresses[client_index], sizeof(client_addresses[client_index]));
                        sendto(client_fds[opponent_index(client_index)], response, strlen(response), 0, &client_addresses[opponent_index(client_index)], sizeof(client_addresses[opponent_index(client_index)]));
                        start_game(client_index);
                    }
                }
            } else if (strcmp(command, "ping") == 0) {
                client_statuses[client_index] = ONLINE;
            } else if (strcmp(command, "move") == 0) {
                int position = (int) strtol(strtok(NULL, " "), NULL, 10);
                move(position, client_index);
            } else if (strcmp(command, "exit") == 0) {
                disconnect_client(client_index);
            } else {
                response = "unknown command\n";
                sendto(events[i].data.fd, response, strlen(response), 0, &receive_address,
                       receive_address_length);
            }
            printf("%s", buffer);
        }
    }
}