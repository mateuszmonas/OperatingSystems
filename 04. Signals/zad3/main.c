#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

bool waiting_for_signal = true;

void handle_sigchld(int sig, siginfo_t * info, void *ucontext){
    printf("sigchld\n");
    printf("si_pid: %d si_uid: %d si_status: %d\n", info->si_pid, info->si_uid, info->si_status);
    waiting_for_signal = false;
}

void handle_sigalarm(int sig, siginfo_t * info, void *ucontext){
    printf("sigalarm\n");
    printf("si_timerid: %d\n", info->si_timerid);
    waiting_for_signal = false;
}

void handle_siqgueue(int sig, siginfo_t * info, void *ucontext){
    printf("siqgueue\n");
    printf("si_int: %d\n", info->si_int);
    waiting_for_signal = false;
}

int main(int argc, char** argv){
    if (argc < 2) {
        fprintf(stderr, "za mało argumentów");
        return 1;
    }

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    if(strcmp(argv[1], "sigchld") == 0){
        act.sa_sigaction = handle_sigchld;
        sigaction(SIGCHLD, &act, NULL);
        if(fork() == 0){
            exit(111);
        }
    } else if(strcmp(argv[1], "sigqueue") == 0){
        act.sa_sigaction = handle_siqgueue;
        sigaction(SIGUSR1, &act, NULL);
        union sigval value = {.sival_int = 222};
        sigqueue(getpid(), SIGUSR1, value);
    } else if(strcmp(argv[1], "sigalarm") == 0){
        act.sa_sigaction = handle_sigalarm;
        sigaction(SIGALRM, &act, NULL);
        alarm(1);
    }

    while(waiting_for_signal){}

    return 0;
}