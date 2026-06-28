#include <stdio.h>
#include <omp.h>

int main()
{
    int i, n = 1000;
    int soma = 0;
    int vetor[1000];

    for (i = 0; i < n; i++)
        vetor[i] = 1;

#pragma omp parallel for reduction(+ : soma)
    for (i = 0; i < n; i++)
        soma += vetor[i];

    printf("Resultado total da soma: %d\n", soma);

    return 0;
}