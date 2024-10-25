#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <malloc.h>
#define TRUE 1
#define FALSE 0
#define N 5
#define LEFT (N + i - 1) % N
#define RIGHT (i + 1) % N
#define THINKING 0
#define HUNGRY 1
#define EATING 2

pthread_mutex_t mutex; // mutex for sequential access to states[N]
int states[N];
sem_t s[N]; // each semaphore is utilized to block an according philosopher

void test(int i) {
    if (states[i] == HUNGRY && states[LEFT] != EATING && states[RIGHT] != EATING) {
        states[i] = EATING;
        sem_post(&s[i]); // either unblocks the calling philosopher if it is blocked or increases the value of s[i] from 0 to 1
    }
}

void take_forks(int i) {
    pthread_mutex_lock(&mutex);
    states[i] = HUNGRY;
    test(i);
    pthread_mutex_unlock(&mutex);
    sem_wait(&s[i]); // if successful to take_forks, then continue executing(sem value turns from 1 to 0), and if not, then blocks the calling philosopher 
}

void put_forks(int i) {
    pthread_mutex_lock(&mutex);
    states[i] = THINKING;
    test(LEFT);
    test(RIGHT);
    pthread_mutex_unlock(&mutex);
}

void eat(int i, int* count) {
    printf("philosopher %d eaten %d times\n", i, ++(*count));
}

void think() {}

void* philosopher(void* arg) {
    int i = *(int*)arg;
    int count = 0;
    while(TRUE) {
        think();
        take_forks(i);
        eat(i,&count);
        put_forks(i);
    }
    return NULL;
}




int main() {
    pthread_mutex_init(&mutex, NULL);
    for(int i = 0; i != N; ++i) {
        sem_init(&s[i], 0, 0);
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
    pthread_mutex_destroy(&mutex);
    for(int i = 0; i != N; ++i) {
        sem_destroy(&s[i]);
    }
    return 0;
}


//Почему нужно синхронизировать доступ к массиву состояний?
//- Дело в том, что в ином случае может быть(будет) так: философ А голоден, хочет взять вилки, проверяет левого философа, тот в данный момент не ест, проверяет правого философа,
//тот тоже не ест в данный момент, а затем, перед тем, как поменять состояние этого философа на EATING, планировщик приостановит философа А и запустит философа Б, который
//сделае всё тоже самое и тоже будет приостановлен, а потом ещё и Д запустит, который тоже проделает те же действия. Далее философ А продолжит выполняться и начнёт есть,
//в то же время начнут есть ещё и Б с Д. По итогу 3 философа-соседа будут есть одновременно, чего не может быть по условию.
//P.S. Конечно, достаточно было упомянуть лишь философа Б, чтобы продемонстрировать, что получим нарушение.
//P.P.S. Касаемо put_forks() - если тут не захватывать мьютекс, то может выйти(даже точно выйдет) та же самая ситуация, когда будем вызывать тест() от левого и от правого
//философов.
//P.P.P.S. Почему эта строка должна выполняться после захвата мьютекса states[i] = HUNGRY;? - Представим, что это было бы не так. Тогда пускай, допустим, поток А кладёт вилки и
//приостанавливается планировщиком прям перед проверкой философа Б на состояние HUNGRY. В этот момент запускается философ Б, который теперь может поменять своё состояние на
//HUNGRY без захвата мьютекса, что он, собственно, и делает. Далее поток Б блокируется на мьютексе, и вновь запускается философ А, который видит, что философ Б голоден,
//а философ В по удачному совпадению не ест в данный момент. Таким образом, философ Б начниает есть и разблокируется вызовом функции sem_post. Далее опять запускается
//философ Б, который вызывает фунцию take_forks() - сейчас его состояние EATING, но на самом деле он не ест, что уже некорректно, так как мог бы сейчас поесть В, который мог
//реально быть голоден. Теперь Б переводит себя в состояние HUNGRY, а затем пытается взять вилки - допустим, берёт их и переходит в состояние EATING и ещё раз инкрементирует
//свой семафор, который принимает значение 2, что тоже некорректно, ведь теперь Б не заблокируется, когда в другой раз не сможет взять вилки. Либо может быть так, что Б
//не удаётся взять вилки, так как, после того, как Б меняет своё состояние на HUNGRY, Б может быть приостановлено, и начнёт выполняться А, который по счастливой случайности
//начнёт есть еду. Тогда Б не заблокируется после неудачной попытки взять вилки, так как её семафор может быть равен 1, так как изначально Б мог быть не заблокированным.





