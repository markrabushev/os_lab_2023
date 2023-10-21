#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;


void *threadFunc1(void *arg) {
    pthread_mutex_lock(&mut1);
    printf("Thread 1 has locked Mutex 1\n");
    printf("Thread 1 is trying to lock Mutex 2\n");
    pthread_mutex_lock(&mut2);
    printf("Thread 1 has locked Mutex 2\n");
    pthread_mutex_unlock(&mut2);
    pthread_mutex_unlock(&mut1);
    return NULL;
}


void *threadFunc2(void *arg) {
    pthread_mutex_lock(&mut2);
    printf("Thread 2 has locked Mutex 2\n");
    printf("Thread 2 is trying to lock Mutex 1\n");
    pthread_mutex_lock(&mut1);
    printf("Thread 2 has locked Mutex 1\n");
    pthread_mutex_unlock(&mut1);
    pthread_mutex_unlock(&mut2);
    return NULL;
}


int main() {
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, threadFunc1, NULL);
    pthread_create(&thread2, NULL, threadFunc2, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mut1);
    pthread_mutex_destroy(&mut2);
    return 0;
}