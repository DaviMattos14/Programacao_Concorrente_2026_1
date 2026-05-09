#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 3

pthread_mutex_t mutex;
pthread_cond_t cond;
int chegou = 0;
void *task(void *arg)
{

    int id = (int)arg;
    pthread_mutex_lock(&mutex);
    //sleep(2);
    chegou++;
    printf("Thread %d terminou passo 1\n", id);
    if (chegou == NUM_THREADS)
    {
        pthread_cond_broadcast(&cond);
    }
    else{
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    //sleep(2);
    printf("Thread %d terminou\n", id);
    return NULL;
}

int main()
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_create(&threads[i], NULL, task, (void *)i))
        {
            fprintf(stderr, "Error\n");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}