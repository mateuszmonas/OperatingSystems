#include <stdlib.h>
#include <stdio.h>
#include <zconf.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <wait.h>
#include "commands.h"

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "not enough arguments");
        return 1;
    }
    long first_count = strtol(argv[1], NULL, 10);
    long second_count = strtol(argv[2], NULL, 10);
    long third_count = strtol(argv[3], NULL, 10);

    union semun arg;
    arg.val = 0;
    key_t project_key = ftok(getenv("HOME"), PROJECT_ID);
    int semid = semget(project_key, SEMAPHORE_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    semctl(semid, PACKAGES_TO_PREPARE, SETVAL, arg);
    semctl(semid, PACKAGES_TO_SEND, SETVAL, arg);
    semctl(semid, MEMORY_BUSY, SETVAL, arg);
    semctl(semid, PACKAGE_TO_PREPARE_INDEX, SETVAL, arg);
    semctl(semid, PACKAGE_TO_SEND_INDEX, SETVAL, arg);
    semctl(semid, PACKAGE_TO_CREATE_INDEX, SETVAL, arg);
    int shmid = shmget(project_key, MAX_PACKAGE_COUNT * sizeof(int), IPC_CREAT | IPC_EXCL | 0666);

    for (int i = 0; i < first_count; ++i) {
        if (fork() == 0) {
            return 0;
        }
    }
    for (int i = 0; i < second_count; ++i) {
        if (fork() == 0) {
            return 0;
        }
    }
    for (int i = 0; i < third_count; ++i) {
        if (fork() == 0) {
            return 0;
        }
    }

    int i = 0;
    while (i < first_count + second_count + third_count) {
        int pid;
        if ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
            printf("Process %d finished\n", pid);
            i++;
        }
    }

    semctl(semid, 0, IPC_RMID);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}