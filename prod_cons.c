#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 10

int buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
pthread_cond_t cond_cheio, cond_vazio;


void insere(int item) {
    static int in = 0;
    printf("Produzindo item\n");
    
    pthread_mutex_lock(&mutex);
    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&mutex);
}

void *produtor(void *arg){

}

void *consumidor(void *arg){

}

int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_cheio, NULL);
    pthread_cond_init(&cond_vazio, NULL);

    pthread_t prod, cons;

    pthread_create(&prod, NULL, produtor, NULL);
    pthread_create(&cons, NULL, consumidor, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_cheio);
    pthread_cond_destroy(&cond_vazio);

    return 0;
}