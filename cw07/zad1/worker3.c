#include <signal.h>
#include <zconf.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include "commands.h"

int *packages;
int semid;
struct sembuf sops_before[2];
struct sembuf sops_after[3];
int running = 1;

static void finish_process(){
    shmdt(packages);
    running = 0;
    exit(EXIT_SUCCESS);
}

void send_package(){
    semop(semid, sops_before, 2);
    int packages_to_prepare = semctl(semid, PACKAGES_TO_PREPARE, GETVAL, NULL);
    int packages_to_send = semctl(semid, PACKAGES_TO_SEND, GETVAL, NULL);
    if (packages_to_send != 0) {
        int package_index = semctl(semid, PACKAGE_TO_SEND_INDEX, GETVAL, NULL) % MAX_PACKAGE_COUNT;
        packages[package_index] *= 3;

        printf("[%d %ld] Wysłałem zamówienie o wielkości %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n",
               getpid(), time(NULL), packages[package_index], packages_to_prepare, packages_to_send - 1);

        semop(semid, sops_after, 3);
    } else {
        semop(semid, sops_after, 1);
    }
}

int main(int argc, char **argv){
    signal(SIGALRM, finish_process);
    alarm(WORKING_TIME);

    srand(time(NULL));
    struct sembuf sops_before0 = {.sem_num = MEMORY_BUSY, .sem_op = 0, .sem_flg = 0};
    struct sembuf sops_before1 = {.sem_num = MEMORY_BUSY, .sem_op = 1, .sem_flg = 0};
    struct sembuf sops_after0 = {.sem_num = MEMORY_BUSY, .sem_op = -1, .sem_flg = 0};
    struct sembuf sops_after1 = {.sem_num = PACKAGES_TO_SEND, .sem_op = -1, .sem_flg = 0};
    struct sembuf sops_after2 = {.sem_num = PACKAGE_TO_SEND_INDEX, .sem_op = 1, .sem_flg = 0};

    sops_before[0] = sops_before0;
    sops_before[1] = sops_before1;
    sops_after[0] = sops_after0;
    sops_after[1] = sops_after1;
    sops_after[2] = sops_after2;

    key_t project_key = ftok(getenv("HOME"), PROJECT_ID);
    semid = semget(project_key, 0, 0);
    int shmid = shmget(project_key, 0, 0);
    packages = shmat(shmid, NULL, 0);

    while (running)
    {
        usleep(rand_time);
        send_package();
    }

}