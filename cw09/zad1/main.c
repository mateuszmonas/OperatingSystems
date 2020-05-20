#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <zconf.h>
#include <pthread.h>
#include <stdbool.h>

long client_count;
long chair_count;
sem_t free_chairs;

pthread_mutex_t sleeping_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sleeping_cond = PTHREAD_COND_INITIALIZER;
bool sleeping = true;

pthread_t *chairs;
pthread_t current_client = -1;

void barber(){
    int free_chair_count;
    int served_clients = 0;
    while (served_clients < client_count) {
        sem_getvalue(&free_chairs, &free_chair_count);
        pthread_mutex_lock(&sleeping_mutex);
        if (chair_count == free_chair_count) {
            printf("Golibroda: ide spac\n");
            sleeping = true;
            pthread_cond_wait(&sleeping_cond, &sleeping_mutex);
            sleeping = false;
        }
        pthread_mutex_unlock(&sleeping_mutex);
        // if barber was waked up, then current_client is already set
        // else get current_client
        if (current_client == -1) {
            current_client = chairs[free_chair_count];
            sem_post(&free_chairs);
            free_chair_count += 1;
        }
        printf("Golibroda: czeka %ld klientow, gole klienta %ld\n", chair_count - free_chair_count, current_client);
        usleep(rand() % 1000000);
        current_client = -1;
        served_clients++;
    }
}

void client(){
    int free_chair_count;
    while (sem_trywait(&free_chairs) == -1) {
        printf("zajęte; %ld\n", pthread_self());
        usleep(rand() % 1000000);
    }
    sem_getvalue(&free_chairs, &free_chair_count);
    chairs[free_chair_count] = pthread_self();
    pthread_mutex_lock(&sleeping_mutex);
    if (sleeping) {
        printf("Klient: budze golibrode; %ld\n", pthread_self());
        current_client = pthread_self();
        pthread_cond_broadcast(&sleeping_cond);
        sem_post(&free_chairs);
        pthread_mutex_unlock(&sleeping_mutex);
        return;
    }
    pthread_mutex_unlock(&sleeping_mutex);
    printf("Klient: poczekalnia, wolne miejsca %d; %ld\n", free_chair_count, pthread_self());
}

int main(int argc, char **argv) {
    if (argc < 3) {
        perror("not enough arguments");
    }
    srand(time(NULL));
    chair_count = strtol(argv[1], NULL, 10);
    client_count = strtol(argv[2], NULL, 10);
    sem_init(&free_chairs, 0, chair_count);
    chairs = malloc(chair_count * sizeof(pthread_t));
    pthread_t *threads = calloc(client_count + 1, sizeof(pthread_t));
    pthread_create(&threads[0], NULL, (void *) barber, NULL);
    for (int i = 0; i < client_count; ++i) {
        usleep(rand() % 1000000);
        pthread_create(&threads[i + 1], NULL, (void *) client, NULL);
    }
    for (int i = 0; i < client_count + 1; i++) {

        pthread_join(threads[i], NULL);
    }
}