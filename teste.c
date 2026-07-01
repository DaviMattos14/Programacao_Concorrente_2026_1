#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
int main() {
    #pragma omp parallel num_threads(6)
    {
        printf("Hello, World!\n");
    }
    return 0;
}