#include <dirent.h>
#include <zconf.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>

bool running = true;

void sigtstp_handle(){
    if(running == false){
        running = true;
        return;
    }
    running = false;
    printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGTSTP);
    while (!running){
        sigsuspend(&mask);
    }
}

void sigint_handle(){
    running = false;
    printf("Odebrano sygnał SIGINT\n");
    exit(0);
}

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = sigint_handle;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    signal(SIGTSTP, sigtstp_handle);
    char *cwd = malloc(PATH_MAX * sizeof(char));
    getcwd(cwd, PATH_MAX);
    while(running){
        DIR* dir = opendir(cwd);
        struct dirent* d;
        while ((d = readdir(dir)) != NULL) {
            printf("%s/%s\n", cwd, d->d_name);
            sleep(1);
        }
    }
}