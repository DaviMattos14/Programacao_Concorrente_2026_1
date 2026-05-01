#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Variaveis globais */
int x = 0;
pthread_mutex_t x_mutex;
pthread_cond_t x_cond;

/* Thread A (Imprime Hello) */
void *A(void *t)
{
    printf("HELLO\n");

    pthread_mutex_lock(&x_mutex);
    x++; // incrementa contador de hellos
    if (x == 2)
    { // se for o segundo hello, acorda quem espera
        pthread_cond_broadcast(&x_cond);
    }
    pthread_mutex_unlock(&x_mutex);

    pthread_exit(NULL);
}

/* Thread B (Espera 2 Hellos para imprimir Bye) */
void *B(void *t)
{
    pthread_mutex_lock(&x_mutex);
    while (x < 2)
    {
        pthread_cond_wait(&x_cond, &x_mutex);
    }
    printf("BYEBYE\n");
    pthread_mutex_unlock(&x_mutex);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    pthread_t threads[4];

    pthread_mutex_init(&x_mutex, NULL);
    pthread_cond_init(&x_cond, NULL);

    /* Criar 2 threads A e 2 thread B */
    pthread_create(&threads[0], NULL, A, NULL);
    pthread_create(&threads[1], NULL, A, NULL);
    pthread_create(&threads[2], NULL, B, NULL);
    pthread_create(&threads[3], NULL, B, NULL);

    for (int i = 0; i < 4; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&x_mutex);
    pthread_cond_destroy(&x_cond);
    return 0;
}