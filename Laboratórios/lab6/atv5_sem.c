#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_THREADS 6
#define VAGAS 3

sem_t semaforo;

void* carro(void *arg) {
    int id = *(int*)arg;

    printf("Carro %d: tentando entrar...\n", id);
    sem_wait(&semaforo);

    printf("Carro %d: entrou na vaga.\n", id);
    sleep(2); // simula tempo usando o recurso
    printf("Carro %d: saiu da vaga.\n", id);

    sem_post(&semaforo);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    sem_init(&semaforo, 0, VAGAS);

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, carro, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&semaforo);

    return 0;
}