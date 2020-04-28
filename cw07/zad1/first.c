#include <signal.h>
#include <zconf.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include "commands.h"

int *packages;
int semid;

static void finish_process(){
    shmdt(packages);
    exit(EXIT_SUCCESS);
}

void get_package(){
    int package_size = rand() % 10;
    semop(semid, NULL, SEMAPHORE_SIZE);

}

int main(int argc, char **argv){
    signal(SIGALRM, finish_process);
    alarm(WORKING_TIME);

    srand(time(NULL));

    key_t project_key = ftok(getenv("HOME"), PROJECT_ID);
    semid = semget(project_key, 0, 0);
    int shmid = shmget(project_key, PACKAGE_COUNT * sizeof(int), 0);
    packages = shmat(shmid, NULL, 0);

}