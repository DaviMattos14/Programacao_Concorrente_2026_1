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

int main(void){
    int resultados_seq[MAX];
    int resultados_par[MAX];
    node *head = NULL;

    for (int i = 1; i <= MAX; i++)
        insere_no_fim(&head, i);

    printf("Original:  ");
    imprime_lista(head);

    int granularidade[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    FILE *arquivo_resultados = fopen("resultados.txt", "w");
    if (arquivo_resultados == NULL) {
        perror("Erro ao criar resultados.txt");
        liberar_lista(head);
        return EXIT_FAILURE;
    }

    for(int g = 0; g < 15; g++) {
        printf("Executando com granularidade: %d\n", granularidade[g]);
        int grain = granularidade[g];

        // SEQUENCIAL
        double seq_inicio = omp_get_wtime();
        node *atual = head;
        int idx = 0;
        while (atual != NULL) {
            usleep(grain); // Simula trabalho
            resultados_seq[idx++] = atual->valor * atual->valor;
            atual = atual->prox;
        }
        //imprimir_resultados(resultados_seq);
        double seq_fim = omp_get_wtime();

        // PARALELO
        double par_inicio = omp_get_wtime();
        #pragma omp parallel num_threads(4)
        {
            #pragma omp single
            {
                node *atual = head;
                int index = 0;
                while (atual != NULL) {
                    #pragma omp task firstprivate(atual, index)
                    {
                        usleep(grain); // Simula trabalho
                        resultados_par[index] = atual->valor * atual->valor;
                    }
                    atual = atual->prox;
                    index++;
                }
                #pragma omp taskwait
                //imprimir_resultados(resultados_par);
            }
        }
        double par_fim = omp_get_wtime();

        fprintf(arquivo_resultados, "Granularidade: %d\n", grain);
        fprintf(arquivo_resultados, "Tempo de exec sequencial: %f segundos\n", seq_fim - seq_inicio);
        fprintf(arquivo_resultados, "Tempo de exec paralelo: %f segundos\n", par_fim - par_inicio);
        fprintf(arquivo_resultados, "Speedup: %f\n", (seq_fim - seq_inicio) / (par_fim - par_inicio));
        fprintf(arquivo_resultados, "Eficiencia: %f\n", ((seq_fim - seq_inicio) / (par_fim - par_inicio)) / 4.0);
        fprintf(arquivo_resultados, "----------------------------------------\n");
        fprintf(arquivo_resultados, "\n");
    }

    fclose(arquivo_resultados);
    liberar_lista(head);
    return EXIT_SUCCESS;
}
