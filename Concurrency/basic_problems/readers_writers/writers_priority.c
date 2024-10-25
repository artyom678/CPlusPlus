#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#define READERS 10
#define WRITERS 3
#define TRUE 1
#define FALSE 0

int wr_cnt = 0;
int rd_cnt = 0;

pthread_mutex_t main_mutex, wr_cnt_mutex, rd_cnt_mutex, writers_mutex;
pthread_cond_t stop_readers_cond;

void Read(int i) {
    printf("Reader %d is reading\t Active readers: %d/%d\n", i, rd_cnt, READERS);
}

void* reader(void* arg) {
    int i = *(int*)arg;
    while(TRUE) {
        pthread_mutex_lock(&wr_cnt_mutex);
        while (wr_cnt > 0) {
            printf("Reader %d waits...\n", i);
            pthread_cond_wait(&stop_readers_cond, &wr_cnt_mutex);
        }
        pthread_mutex_unlock(&wr_cnt_mutex);
        pthread_mutex_lock(&rd_cnt_mutex);
        ++rd_cnt;
        if (rd_cnt == 1) {
            pthread_mutex_lock(&main_mutex);
        }
        pthread_mutex_unlock(&rd_cnt_mutex);
        Read(i);
        pthread_mutex_lock(&rd_cnt_mutex);
        --rd_cnt;
        if (rd_cnt == 0) {
            pthread_mutex_unlock(&main_mutex);
        }
        pthread_mutex_unlock(&rd_cnt_mutex);
        sleep(1);
    }
}

void Write(int i) {
    printf("Writer %d is wrinting\t Active writers: %d/%d\n", i, wr_cnt, WRITERS);
}

void* writer(void* arg) {
    int i = *(int*)arg;
    while(TRUE) {
        //to stop giving new readers an access to the critical zone
        
        pthread_mutex_lock(&wr_cnt_mutex);
        ++wr_cnt;
        if (wr_cnt == 1) {
                printf("Stop new readers!\n");
                pthread_mutex_lock(&main_mutex);
        }
        pthread_mutex_unlock(&wr_cnt_mutex);
        pthread_mutex_lock(&writers_mutex);
        Write(i);
        pthread_mutex_unlock(&writers_mutex);
        pthread_mutex_lock(&wr_cnt_mutex);
        --wr_cnt;
        if (wr_cnt == 0) {
            pthread_mutex_unlock(&main_mutex);
            printf("Readers unlocked!\n");
            pthread_cond_broadcast(&stop_readers_cond);
        }
        pthread_mutex_unlock(&wr_cnt_mutex);
        sleep(1);
    }
}


int main() {
    pthread_mutex_init(&rd_cnt_mutex, NULL);
    pthread_mutex_init(&wr_cnt_mutex, NULL);
    pthread_mutex_init(&main_mutex, NULL);
    pthread_mutex_init(&writers_mutex, NULL);
    pthread_cond_init(&stop_readers_cond, NULL);
    const int MAX = READERS > WRITERS? READERS : WRITERS;
    int* nums = (int*)malloc(MAX*sizeof(int));
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
    pthread_cond_destroy(&stop_readers_cond);
    pthread_mutex_destroy(&rd_cnt_mutex);
    pthread_mutex_destroy(&wr_cnt_mutex);
    pthread_mutex_destroy(&main_mutex);
    pthread_mutex_destroy(&writers_mutex);
    return 0;
}
