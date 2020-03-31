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
bool waiting_for_confirmation = false;
bool use_sigqueue = false;
sigset_t mask;

void handle_confirmation(){
    waiting_for_confirmation = false;
}

void wait_for_confirmation() {
    waiting_for_confirmation = true;
    while (waiting_for_confirmation) {
    }
}

void using_kill(int pid, int amount){
    printf("sending");
    for (int i = 0; i < amount; ++i) {
        kill(pid, SIGUSR1);
        sigsuspend(&mask);
    }
    kill(pid, SIGUSR2);
}

void using_sigqueue(int pid, int amount){
    union sigval value;
    value.sival_int = 0;
    for (int i = 0; i < amount; ++i) {
        value.sival_int = i;
        sigqueue(pid, SIGUSR1, value);
//        wait_for_confirmation();
    }
    sigqueue(pid, SIGUSR2, value);
}

void using_sigrt(int pid, int amount) {
    for (int i = 0; i < amount; ++i) {
        kill(pid, SIGRTMIN);
//        wait_for_confirmation();
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

    struct sigaction sigusr1_action;
    sigusr1_action.sa_handler = handle_confirmation;
    sigusr1_action.sa_flags = 0;
    sigaddset(&sigusr1_action.sa_mask, SIGUSR1);
    sigaddset(&sigusr1_action.sa_mask, SIGRTMIN);
    sigaction(SIGUSR1, &sigusr1_action, NULL);
    sigaction(SIGRTMIN, &sigusr1_action, NULL);

    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGRTMIN);
    sigdelset(&mask, SIGRTMIN + 1);
    sigdelset(&mask, SIGINT);

    if(strcmp(argv[3], "kill") == 0){
        using_kill(pid, message_count);
    } else if (strcmp(argv[3], "sigqueue") == 0) {
        use_sigqueue = true;
        using_sigqueue(pid, message_count);
    } else if (strcmp(argv[3], "sigrt") == 0) {
        using_sigrt(pid, message_count);
    }


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