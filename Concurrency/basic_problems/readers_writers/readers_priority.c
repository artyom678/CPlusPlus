#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#define READERS 5
#define WRITERS 3
#define TRUE 1
#define FALSE 0

pthread_mutex_t mutex;
pthread_mutex_t read_mutex;

int count_readers = 0;

void my_read(int i) {
    printf("Reader %d reads\nNow %d/%d readers are reading\n", i, count_readers, READERS);
    sleep(1);
}

void* reader(void* arg) {
    int i = *(int*)arg;
    while(TRUE) {
        pthread_mutex_lock(&read_mutex);
        ++count_readers;
        if (count_readers == 1) {
            pthread_mutex_lock(&mutex);
        }
        pthread_mutex_unlock(&read_mutex);
        my_read(i);
        pthread_mutex_lock(&read_mutex);
        --count_readers;
        if (count_readers == 0) {
            pthread_mutex_unlock(&mutex);
        }
        pthread_mutex_unlock(&read_mutex);
        sleep(1);
    }
}

void my_write(int i) {
    printf("Writer %d writes\n", i);
    sleep(1);
}

void* writer(void* arg) {
    int i = *(int*)arg;
    while(TRUE) {
        pthread_mutex_lock(&mutex);
        my_write(i);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}


int max(int x, int y) {
    return x > y ? x : y;
}

int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&read_mutex, NULL);
    const int MAX = max(READERS, WRITERS);
    int* nums = (int*)malloc((MAX * sizeof(int)));
    for(int i = 0; i != MAX; ++i) {
        nums[i] = i;
    }
    pthread_t rd[READERS];
    pthread_t wr[WRITERS];
    for(int i = 0; i != READERS; ++i) {
        pthread_create(&rd[i], NULL, &reader, (void*)&nums[i]);
    }
    for(int i = 0; i != WRITERS; ++i) {
        pthread_create(&wr[i], NULL, &writer, (void*)&nums[i]);
    }
    for(int i = 0; i != READERS; ++i) {
        pthread_join(rd[i], NULL);
    }
    for(int i = 0; i != WRITERS; ++i) {
        pthread_join(wr[i], NULL);
    }
    free(nums);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&read_mutex);
    return 0;
}
