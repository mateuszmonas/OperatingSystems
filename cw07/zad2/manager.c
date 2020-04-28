#include <stdlib.h>
#include <stdio.h>
#include <zconf.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include "commands.h"

sem_t *semaphores[SEMAPHORES_SIZE];

void create_semaphores(){
    for (int i = 0; i < SEMAPHORES_SIZE; ++i) {
        semaphores[i] = sem_open(SEMAPHORES[i], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    }
    sem_post(semaphores[2]);
}

void remove_semaphores(){
    for (int i = 0; i < SEMAPHORES_SIZE; ++i) {
        sem_close(semaphores[i]);
        sem_unlink(SEMAPHORES[i]);
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }
    long first_count = strtol(argv[1], NULL, 10);
    long second_count = strtol(argv[2], NULL, 10);
    long third_count = strtol(argv[3], NULL, 10);

    create_semaphores();
    int fd = shm_open(SHARED_MEMORY, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    ftruncate(fd, MAX_PACKAGE_COUNT * sizeof(int));

    for (int i = 0; i < first_count; ++i) {
        if (fork() == 0) {
            execlp("./out/worker1", "worker1", NULL);
        }
    }
    for (int i = 0; i < second_count; ++i) {
        if (fork() == 0) {
            execlp("./out/worker2", "worker2", NULL);
        }
    }
    for (int i = 0; i < third_count; ++i) {
        if (fork() == 0) {
            execlp("./out/worker3", "worker3", NULL);
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

    remove_semaphores();
    shm_unlink(SHARED_MEMORY);

    return 0;
}