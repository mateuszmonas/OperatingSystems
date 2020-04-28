#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#define KILL 0
#define SIGQUEUE 1
#define SIGRT 2

int received_signals = 0;
bool waiting_for_signals = true;
bool use_sigqueue = false;

void using_kill(int pid, int amount){
    for (int i = 0; i < amount; ++i) {
        kill(pid, SIGUSR1);
    }
    kill(pid, SIGUSR2);
}

void using_sigqueue(int pid, int amount){
    union sigval value;
    value.sival_int = 0;
    for (int i = 0; i < amount; ++i) {
        value.sival_int = i;
        sigqueue(pid, SIGUSR1, value);
    }
    sigqueue(pid, SIGUSR2, value);
}

void using_sigrt(int pid, int amount) {
    for (int i = 0; i < amount; ++i) {
        kill(pid, SIGRTMIN);
    }
    kill(pid, SIGRTMIN + 1);
}

void handle_sigusr1(int sig, siginfo_t * info, void *ucontext){
    if(use_sigqueue){
        printf("otrzymano sygnał %d\n", info->si_value.sival_int);
    }
    received_signals++;
}

void handle_sigusr2(){
    waiting_for_signals = false;
}

int main(int argc, char** argv){
    if(argc < 4){
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }

    int pid = (int) strtol(argv[1], NULL, 10);
    int message_count = (int) strtol(argv[2], NULL, 10);
    if(strcmp(argv[3], "kill") == 0){
        using_kill(pid, message_count);
    } else if (strcmp(argv[3], "sigqueue") == 0) {
        use_sigqueue = true;
        using_sigqueue(pid, message_count);
    } else if (strcmp(argv[3], "sigrt") == 0) {
        using_sigrt(pid, message_count);
    }

    struct sigaction message_action;
    message_action.sa_sigaction = handle_sigusr1;
    message_action.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &message_action, NULL);
    sigaction(SIGRTMIN, &message_action, NULL);

    struct sigaction final_message_action;
    final_message_action.sa_handler = handle_sigusr2;
    final_message_action.sa_flags = 0;

    sigaction(SIGUSR2, &final_message_action, NULL);
    sigaction(SIGRTMIN + 1, &final_message_action, NULL);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGRTMIN);
    sigdelset(&mask, SIGRTMIN + 1);
    sigdelset(&mask, SIGINT);

    while (waiting_for_signals) {
        sigsuspend(&mask);
    }
    printf("otrzymane sygnały: %d/%d\n", received_signals, message_count);
}