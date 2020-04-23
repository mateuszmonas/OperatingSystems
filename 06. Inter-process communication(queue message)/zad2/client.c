#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <zconf.h>
#include <stdbool.h>
#include <string.h>
#include <mqueue.h>
#include "commands.h"

bool running = true;
char private_queue_path[16];
int private_descriptor;
int server_descriptor;
int client_id;

int receiver_id;
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
    chat_msg msg = {.mtype=INIT};
    strcpy(msg.queue_path, private_queue_path);
    mq_send(server_descriptor, (char *) &msg, MSG_SIZE, 1);
    mq_receive(private_descriptor, (char *) &msg, MSG_SIZE, NULL);
    if (msg.client_id == -1) {
        printf("server full");
        exit(EXIT_FAILURE);
    }
    client_id = msg.client_id;
    printf("%s", msg.text);
}

void echo(char *text) {
    chat_msg msg = {.mtype=ECHO, .client_id=client_id};
    strcpy(msg.text, text);
    mq_send(receiver_descriptor, (char *) &msg, MSG_SIZE, 1);
}

void connect(char* id){
    chat_msg msg = {.mtype=CONNECT, .client_id=client_id};
    strcpy(msg.text, id);
    mq_send(server_descriptor, (char *) &msg, MSG_SIZE, 1);
    mq_receive(private_descriptor, (char *) &msg, MSG_SIZE, NULL);
    receiver_id = msg.client_id;
    receiver_descriptor = mq_open(msg.queue_path, O_WRONLY);
    printf("%s", msg.text);
}

void list(){
    chat_msg msg = {.mtype=LIST, .client_id=client_id};
    strcpy(msg.queue_path, private_queue_path);
    mq_send(server_descriptor, (char *) &msg, MSG_SIZE, 1);
    mq_receive(private_descriptor, (char *) &msg, MSG_SIZE, NULL);
    printf("%s", msg.text);
}

void disconnect(){
    chat_msg msg = {.mtype=DISCONNECT, .client_id=client_id};
    sprintf(msg.text, "%d", receiver_id);
    strcpy(msg.queue_path, private_queue_path);
    mq_send(server_descriptor, (char *) &msg, MSG_SIZE, 1);
    mq_close(receiver_descriptor);
    receiver_descriptor = server_descriptor;
}

void stop(){
    printf("stopping\n");
    chat_msg msg = {.mtype=STOP, .client_id=client_id};
    strcpy(msg.queue_path, private_queue_path);
    mq_send(server_descriptor, (char *) &msg, MSG_SIZE, 1);
    mq_close(private_descriptor);
    mq_close(receiver_descriptor);
    mq_close(server_descriptor);
    mq_unlink(private_queue_path);
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
            receiver_descriptor = mq_open(msg->queue_path, O_WRONLY);
            receiver_id = msg->client_id;
            printf("%s", msg->text);
            break;
        case DISCONNECT:
            mq_close(receiver_descriptor);
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
}

int main(int argc, char **argv) {
    atexit(handle_exit);
    signal(SIGINT, handle_sigint);
    snprintf(private_queue_path, 10, "/%d", getpid());
    struct mq_attr posix_attr;
    posix_attr.mq_maxmsg = 10;
    posix_attr.mq_msgsize = MSG_SIZE;
    private_descriptor = mq_open(private_queue_path, O_RDONLY | O_CREAT | O_EXCL, 0666, &posix_attr);

    server_descriptor = mq_open(server_queue_path, O_WRONLY);
    receiver_descriptor = server_descriptor;

    init();

    char buff[256];
    char rest[256];
    char cmd[256];
    chat_msg msg;
    struct mq_attr buf;
    while (running){
        printf("\nType command:\n> ");
        fgets(buff, 256, stdin);
        separate_command(buff, cmd, rest);
        handle_command(cmd, rest);

        mq_getattr(private_descriptor, &buf);
        while (0 < buf.mq_curmsgs){
            mq_receive(private_descriptor, (char *) &msg, MSG_SIZE, NULL);
            handle_message(&msg);
            mq_getattr(private_descriptor, &buf);
        }
    }

    return 0;
}