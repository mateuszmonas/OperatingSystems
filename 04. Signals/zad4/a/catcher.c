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

void handle_sigusr1(){
    received_signals++;
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

    sigaction(SIGRTMIN, &message_action, NULL);
    sigaction(SIGUSR1, &message_action, NULL);

    struct sigaction final_message_action;
    final_message_action.sa_sigaction = handle_sigusr2;
    final_message_action.sa_flags = SA_SIGINFO;

    sigaction(SIGRTMIN + 1, &final_message_action, NULL);
    sigaction(SIGUSR2, &final_message_action, NULL);

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
    union sigval value;
    value.sival_int = 0;
    for (int i = 0; i < received_signals; ++i) {
        switch(mode){
            case KILL:
                kill(pid, SIGUSR1);
                break;
            case SIGQUEUE:
                value.sival_int = i;
                sigqueue(pid, SIGUSR1, value);
                break;
            case SIGRT:
                kill(pid, SIGRTMIN);
                break;
        }
    }
    switch(mode){
        case KILL:
            kill(pid, SIGUSR2);
            break;
        case SIGQUEUE:
            sigqueue(pid, SIGUSR2, value);
            break;
        case SIGRT:
            kill(pid, SIGRTMIN + 1);
            break;
    }
    return 0;
}