#include <signal.h>
#include <zconf.h>
#include <stdlib.h>
#include "commands.h"

static void finish_process(){
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){
    signal(SIGALRM, finish_process);
    alarm(WORKING_TIME);
}