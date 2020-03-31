#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void handler(int sig, siginfo_t * info, void *ucontext){

}

int main(int argc, char** argv){

    if (argc < 2) {
        fprintf(stderr, "za mało argumentów");
        return 1;
    }
    if(strcmp(argv[1], "sigchld") == 0){

    }

    struct sigaction act;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, NULL);
}