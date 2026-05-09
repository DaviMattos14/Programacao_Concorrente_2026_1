#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {

    printf("Processo pai (PID: %d) iniciando...\n", getpid());

    for (int i = 0; i < 3; i++) {

        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Erro no fork\n");
            return 1;
        }

        else if (pid == 0) {
            // FILHO
            printf("Sou o filho (PID: %d)\n", getpid());
            printf("Processo Filho (%d) terminou.\n", getpid());
            exit(0); // filho termina aqui
        }

        // apenas o pai chega aqui
    }

    //printf("Sou o processo pai (PID: %d). Esperando os filhos...\n", getpid());
    // pai espera todos os filhos
    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }

    printf("Pai terminou.\n");

    return 0;
}