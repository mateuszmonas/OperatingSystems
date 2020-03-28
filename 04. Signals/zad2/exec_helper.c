#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>

bool check_pending(int signal){
    sigset_t pending;
    sigpending(&pending);
    return sigismember(&pending, signal);
}


int main(int argc, char** argv){
    printf("exec\n");
    if (argc > 1 && strcmp(argv[1], "pending") == 0) {
        printf("SIGUSR1 %s pending\n", check_pending(SIGUSR1) ? "is" : "is not");
    } else {
        raise(SIGUSR1);
    }
    return 0;
}