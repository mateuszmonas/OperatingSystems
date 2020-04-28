#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

bool waiting_for_signal = true;

void handle_sigchld(int sig, siginfo_t * info, void *ucontext){
    printf("sigchld\n");
    printf("si_pid: %d si_uid: %d si_status: %d\n", info->si_pid, info->si_uid, info->si_status);
    waiting_for_signal = false;
}

void handle_sigalarm(int sig, siginfo_t * info, void *ucontext){
    printf("sigalarm\n");
    printf("si_code: %d == %d :SI_TIMER\n", info->si_code, SI_TIMER);
    printf("si_timerid: %d si_overrun: %d\n", info->si_timerid, info->si_overrun);
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
        sigaction(SIGRTMIN, &act, NULL);

        struct itimerspec its;
        struct sigevent sev;
        timer_t timerid;

        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGRTMIN;
        sev.sigev_value.sival_ptr = &timerid;
        timer_create(CLOCK_REALTIME, &sev, &timerid);
        its.it_value.tv_sec = 1;
        its.it_value.tv_nsec = 0;
        its.it_interval.tv_sec = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;

        timer_settime(timerid, 0, &its, NULL);
    }

    while(waiting_for_signal){}

    return 0;
}