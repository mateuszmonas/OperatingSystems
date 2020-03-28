#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <zconf.h>
#include <stdbool.h>

void block_signal(int Signal){
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, Signal);
    sigprocmask(SIG_BLOCK, &mask, NULL);
}

void handle_sig_usr1(){
    printf("SIGUSR handled\n");
}

bool check_pending(int signal){
    sigset_t pending;
    sigpending(&pending);
    return sigismember(&pending, signal);
}

int main(int argc, char **argv){
    if(argc < 2){
        fprintf(stderr, "no arguments\n");
        return 1;
    } else if (strcmp(argv[1], "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN);
    } else if (strcmp(argv[1], "handler") == 0) {
        signal(SIGUSR1, handle_sig_usr1);
    } else if (strcmp(argv[1], "mask") == 0) {
        block_signal(SIGUSR1);
    } else if (strcmp(argv[1], "pending") == 0) {
        block_signal(SIGUSR1);
    } else {
        fprintf(stderr, "wrong arguments\n");
        return 1;
    }

    printf("parent\n");
    raise(SIGUSR1);
    if (strcmp(argv[1], "pending") == 0) {
        printf("SIGUSR1 %s pending\n", check_pending(SIGUSR1) ? "is" : "is not");
    }

    if (fork() == 0) {
        printf("child\n");
        if (strcmp(argv[1], "pending") == 0) {
            printf("SIGUSR1 %s pending\n", check_pending(SIGUSR1) ? "is" : "is not");
        } else {
            raise(SIGUSR1);
        }
    } else if(strcmp(argv[1], "handler") != 0){
            execvp("./out/exec_helper", argv);
    }
    return 0;
}