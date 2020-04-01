#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#define KILL 0
#define SIGQUEUE 1
#define SIGRT 2

int mode = KILL;
int received_signals = 0;
int confirmed_signals = 0;
int pid;
bool waiting_for_signals = true;
bool waiting_for_confirmation = false;
sigset_t mask;

void send_signal(int signal, int val){
    switch(mode){
        case KILL:
            kill(pid, signal);
            break;
        case SIGQUEUE: {
           union sigval value = {.sival_int = val};
            sigqueue(pid, signal, value);
        }
        case SIGRT:
            kill(pid, signal);
            break;
    }
}

void handle_confirmation(){
    waiting_for_confirmation = false;
}

void request_confirmation(){
    if (waiting_for_confirmation) {
        kill(pid, SIGRTMIN + 2);
    }
}

void wait_for_confirmation() {
    waiting_for_confirmation = true;
    while (waiting_for_confirmation) {
        alarm(1);
        sigsuspend(&mask);
    }
    printf("otrzymano potwierdzenie %d\n", ++confirmed_signals);
}

void handle_sigusr1(int sig, siginfo_t * info, void *ucontext){
    if(mode == SIGQUEUE){
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
    pid = (int) strtol(argv[1], NULL, 10);
    int message_count = (int) strtol(argv[2], NULL, 10);

    struct sigaction sigusr1_action;
    sigusr1_action.sa_handler = handle_confirmation;
    sigusr1_action.sa_flags = 0;
    sigaddset(&sigusr1_action.sa_mask, SIGUSR1);
    sigaddset(&sigusr1_action.sa_mask, SIGRTMIN);
    sigaction(SIGUSR1, &sigusr1_action, NULL);
    sigaction(SIGRTMIN, &sigusr1_action, NULL);

    signal(SIGALRM, request_confirmation);

    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGRTMIN);
    sigdelset(&mask, SIGRTMIN + 1);
    sigdelset(&mask, SIGINT);

    if(strcmp(argv[3], "kill") == 0){
        mode = KILL;
    } else if (strcmp(argv[3], "sigqueue") == 0) {
        mode = SIGQUEUE;
    } else if (strcmp(argv[3], "sigrt") == 0) {
        mode = SIGRT;
    }

    for (int i = 0; i < message_count; ++i) {
        send_signal(mode == SIGRT ? SIGRTMIN : SIGUSR1, i);
        wait_for_confirmation();
    }
    send_signal(mode == SIGRT ? SIGRTMIN + 1 : SIGUSR2, 0);

    sigusr1_action.sa_sigaction = handle_sigusr1;
    sigusr1_action.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &sigusr1_action, NULL);
    sigaction(SIGRTMIN, &sigusr1_action, NULL);

    struct sigaction final_message_action;
    final_message_action.sa_handler = handle_sigusr2;
    final_message_action.sa_flags = 0;

    sigaction(SIGUSR2, &final_message_action, NULL);
    sigaction(SIGRTMIN + 1, &final_message_action, NULL);

    while (waiting_for_signals) {
        sigsuspend(&mask);
    }
    printf("otrzymane sygnały: %d/%d\n", received_signals, message_count);
}