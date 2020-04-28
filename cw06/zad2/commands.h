#ifndef OPERATINGSYSTEMS_COMMANDS_H
#define OPERATINGSYSTEMS_COMMANDS_H

#include <sys/ipc.h>
#include <glob.h>

#define MAX_CLIENTS  10
#define PROJECT_ID 0x099
#define MAX_MSG_SIZE 2048

#define INIT 1
#define ECHO 2
#define CONNECT 3
#define LIST 4
#define DISCONNECT 5
#define STOP 6

typedef struct chat_msg{
    long mtype;
    int client_id;
    char queue_path[16];
    char text[MAX_MSG_SIZE];
} chat_msg;

char *server_queue_path = "/server_queue";

const size_t MSG_SIZE = sizeof(chat_msg);

#endif //OPERATINGSYSTEMS_COMMANDS_H