#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define KILL 0
#define SIGQUEUE 1
#define SIGRT 2

int received_signals = 0;
bool waiting_for_signals = true;
int pid;
int mode = KILL;

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

void confirm_sig_received(){
    send_signal(mode == SIGRT ? SIGRTMIN : SIGUSR1, 0);
}

void handle_confirmation_request(){
    confirm_sig_received();
}

void handle_sigusr1(int sig, siginfo_t * info, void *ucontext){
    pid = info->si_pid;
    printf("otrzymano sygnał %d\n", ++received_signals);
    confirm_sig_received();
}

void handle_sigusr2(int sig, siginfo_t * info, void *ucontext){
    pid = info->si_pid;
    waiting_for_signals = false;
}

int main(int argc, char** argv){

    if(strcmp(argv[1], "kill") == 0){
        mode = KILL;
    } else if (strcmp(argv[1], "sigqueue") == 0) {
        mode = SIGQUEUE;
    } else if (strcmp(argv[1], "sigrt") == 0) {
        mode = SIGRT;
    }
    struct sigaction message_action;
    message_action.sa_sigaction = handle_sigusr1;
    message_action.sa_flags = SA_SIGINFO;
    sigaddset(&message_action.sa_mask, SIGUSR1);
    sigaddset(&message_action.sa_mask, SIGRTMIN);

    sigaction(SIGUSR1, &message_action, NULL);
    sigaction(SIGRTMIN, &message_action, NULL);

    struct sigaction confirmation_request_action;
    message_action.sa_handler = handle_confirmation_request;
    message_action.sa_flags = 0;
    sigaddset(&message_action.sa_mask, SIGUSR1);
    sigaddset(&message_action.sa_mask, SIGRTMIN);
    sigaddset(&message_action.sa_mask, SIGRT + 2);

    sigaction(SIGRTMIN + 2, &confirmation_request_action, NULL);

    struct sigaction final_message_action;
    final_message_action.sa_sigaction = handle_sigusr2;
    final_message_action.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR2, &final_message_action, NULL);
    sigaction(SIGRTMIN + 1, &final_message_action, NULL);

    printf("%d\n", getpid());

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
    printf("otrzymane sygnały: %d\n", received_signals);

    for (int i = 0; i < received_signals; ++i) {
        send_signal(mode == SIGRT ? SIGRTMIN : SIGUSR1, i);
    }
    send_signal(mode == SIGRT ? SIGRTMIN + 1 : SIGUSR2, 0);
    return 0;
}