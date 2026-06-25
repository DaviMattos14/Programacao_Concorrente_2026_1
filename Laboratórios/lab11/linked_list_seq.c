#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <omp.h>

#define MAX 100
int resultados[MAX];

typedef struct No {
    int valor;
    struct No *prox;
} node;

void insere_no_fim(node **ptcab, int novo_valor) {
    node *novo_no = (node *) malloc(sizeof(node));
    if (!novo_no) { perror("malloc"); exit(EXIT_FAILURE); }
    novo_no->valor = novo_valor;
    novo_no->prox  = NULL;

    if (*ptcab == NULL) {       
        *ptcab = novo_no;
        return;
    }

    node *p = *ptcab;
    while (p->prox != NULL)     
        p = p->prox;
    p->prox = novo_no;
}


void quadrado(node *ptcab) {
    int idx = 0;
    while (ptcab != NULL) {
        resultados[idx++] = ptcab->valor * ptcab->valor;
        ptcab = ptcab->prox;
    }
}

void imprimir_resultados(int *resultados) {
    printf("Resultados: ");
    for (int i = 0; i < MAX; i++) {
        printf("%d ", resultados[i]);
    }
    printf("\n");
}

void imprime_lista(node *ptcab) {
    while (ptcab != NULL) {
        printf("%d ", ptcab->valor);
        ptcab = ptcab->prox;
    }
    printf("\n");
}

void liberar_lista(node *ptcab) {
    node *aux;
    while (ptcab != NULL) {
        aux   = ptcab->prox;
        free(ptcab);
        ptcab = aux;
    }
}


int main(void) {
    node *head = NULL;

    for (int i = 1; i <= MAX; i++)
        insere_no_fim(&head, i); 

    double inicio = omp_get_wtime();
    printf("Original:  ");
    imprime_lista(head);

    quadrado(head);

    printf("Quadrados: ");
    imprimir_resultados(resultados);

    double fim = omp_get_wtime();
    printf("Tempo de exec: %f segundos\n", fim - inicio);

    liberar_lista(head);
    return 0;
}