#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
int main()
{
    #pragma omp parallel num_threads(4)
    {
        #pragma omp task
        printf("Tarefa A\n");
        #pragma omp task
        printf("Tarefa B\n");
        // A e B podem executar em qualquer ordem, em threads diferentes
    }
}