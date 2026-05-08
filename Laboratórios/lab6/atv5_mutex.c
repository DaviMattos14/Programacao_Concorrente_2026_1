#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS 6
#define VAGAS_TOTAIS 3

int vagas_livres = VAGAS_TOTAIS;
pthread_mutex_t mutex;
pthread_cond_t cond_vaga_disponivel;

void* carro(void *arg) {
    int id = *(int*)arg;

    printf("Carro %d: tentando entrar...\n", id);

    
    pthread_mutex_lock(&mutex);
    
    while (vagas_livres == 0) {
        printf("Carro %d: Aguardando vaga...\n", id);
        pthread_cond_wait(&cond_vaga_disponivel, &mutex);
    }
    
    vagas_livres--;
    printf("Carro %d: entrou na vaga. Vagas livres = %d\n", id, vagas_livres);
    
    pthread_mutex_unlock(&mutex);

    sleep(rand() % 4 + 1); 

    pthread_mutex_lock(&mutex);
    
    vagas_livres++;
    printf("Carro %d: saiu da vaga. Vagas livres = %d\n", id, vagas_livres);
    
    pthread_cond_broadcast(&cond_vaga_disponivel);
    
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    srand(time(NULL));
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_vaga_disponivel, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, carro, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_vaga_disponivel);

    return 0;
}