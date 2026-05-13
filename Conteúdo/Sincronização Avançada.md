# Guia Técnico de Programação Paralela com Pthreads: Sincronização Avançada

Este documento fornece uma visão técnica e prática sobre os mecanismos de sincronização em sistemas de memória compartilhada utilizando a biblioteca Pthreads em C. Ele abrange Variáveis de Condição, Barreiras e Semáforos, servindo como referência para desenvolvimento e estudo.

--------------------------------------------------------------------------------

# 1. Variáveis de Condição

As Variáveis de Condição são mecanismos de sincronização que permitem que uma thread suspenda sua execução até que uma condição lógica específica se torne verdadeira.

### Definição

São variáveis especiais (tipo `pthread_cond_t`) que servem como filas de espera para threads que aguardam por mudanças no estado de dados compartilhados.

### Objetivo

Resolver o problema da espera ocupada (_busy waiting_). Em vez de uma thread testar repetidamente uma condição em um loop, consumindo CPU, ela se bloqueia e libera o processador até ser notificada de que pode prosseguir.

### Funcionamento

- **Comportamento na Memória:** Trabalham sempre em conjunto com um **Mutex**. O mutex protege a variável que representa a condição lógica.
- **Sincronização:** Quando uma thread chama `wait`, ela libera o mutex atomicamente e entra em estado de bloqueio.
- **Execução:** Quando outra thread altera a condição e sinaliza (`signal` ou `broadcast`), a thread bloqueada é acordada.
- **Retomada:** Ao acordar, a thread **readquire obrigatoriamente o mutex** antes de continuar a execução.

### Sintaxe

```c
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_destroy(pthread_cond_t *cond);
```

### Parâmetros

- `cond`: Ponteiro para a variável de condição.
- `attr`: Atributos da variável (geralmente `NULL` para padrões).
- `mutex`: O mutex associado que protege a condição lógica.

### Valor de Retorno

As funções retornam `0` em caso de sucesso e um código de erro em caso de falha.

### Exemplo Completo: Produtor e Consumidor

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* Inicialização estática de Mutex e Condição */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int dado_pronto = 0;
int dado = 0;

void *produtor(void *arg) {
    sleep(2); /* Simula processamento */
    
    pthread_mutex_lock(&mutex);
    dado = 42;
    dado_pronto = 1;
    printf("Produtor: dado pronto = %d\n", dado);
    
    pthread_cond_signal(&cond); /* Acorda uma thread na fila */
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

void *consumidor(void *arg) {
    pthread_mutex_lock(&mutex);
    
    /* Sempre use while para verificar a condição */
    while (dado_pronto == 0) {
        printf("Consumidor: aguardando...\n");
        pthread_cond_wait(&cond, &mutex); /* Libera mutex e bloqueia */
    }
    
    printf("Consumidor: recebeu dado = %d\n", dado);
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main(void) {
    pthread_t t_prod, t_cons;

    pthread_create(&t_cons, NULL, consumidor, NULL);
    pthread_create(&t_prod, NULL, produtor, NULL);

    pthread_join(t_prod, NULL);
    pthread_join(t_cons, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
```

### Explicação do Código

1. **Consumidor** inicia, trava o mutex e verifica `dado_pronto`. Como é 0, ele executa `pthread_cond_wait`.
2. Internamente, o `wait` libera o mutex e coloca a thread para dormir.
3. **Produtor** trava o mutex (que foi liberado pelo consumidor), altera o `dado` e `dado_pronto`.
4. O **Produtor** chama `signal`, notificando a fila, e então libera o mutex.
5. O **Consumidor** acorda, readquire o mutex automaticamente, sai do `wait`, revalida o `while` e prossegue para imprimir o dado.

### Saída Esperada

```text
Consumidor: aguardando...
Produtor: dado pronto = 42
Consumidor: recebeu dado = 42
```

### Possíveis Problemas e Boas Práticas

- **Problema:** Esquecer de usar `while` para testar a condição (usar `if`). Se houver um "despertar espúrio", a thread pode prosseguir com a condição ainda falsa.
- **Boa Prática:** Sempre manter o mutex travado ao chamar `wait` e garantir que o sinal seja enviado após a alteração da variável lógica.

--------------------------------------------------------------------------------

# 2. Sincronização por Barreira

A barreira atua como um ponto de encontro coletivo para um grupo de threads.

### Definição

É um ponto de sincronização onde todas as threads de um grupo devem chegar antes que qualquer uma possa prosseguir.

### Objetivo

Coordenar algoritmos paralelos que operam em etapas (fases). Garante que a etapa N seja concluída por todos antes que a etapa N+1 comece.

### Funcionamento

- **Contagem:** A barreira é inicializada com um valor `count`.
- **Bloqueio:** Cada thread que executa `wait` fica bloqueada na barreira.
- **Liberação:** Quando a última thread (a de número `count`) chega à barreira, todas as threads são desbloqueadas simultaneamente.

### Sintaxe

```c
int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned count);
int pthread_barrier_wait(pthread_barrier_t *barrier);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
```

### Exemplo Completo: Método de Jacobi (Simplificado)

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define N 4
#define NTHREADS 2

double x[N], x_new[N];
pthread_barrier_t barrier;

typedef struct { int id; int start; int end; } thread_data_t;

void* jacobi_thread(void* arg) {
    thread_data_t* data = (thread_data_t*) arg;
    
    for (int k = 0; k < 2; k++) { /* Simulação de 2 iterações */
        /* Etapa 1: Cálculo */
        for (int i = data->start; i < data->end; i++) {
            x_new[i] = x[i] + 1.0; /* Cálculo simplificado */
        }
        
        /* Barreira 1: Espera todos calcularem x_new */
        pthread_barrier_wait(&barrier);

        /* Etapa 2: Atualização */
        for (int i = data->start; i < data->end; i++) {
            x[i] = x_new[i];
        }

        /* Barreira 2: Espera todos atualizarem x */
        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

int main() {
    pthread_t threads[NTHREADS];
    thread_data_t data[NTHREADS];
    pthread_barrier_init(&barrier, NULL, NTHREADS);

    int chunk = N / NTHREADS;
    for (int t = 0; t < NTHREADS; t++) {
        data[t].id = t;
        data[t].start = t * chunk;
        data[t].end = (t + 1) * chunk;
        pthread_create(&threads[t], NULL, jacobi_thread, &data[t]);
    }

    for (int t = 0; t < NTHREADS; t++) pthread_join(threads[t], NULL);
    
    printf("Resultado: x[0] = %f\n", x[0]);
    pthread_barrier_destroy(&barrier);
    return 0;
}
```

### Explicação do Código

O algoritmo de Jacobi exige que todos os novos valores de x sejam calculados antes de atualizar o vetor original. A barreira garante que nenhuma thread comece a fase de atualização antes que as outras terminem seus cálculos locais.

--------------------------------------------------------------------------------

# 3. Semáforos

Semáforos são ferramentas poderosas de sincronização baseadas em contadores atômicos.

### Definição

Variáveis inteiras acessadas via operações atômicas `wait` (P) e `signal/post` (V).

### Tipos de Semáforos

1. **Binário:** Valor 0 ou 1. Funciona de forma similar a um mutex.
2. **Contador:** Valor de 0 a N. Controla o acesso a múltiplas instâncias de um recurso.
3. **Nomeado:** Global ao sistema, acessado por um nome (ex: `/meu_sem`). Útil para processos independentes.
4. **Não Nomeado:** Existe na memória de um processo. Útil para threads.

### Funcionamento

- `**sem_wait()**`**:** Decrementa o semáforo. Se o valor for 0, a thread bloqueia.
- `**sem_post()**`**:** Incrementa o semáforo. Se houver threads esperando, acorda uma delas.

### Sintaxe (POSIX)

```c
#include <semaphore.h>

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_destroy(sem_t *sem);
```

### Exemplo Completo: Controle de Vagas (Estacionamento)

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_THREADS 4
#define VAGAS 2

sem_t semaforo;

void* carro(void *arg) {
    int id = *(int*)arg;
    printf("Carro %d: tentando entrar...\n", id);
    
    sem_wait(&semaforo); /* Adquire uma vaga */
    printf("Carro %d: ENTROU na vaga.\n", id);
    sleep(1);
    printf("Carro %d: SAIU da vaga.\n", id);
    sem_post(&semaforo); /* Libera a vaga */
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    sem_init(&semaforo, 0, VAGAS); /* Inicializa com 2 vagas */

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, carro, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);
    sem_destroy(&semaforo);
    return 0;
}
```

### Saída Esperada (Exemplo)

```text
Carro 1: tentando entrar...
Carro 1: ENTROU na vaga.
Carro 2: tentando entrar...
Carro 2: ENTROU na vaga.
Carro 3: tentando entrar...
Carro 1: SAIU da vaga.
Carro 3: ENTROU na vaga.
...
```

--------------------------------------------------------------------------------

# Comparativo e Resumo de Problemas

|   |   |   |
|---|---|---|
|Recurso|Principal Uso|Diferencial|
|**Mutex**|Exclusão Mútua|Apenas uma thread por vez na seção crítica.|
|**Var. Condição**|Sincronização Lógica|Faz a thread esperar "um evento" sem gastar CPU.|
|**Barreira**|Sincronização de Fases|Ponto de encontro para algoritmos iterativos.|
|**Semáforo**|Gestão de Recursos|Permite N acessos simultâneos.|

### Problemas Comuns em Programação Paralela

- **Condição de Corrida:** Quando o resultado depende da ordem imprevista de execução das threads.
- **Deadlock:** Duas ou mais threads esperam indefinidamente por recursos detidos uma pela outra.
- **Inversão de Prioridade:** Uma thread de baixa prioridade detém um semáforo, impedindo uma de alta prioridade de executar.
- **Starvation (Inanição):** Uma thread nunca consegue acesso ao recurso pois outras sempre passam à frente.

### Boas Práticas

1. **Sempre inicialize** e **destrua** seus objetos de sincronização (mutex, cond, barrier, sem).
2. **Minimize a Seção Crítica:** Mantenha o código entre `lock` e `unlock` o menor possível.
3. **Ordem de Lock:** Se precisar de múltiplos mutexes, adquira-os sempre na mesma ordem em todas as threads para evitar deadlocks.
4. **Uso de** `**while**` **em Condições:** Nunca confie que um despertar de variável de condição significa que a condição é verdadeira; revalide-a.

### Erros Frequentes em Provas e Exercícios

- Não passar o endereço do objeto para a função (ex: usar `sem_wait(sem)` em vez de `sem_wait(&sem)`).
- Esquecer que `pthread_cond_wait` exige um mutex já travado como parâmetro.
- Tentar usar `pthread_barrier_t` sem definir o número correto de threads no `init`.
- Confundir semáforo binário com mutex (embora similares, semáforos não têm conceito de "dono", um mutex só pode ser destravado por quem o travou).