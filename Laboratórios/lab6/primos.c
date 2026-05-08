#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>

int *buffer;
sem_t vazio, cheio, mutex;
int posInsere = 0, posRetira = 0, consumido = 0;
int n, m;

int ehPrimo(long long int n)
{
    int i;
    if (n <= 1)
        return 0;
    if (n == 2)
        return 1;
    if (n % 2 == 0)
        return 0;
    for (i = 3; i < sqrt(n) + 1; i += 2)
        if (n % i == 0)
            return 0;
    return 1;
}

void insere(int elemento)
{
    buffer[posInsere] = elemento;
    posInsere = (posInsere + 1) % m;
}

int retira()
{
    int elemento = buffer[posRetira];
    posRetira = (posRetira + 1) % m;
    return elemento;
}

void *produtor(void *arg)
{
    int i;
    for (i = 0; i < n; i++)
    {
        sem_wait(&vazio);
        sem_wait(&mutex);
        insere(i);
        sem_post(&mutex);
        sem_post(&cheio);
    }
    sem_wait(&vazio);
    sem_wait(&mutex);
    buffer[posInsere] = -1;
    posInsere = (posInsere + 1) % m;
    sem_post(&mutex);
    sem_post(&cheio);
    pthread_exit(NULL);
}

void *consumidor(void *arg)
{
    int id = *((int *)arg);

    int *qtdPrimos = malloc(sizeof(int));
    *qtdPrimos = 0;

    while (1)
    {
        sem_wait(&cheio);
        sem_wait(&mutex);

        int elemento = retira();

        if (elemento == -1)
        {
            sem_post(&mutex);
            sem_post(&cheio);
            break;
        }

        consumido++;

        sem_post(&mutex);
        sem_post(&vazio);

        if (ehPrimo(elemento))
        {
            (*qtdPrimos)++;
        }
    }

    pthread_exit((void *)qtdPrimos);
}

int main(int argc, char *argv[])
{
    if (scanf("%d %d", &n, &m) != 2)
    {
        fprintf(stderr, "Erro. \n");
        return 1;
    }

    int vencedor = -1, qtdPrimoVencedor = 0, qtdTotalPrimos = 0;

    buffer = (int *)malloc(m * sizeof(int));
    pthread_t prod;
    pthread_t cons;

    sem_init(&vazio, 0, m);
    sem_init(&cheio, 0, 0);
    sem_init(&mutex, 0, 1);

    if (pthread_create(&prod, NULL, produtor, (void *)&n))
        exit(1);
    if (pthread_create(&cons, NULL, consumidor, (void *)&m))
        exit(1);

    
    pthread_join(prod, NULL);
    void *retorno;
    pthread_join(cons, &retorno);
    qtdTotalPrimos += *((int *)retorno);
    

    free(retorno);

    printf("Total de primos: %d\n", qtdTotalPrimos);

    sem_destroy(&vazio);
    sem_destroy(&cheio);
    sem_destroy(&mutex);
    free(buffer);
}