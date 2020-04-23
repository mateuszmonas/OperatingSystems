#include "commands.h"
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>

#define FREE_SLOT -1

bool running = true;
int private_descriptor;
char *client_queue_paths[MAX_CLIENTS];
int client_queues[MAX_CLIENTS];
bool available[MAX_CLIENTS];
int client_count = 0;

void handle_message(chat_msg *msg);

void handle_exit(){
    running = false;
    chat_msg msg = {.mtype=STOP};
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_queues[i] != FREE_SLOT) {
            mq_send(client_queues[i], (char *) &msg, MSG_SIZE, 1);
            mq_close(client_queues[i]);
        }
    }
    mq_close(private_descriptor);
    mq_unlink(server_queue_path);
}

void handle_sigint(){
    exit(EXIT_SUCCESS);
}

void init(chat_msg * msg){
    int i = 0;
    while (i < MAX_CLIENTS) {
        if (client_queues[i] == FREE_SLOT) {
            break;
        }
        i++;
    }
    char *response = calloc(256, sizeof(char));
    if (i == MAX_CLIENTS) {
        sprintf(response, "server is full\n");
        strcpy(msg->text, response);
        msg->client_id = -1;
        int queue = mq_open(client_queue_paths[i], O_WRONLY);
        mq_send(queue, (char *) msg, MSG_SIZE, 1);
        mq_close(queue);
        printf("%s", response);
        return;
    }
    msg->client_id = i;
    strcpy(client_queue_paths[i], msg->queue_path);
    available[i] = true;
    client_queues[i] = mq_open(client_queue_paths[i], O_WRONLY);
    sprintf(response, "%d joined\n", i);
    strcpy(msg->text, response);
    mq_send(client_queues[i], (char *) msg, MSG_SIZE, 1);
    client_count++;
    printf("%s", response);
}

void echo(chat_msg * msg){
    mq_send(client_queues[msg->client_id], (char *) msg, MSG_SIZE, 1);
    printf("%s\n", msg->text);
}

void connect(chat_msg * msg){
    int id1 = atoi(msg->text);
    int id2 = msg->client_id;
    if (id1 < MAX_CLIENTS && available[id1] && available[id2]) {
        msg->mtype = CONNECT;
        strcpy(msg->text, "connected");
        msg->client_id = id2;
        strcpy(msg->queue_path, client_queue_paths[id2]);
        mq_send(client_queues[id1], (char *) msg, MSG_SIZE, 1);
        msg->client_id = id1;
        strcpy(msg->queue_path, client_queue_paths[id1]);
        mq_send(client_queues[id2], (char *) msg, MSG_SIZE, 1);
        available[id1] = false;
        available[id2] = false;
        printf("connected\n");
    }
}

void list(chat_msg * msg){
    char list[2048];
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        char client_text[16];
        if (client_queues[i] != FREE_SLOT) {
            sprintf(client_text, "%d %s\n", i, available[i] ? "" : "busy");
            strcat(list, client_text);
        }
    }
    printf("%s\n", list);
    strcpy(msg->text, list);
    mq_send(client_queues[msg->client_id], (char *) msg, MSG_SIZE, 1);
}

void disconnect(chat_msg * msg){
    int id1 = atoi(msg->text);
    int id2 = msg->client_id;
    if (!available[id1] && !available[id2]) {
        msg->mtype = DISCONNECT;
        strcpy(msg->text, "disconnected");
        mq_send(client_queues[id1], (char *) msg, MSG_SIZE, 1);
        mq_send(client_queues[id2], (char *) msg, MSG_SIZE, 1);
        available[id1] = true;
        available[id2] = true;
        printf("disconnected\n");
    }
}

void stop(chat_msg * msg){
    mq_close(client_queues[msg->client_id]);
    client_queues[msg->client_id] = FREE_SLOT;
    available[msg->client_id] = false;
    client_count--;
    printf("%d left\n", msg->client_id);
}

void handle_message(chat_msg * msg){
    if(!msg) return;
    switch (msg->mtype) {
        case INIT:
            init(msg);
            break;
        case ECHO:
            echo(msg);
            break;
        case CONNECT:
            connect(msg);
            break;
        case LIST:
            list(msg);
            break;
        case DISCONNECT:
            disconnect(msg);
            break;
        case STOP:
            stop(msg);
            break;
    }
}

int main(int argc, char** argv) {
    mq_unlink(server_queue_path);
        atexit(handle_exit);
    signal(SIGINT, handle_sigint);

    struct mq_attr posix_attr;
    posix_attr.mq_maxmsg = 10;
    posix_attr.mq_msgsize = MSG_SIZE;
    private_descriptor = mq_open(server_queue_path, O_RDONLY | O_CREAT | O_EXCL, 0666, &posix_attr);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_queues[i] = FREE_SLOT;
        available[i] = false;
        client_queue_paths[i] = calloc(16, sizeof(char));
    }

    chat_msg msg;
    while(running){
        mq_receive(private_descriptor, (char *) &msg, MSG_SIZE, NULL);
        handle_message(&msg);
    }
    return 0;
}