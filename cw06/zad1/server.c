#include "commands.h"
#include <sys/ipc.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define FREE_SLOT -1

bool running = true;
key_t private_key;
int private_descriptor;
key_t client_pids[MAX_CLIENTS];
bool available[MAX_CLIENTS];
int client_count = 0;

void handle_message(chat_msg *msg);

void handle_exit(){
    running = false;
    chat_msg msg = {.mtype=STOP};
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_pids[i] != FREE_SLOT) {
            msg.client_id = i;
            msg.client_key = client_pids[i];
            msgsnd(msgget(client_pids[i], 0), &msg, MSG_SIZE, 0);
        }
    }
    while (0 < client_count) {
        msgrcv(private_descriptor, &msg, MSG_SIZE, STOP, 0);
        handle_message(&msg);
    }
    msgctl(private_descriptor, IPC_RMID, NULL);
}

void handle_sigint(){
    exit(EXIT_SUCCESS);
}

void init(chat_msg * msg){
    int i = 0;
    while (i < MAX_CLIENTS) {
        if (client_pids[i] == FREE_SLOT) {
            break;
        }
        i++;
    }
    char *response = calloc(256, sizeof(char));
    if (i == MAX_CLIENTS) {
        sprintf(response, "server is full\n");
        msg->client_id = -1;
        msgsnd(msgget(msg->client_key, 0), msg, MSG_SIZE, 0);
        printf("%s", response);
        return;
    }
    client_pids[i] = msg->client_key;
    available[i] = true;
    msg->client_id = i;
    sprintf(response, "%d joined\n", i);
    strcpy(msg->text, response);
    msgsnd(msgget(client_pids[i], 0), msg, MSG_SIZE, 0);
    client_count++;
    printf("%s", response);
}

void echo(chat_msg * msg){
    msgsnd(msgget(client_pids[msg->client_id], 0), msg, MSG_SIZE, 0);
    printf("%s\n", msg->text);
}

void connect(chat_msg * msg){
    int id1 = atoi(msg->text);
    int id2 = msg->client_id;
    if (id1 < MAX_CLIENTS && available[id1] && available[id2]) {
        msg->mtype = CONNECT;
        strcpy(msg->text, "connected");
        msg->client_id = id2;
        msg->client_key = client_pids[id2];
        msgsnd(msgget(client_pids[id1], 0), msg, MSG_SIZE, 0);
        msg->client_id = id1;
        msg->client_key = client_pids[id1];
        msgsnd(msgget(client_pids[id2], 0), msg, MSG_SIZE, 0);
        available[id1] = false;
        available[id2] = false;
        printf("connected\n");
    }
}

void list(chat_msg * msg){
    char list[2048];
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        char client_text[16];
        if (client_pids[i] != FREE_SLOT) {
            sprintf(client_text, "%d %s\n", i, available[i] ? "" : "busy");
            strcat(list, client_text);
        }
    }
    printf("%s\n", list);
    strcpy(msg->text, list);
    msgsnd(msgget(client_pids[msg->client_id], 0), msg, MSG_SIZE, 0);
}

void disconnect(chat_msg * msg){
    int id1 = atoi(msg->text);
    int id2 = msg->client_id;
    if (!available[id1] && !available[id2]) {
        msg->mtype = DISCONNECT;
        strcpy(msg->text, "disconnected");
        msgsnd(msgget(client_pids[id1], 0), msg, MSG_SIZE, 0);
        msgsnd(msgget(client_pids[id2], 0), msg, MSG_SIZE, 0);
        available[id1] = true;
        available[id2] = true;
        printf("disconnected\n");
    }
}

void stop(chat_msg * msg){
    client_pids[msg->client_id] = FREE_SLOT;
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
    atexit(handle_exit);
    signal(SIGINT, handle_sigint);
    private_key = ftok(getenv("HOME"), PROJECT_ID);
    private_descriptor = msgget(private_key, IPC_CREAT | IPC_EXCL | 0666);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_pids[i] = FREE_SLOT;
        available[i] = false;
    }
    chat_msg msg;
    while(running){
        msgrcv(private_descriptor, &msg, MSG_SIZE, -STOP, 0);
        handle_message(&msg);
    }
    return 0;
}