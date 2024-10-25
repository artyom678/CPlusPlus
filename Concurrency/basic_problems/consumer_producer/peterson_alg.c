#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#define TRUE 1
#define FALSE 0
#define N 2


int interested[N];
int turn = 0;

void enter_region(int process) {
    int other = 1 - process;
    turn = process;
    interested[process] = TRUE;
    while(turn == process && interested[other] == TRUE) {
        printf("waiting...\n");
    }
}

void leave_region(int process) {
    interested[process] = FALSE;
}

#define BUFSIZE 100
#define CONSUMER 0
#define PRODUCER 1

sem_t full;
sem_t empty;
int count = 0;

void* consumer() {
    for(int i = 0; i != BUFSIZE*2;++i) {
        sem_wait(&empty);
        enter_region(CONSUMER);
        --count;
        printf("consumer takes an item!\nitems left: %d\n", count);
        leave_region(CONSUMER);
        sem_post(&full);
    }
    return NULL;
}

void* producer() {
    for(int i = 0; i != BUFSIZE*2; ++i) {
        sem_wait(&full);
        enter_region(PRODUCER);
        ++count;
        printf("producer puts an item!\nitems left: %d\n", count);
        leave_region(PRODUCER);
        sem_post(&empty);
    }
    return NULL;
}





int main() {
    sem_init(&full, 0, BUFSIZE); //if the initial value of semaphore is less, than 1, then the very first thread, that calls wait() is blocked.
    sem_init(&empty, 0, 0);
    pthread_t cons;
    pthread_t prod;
    pthread_create(&cons, NULL, &consumer, NULL);
    pthread_create(&prod, NULL, &producer, NULL);
    pthread_join(prod,  NULL);
    pthread_join(cons, NULL);
    sem_destroy(&full);
    sem_destroy(&empty);
    return 0;
}








