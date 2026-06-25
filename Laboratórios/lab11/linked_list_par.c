#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <omp.h>
#define MAX 100

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

void imprimir_resultados(int *resultados) {
    printf("Resultados: ");
    for (int i = 0; i < MAX; i++) {
        printf("%d ", resultados[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]){

    int grain = 4;

    if (argc > 1) grain = atoi(argv[1]);
    node *head = NULL;

    for (int i = 1; i <= MAX; i++)
        insere_no_fim(&head, i);
    

    int resultados[MAX];


    double tinicio = omp_get_wtime();
    #pragma omp parallel num_threads(4)
    {
        #pragma omp single
        {
            node *atual = head;
            int index = 0;

            while (atual != NULL) {
                #pragma omp task firstprivate(atual, index)
                {
                    resultados[index] = atual->valor * atual->valor;
                }
                atual = atual->prox;
                index++;
            }
            #pragma omp taskwait
            imprime_lista(head);
        }
    }
    imprimir_resultados(resultados);
    liberar_lista(head);
    double tfim = omp_get_wtime();
    printf("Tempo de exec: %f segundos\n", tfim - tinicio);

    return 0;
}