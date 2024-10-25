#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#define N 5
#define TRUE 1
#define FALSE 0

pthread_mutex_t forks[N];

void eat() {}
void think() {}

void* philosopher(void* arg) {
    int i = *(int*)arg;
    int first, second;
    int count = 0;
    if (i == N - 1) {
        first = 0;
        second = i;
    }
    else {
        first = i;
        second = i + 1;
    }
    while(TRUE) {
        think();
        pthread_mutex_lock(&forks[first]);
        pthread_mutex_lock(&forks[second]);
        eat();
        ++count;
        printf("philosopher %d eaten %d times\n", i, count);
        pthread_mutex_unlock(&forks[first]);
        pthread_mutex_unlock(&forks[second]);
    }
    return NULL;
}

int main() {
    for(int i = 0; i != N; ++i) {
        pthread_mutex_init(&forks[i], NULL);
    }
    int* nums = (int*)malloc(N*sizeof(int));
    for(int i = 0; i != N; ++i) {
        nums[i] = i;
    }
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
    return 0;
}
