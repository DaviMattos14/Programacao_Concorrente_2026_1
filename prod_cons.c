#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 10

int buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
pthread_cond_t cond_cheio, cond_vazio;
int cheio = 0;

void *produtor(void *arg){

    static int in = 0;

    printf("Iniciando Producão\n");
    for (int i = 0; i < NUM_ITEMS; i++)
    {
        pthread_mutex_lock(&mutex);
        while (cheio == BUFFER_SIZE) {
            printf("P - Buffer cheio. Aguardando...\n");
            pthread_cond_wait(&cond_vazio, &mutex);
        }
        buffer[in] = i;
        printf("Produzido: %d\n", buffer[in]);
        in = (in + 1) % BUFFER_SIZE;
        cheio++;
        pthread_cond_signal(&cond_cheio);
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex);
    while (cheio == BUFFER_SIZE) pthread_cond_wait(&cond_vazio, &mutex);
    buffer[in] = -1;
    cheio++;
    printf("Producao finalizada\n");
    pthread_cond_signal(&cond_cheio);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *consumidor(void *arg){

    static int out = 0;
    int item;

    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (cheio == 0) {
            printf("C - Buffer vazio. Aguardando...\n");
            pthread_cond_wait(&cond_cheio, &mutex);
        }
        item = buffer[out];
        if (item == -1)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        printf("Consumido: %d\n", item);
        out = (out + 1) % BUFFER_SIZE;
        cheio--;
        pthread_cond_signal(&cond_vazio);
        pthread_mutex_unlock(&mutex);

    }
    pthread_exit(NULL);
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