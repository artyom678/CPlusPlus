#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#define BUFSIZE 100

sem_t full, empty, mutex;
int count = 0;

void* consumer() {
    for(int i = 0; i != BUFSIZE; ++i) {
        sem_wait(&empty);
        sem_wait(&mutex);
        --count;
        printf("consumer takes an item\nitems left: %d\n", count);
        sem_post(&mutex);
        sem_post(&full);
    }
    return NULL;
}

void* producer() {
    for(int i = 0; i != BUFSIZE; ++i) {
        sem_wait(&full);
        sem_wait(&mutex);
        ++count;
        printf("producer puts an item\nitems left: %d\n", count);
        sem_post(&mutex);
        sem_post(&empty);
    }
    return NULL;
}


int main() {
    sem_init(&full, 0, BUFSIZE);
    sem_init(&empty, 0, 0);
    sem_init(&mutex, 0, 1);
    pthread_t cons, prod;
    pthread_create(&prod, NULL, &producer, NULL);
    pthread_create(&cons, NULL, &consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    sem_destroy(&full);
    sem_destroy(&empty);
    sem_destroy(&mutex);
    return 0;
}
