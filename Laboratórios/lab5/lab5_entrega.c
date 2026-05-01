#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

long int soma = 0;                    // variavel compartilhada entre as threads
bool acabou = false;                  // flag para indicar o fim da execucao das threads de tarefa
bool somando = true;                  // flag para indicar o fim da execucao das threads de tarefa
pthread_mutex_t mutex;                // variavel de lock para exclusao mutua
pthread_cond_t cond_soma, cond_print; // variavel de condicao para sincronizacao

// funcao executada pelas threads
void *ExecutaTarefa(void *arg)
{
    int id = *(int *)arg;
    printf("Thread : %d esta executando...\n", id);

    for (int i = 0; i < 100000; i++)
    {
        pthread_mutex_lock(&mutex);
        while (!somando)
        {
            pthread_cond_wait(&cond_soma, &mutex);
        }

        soma++;

        if (soma % 1000 == 0)
        {
            somando = false;
            pthread_cond_signal(&cond_print);

            while (!somando)
            {
                pthread_cond_wait(&cond_soma, &mutex);
            }
        }

        pthread_mutex_unlock(&mutex);
    }
    printf("Thread : %d terminou!\n", id);
    pthread_exit(NULL);
}

// funcao executada pela thread de log
void *extra(void *args)
{
    printf("Extra : esta executando...\n");
    pthread_mutex_lock(&mutex);
    while (1)
    {
        while (somando && !acabou)
        {
            pthread_cond_wait(&cond_print, &mutex);
        }

        if (acabou)
            break;

        printf("Extra : soma = %ld\n", soma);
        somando = true;
        pthread_cond_broadcast(&cond_soma);
    }
    pthread_mutex_unlock(&mutex);

    printf("Extra : terminou!\n");
    pthread_exit(NULL);
}

// fluxo principal
int main(int argc, char *argv[])
{
    pthread_t *tid; // identificadores das threads no sistema
    int nthreads;   // qtde de threads (passada linha de comando)

    //--le e avalia os parametros de entrada
    if (argc < 2)
    {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);
    int id[nthreads];

    //--aloca as estruturas
    tid = (pthread_t *)malloc(sizeof(pthread_t) * (nthreads + 1));
    if (tid == NULL)
    {
        puts("ERRO--malloc");
        return 2;
    }

    //--inicilaiza o mutex (lock de exclusao mutua)
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_soma, NULL);
    pthread_cond_init(&cond_print, NULL);

    //--cria as threads
    for (int t = 0; t < nthreads; t++)
    {
        id[t] = t;
        if (pthread_create(&tid[t], NULL, ExecutaTarefa, &id[t]))
        {
            printf("--ERRO: pthread_create()\n");
            exit(-1);
        }
    }

    //--cria thread de log
    if (pthread_create(&tid[nthreads], NULL, extra, NULL))
    {
        printf("--ERRO: pthread_create()\n");
        exit(-1);
    }

    //--espera as workers terminarem
    for (int t = 0; t < nthreads; t++)
    {
        if (pthread_join(tid[t], NULL))
        {
            printf("--ERRO: pthread_join() \n");
            exit(-1);
        }
    }

    // sinaliza que as workers terminaram
    pthread_mutex_lock(&mutex);
    acabou = true;
    pthread_cond_broadcast(&cond_print); // acorda extra para sair do loop
    pthread_mutex_unlock(&mutex);

    //--espera a thread de log terminar
    if (pthread_join(tid[nthreads], NULL))
    {
        printf("--ERRO: pthread_join() \n");
        exit(-1);
    }

    //--finaliza o mutex
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_soma);
    pthread_cond_destroy(&cond_print);

    printf("Valor de 'soma' = %ld\n", soma);

    return 0;
}