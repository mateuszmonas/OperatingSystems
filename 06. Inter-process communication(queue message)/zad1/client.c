#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/msg.h>
#include <zconf.h>
#include <stdbool.h>
#include <string.h>
#include "commands.h"

bool running = true;
key_t private_key;
int private_descriptor;
key_t server_key;
int server_descriptor;
int client_id;

int receiver_id;
int receiver_key;
int receiver_descriptor;

void separate_command(char *line, char *command, char *rest)
{
    char line_cpy[256];
    strcpy(line_cpy, line);

    char *line_tmp = line_cpy;
    char *cmd = strtok_r(line_cpy, " \n", &line_tmp);

    char *txt = strtok_r(NULL, "\n", &line_tmp);
    if (txt == NULL)
    {
        rest[0] = '\0';
    }
    else
    {
        strcpy(rest, txt);
    }
    if (cmd == NULL)
    {
        command[0] = '\0';
    }
    else
    {
        strcpy(command, cmd);
    }
}

void init(){
    chat_msg msg = {.mtype=INIT, .client_key=private_key};
    msgsnd(server_descriptor, &msg, MSG_SIZE, 0);
    if (msgrcv(private_descriptor, &msg, MSG_SIZE, 0, 0) < 0) {
        printf("server error");
    }
    if (msg.client_id == -1) {
        printf("server full");
        exit(EXIT_FAILURE);
    }
    client_id = msg.client_id;
    printf("%s", msg.text);
}

void echo(char *text) {
    chat_msg msg = {.mtype=ECHO, .client_id=client_id, .client_key=private_key};
    strcpy(msg.text, text);
    msgsnd(receiver_descriptor, &msg, MSG_SIZE, 0);
//    msgrcv(private_descriptor, &msg, MSG_SIZE, 0, 0);
//    printf("%s", msg.text);
}

void connect(char* id){
    chat_msg msg = {.mtype=CONNECT, .client_id=client_id, .client_key=private_key};
    strcpy(msg.text, id);
    msgsnd(server_descriptor, &msg, MSG_SIZE, 0);
    msgrcv(private_descriptor, &msg, MSG_SIZE, 0, 0);
    receiver_key = msg.client_key;
    receiver_id = msg.client_id;
    receiver_descriptor = msgget(receiver_key, 0);
    printf("%s", msg.text);
}

void list(){
    chat_msg msg = {.mtype=LIST, .client_id=client_id, .client_key=private_key};
    msgsnd(server_descriptor, &msg, MSG_SIZE, 0);
    msgrcv(private_descriptor, &msg, MSG_SIZE, 0, 0);
    printf("%s", msg.text);
}

void disconnect(){
    chat_msg msg = {.mtype=DISCONNECT, .client_id=client_id, .client_key=private_key};
    msgsnd(server_descriptor, &msg, MSG_SIZE, 0);
}

void stop(){
    printf("stopping\n");
    chat_msg msg = {.mtype=STOP, .client_id=client_id, .client_key=private_key};
    msgsnd(server_descriptor, &msg, MSG_SIZE, 0);
}

void handle_sigint(){
    exit(EXIT_SUCCESS);
}

void handle_message(chat_msg * msg){
    if(!msg) return;
    switch (msg->mtype) {
        case ECHO:
            printf("%s", msg->text);
            break;
        case STOP:
            exit(EXIT_SUCCESS);
        case CONNECT:
            receiver_id = msg->client_id;
            receiver_key = msg->client_key;
            receiver_descriptor = msgget(msg->client_key, 0);
            printf("%s", msg->text);
            break;
        case DISCONNECT:
            receiver_descriptor = server_descriptor;
            printf("%s", msg->text);
    }
}

void handle_command(char* cmd, char* args){
    if (strcmp(cmd, "stop") == 0) {
        exit(EXIT_SUCCESS);
    } else if(strcmp(cmd, "echo") == 0){
        echo(args);
    } else if(strcmp(cmd, "list") == 0){
        list();
    } else if(strcmp(cmd, "connect") == 0){
        connect(args);
    } else if(strcmp(cmd, "disconnect") == 0){
        disconnect();
    }
}

void handle_exit(){
    running = false;
    disconnect();
    stop();
    msgctl(private_descriptor, IPC_RMID, NULL);
}

int main(int argc, char **argv) {
    atexit(handle_exit);
    signal(SIGINT, handle_sigint);
    private_key = ftok(getenv("HOME"), getpid());
    private_descriptor = msgget(private_key, IPC_CREAT | IPC_EXCL | 0666);

    server_key = ftok(getenv("HOME"), PROJECT_ID);
    server_descriptor = msgget(server_key, 0);
    receiver_descriptor = server_descriptor;

    init();

    char buff[256];
    char rest[256];
    char cmd[256];
    chat_msg msg;
    struct msqid_ds buf;
    while (running){
        printf("\nType command:\n> ");
        fgets(buff, 256, stdin);
        separate_command(buff, cmd, rest);
        handle_command(cmd, rest);

        msgctl(private_descriptor, IPC_STAT, &buf);
        while (0 < buf.msg_qnum){
            msgrcv(private_descriptor, &msg, MSG_SIZE, 0, IPC_NOWAIT);
            handle_message(&msg);
            msgctl(private_descriptor, IPC_STAT, &buf);
        }
    }

    return 0;
}