#include <signal.h>
#include <zconf.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "commands.h"

int running = 1;
int memory_descriptor;
int *packages;
sem_t *memory_busy_sem;
sem_t *packages_to_prepare_sem;
sem_t *packages_to_send_sem;
sem_t *packages_index_sem;

static void finish_process(){
    sem_close(memory_busy_sem);
    sem_close(packages_to_prepare_sem);
    sem_close(packages_to_send_sem);
    sem_close(packages_index_sem);
    munmap(packages, MAX_PACKAGE_COUNT * sizeof(int));
    running = 0;
    exit(EXIT_SUCCESS);
}

void create_package(){
    sem_wait(memory_busy_sem);

    int packages_to_prepare;
    sem_getvalue(packages_to_prepare_sem, &packages_to_prepare);
    int packages_to_send;
    sem_getvalue(packages_to_send_sem, &packages_to_send);
    if (packages_to_prepare != 0) {
        int package_index;
        sem_getvalue(packages_index_sem, &package_index);
        packages[package_index] *= 2;
        sem_wait(packages_to_prepare_sem);
        sem_post(packages_to_send_sem);
        sem_post(packages_index_sem);

        printf("[%d %ld] Przygotowałem zamówienie o wielkości %d. Liczba zamównień do przygotowania: %d. Liczba zamównień do wysłania: %d.\n",
               getpid(), time(NULL), packages[package_index], packages_to_prepare - 1, packages_to_send + 1);

    }
    sem_post(memory_busy_sem);
}

int main(int argc, char **argv){
    signal(SIGALRM, finish_process);
    alarm(WORKING_TIME);

    memory_busy_sem = sem_open(MEMORY_BUSY, O_RDWR);
    packages_to_prepare_sem = sem_open(PACKAGES_TO_PREPARE, O_RDWR);
    packages_to_send_sem = sem_open(PACKAGES_TO_SEND, O_RDWR);
    packages_index_sem = sem_open(PACKAGE_TO_PREPARE_INDEX, O_RDWR);

    memory_descriptor = shm_open(SHARED_MEMORY, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    packages = mmap(NULL, MAX_PACKAGE_COUNT * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, memory_descriptor, 0);

    while (running)
    {
        usleep(rand_time);
        create_package();
    }

}