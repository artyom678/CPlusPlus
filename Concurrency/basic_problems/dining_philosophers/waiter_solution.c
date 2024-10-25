//This solution is absolutely the same as mine(concerning the idea), but is more effective as it uses only 1 semaphore instead of 2(as like my decision does)
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#define N 5
#define TRUE 1
#define FALSE 0

pthread_mutex_t forks[N];
sem_t waiter;

void* eat() {
    return NULL;
}

void* think() {
    return NULL;
}

void* philosopher(void* arg) {
    int n = *(int*)arg;
    int count = 0;
    while(TRUE) {
        sem_wait(&waiter);
        pthread_mutex_lock(&forks[n]);
        pthread_mutex_lock(&forks[(n+1)%N]);
        eat();
        ++count;
        printf("philosopher %d ate %d times\n", n, count);
        pthread_mutex_unlock(&forks[(n+1)%N]);
        pthread_mutex_unlock(&forks[n]);
        sem_post(&waiter);
    }
    return NULL;
}

int main() {

    for(int i = 0; i != N; ++i) {
        pthread_mutex_init(&forks[i], NULL);
    }
    sem_init(&waiter, 0, N-1);
    
    int* nums = (int*)malloc(N*sizeof(int));
    for(int i = 0; i != N; ++i) nums[i] = i;

    pthread_t ph[N];
    for(int i = 0; i != N; ++i) {
        pthread_create(&ph[i], NULL, &philosopher, (void*)&nums[i]); 
    }

    for(int i = 0; i != N; ++i) {
        pthread_join(ph[i], NULL);
    }
    
    free(nums);
    for(int i = 0; i != N; ++i) {
        pthread_mutex_destroy(&forks[i]);
    }
    sem_destroy(&waiter);
    return 0;
}








