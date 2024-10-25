#include <stdio.h>
#include <pthread.h>
#define BUFSIZE 20
int count = 0;
pthread_mutex_t mutex;
pthread_cond_t empty, full;

void* consumer() {
    for(int i = 0; i != BUFSIZE; ++i) {
        pthread_mutex_lock(&mutex);
        while (count == 0) pthread_cond_wait(&empty, &mutex);
        --count;
        printf("consumer takes an item\nitems left: %d\n", count);
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* producer() {
    for(int i = 0; i != BUFSIZE; ++i) {        
        pthread_mutex_lock(&mutex);
        while (count == BUFSIZE) pthread_cond_wait(&full, &mutex);
        ++count;
        printf("producer puts an item\nitems left: %d\n", count);
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&empty, NULL);
    pthread_cond_init(&full, NULL);
    pthread_t cons, prod;
    pthread_create(&prod, NULL, &producer, NULL);
    pthread_create(&cons, NULL, &consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&empty);
    pthread_cond_destroy(&full);
    return 0;
}
