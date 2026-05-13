# Documentação Técnica: Programação Paralela e Sincronização com Pthreads

Este documento provê uma síntese técnica e didática sobre os fundamentos da programação concorrente utilizando a biblioteca Pthreads em linguagem C, com foco em comunicação via memória compartilhada, sincronização e balanceamento de carga.

--------------------------------------------------------------------------------

## 1. Seção Crítica (Critical Section)

### Definição

A seção crítica é um trecho de código que acessa uma variável ou recurso compartilhado que pode ser modificado por múltiplas threads simultaneamente.

### Objetivo

O objetivo de identificar e proteger uma seção crítica é evitar a **corrida de dados (data race)**, garantindo que operações sobre dados compartilhados sejam tratadas como ações atômicas.

### Funcionamento

- **Comportamento na Memória:** Em sistemas multiprocessadores, as threads compartilham o mesmo espaço de endereçamento. Quando uma thread altera uma variável global, essa alteração é visível para as outras.
- **Sincronização:** Para evitar inconsistências, a seção crítica deve ser cercada por uma "seção de entrada" (requisição de acesso) e uma "seção de saída" (liberação do acesso).
- **Riscos:** Sem proteção, instruções de máquina de threads diferentes podem se entrelaçar. Por exemplo, uma atribuição simples em C como `x = y + z` pode ser traduzida em múltiplas instruções de máquina (leitura, soma, escrita). Se houver interrupção entre essas etapas, o resultado final pode ser imprevisível.

### Requisitos de uma Seção Crítica Segura

1. **Exclusão Mútua:** Apenas uma thread por vez na seção crítica.
2. **Ausência de Inanição (Starvation):** Toda thread que deseja entrar deve conseguir em algum momento.
3. **Independência:** Threads fora da seção crítica não devem impedir outras de entrar nela.
4. **Garantia de Entrada:** Se ninguém está na seção crítica, o acesso deve ser imediato para quem solicita.

--------------------------------------------------------------------------------

## 2. Mutex (Exclusão Mútua com Locks)

### O que é

O Mutex (abreviação de _Mutual Exclusion_) é um mecanismo de sincronização por escalonamento que atua como um "cadeado" para proteger seções críticas.

### Para que serve

Serve para garantir que trechos de código que acessam objetos compartilhados não sejam executados simultaneamente por mais de uma thread, evitando inconsistência de memória.

### Quando usar

Deve ser usado sempre que houver acesso concorrente a uma variável global ou recurso compartilhado onde pelo menos uma das operações seja de escrita.

### Como funciona internamente

O Mutex possui uma thread proprietária. Quando uma thread executa a operação de _lock_, ela tenta obter a posse. Se o mutex já estiver ocupado, a thread é bloqueada pelo sistema operacional e retira-se da fila de execução, cedendo o processador até que o recurso seja liberado (_unlock_). Isso evita o desperdício de ciclos de CPU (diferente da "espera ocupada").

### Sintaxe

```c
pthread_mutex_t mutex; // Declaração

// Inicialização Estática
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Operações Básicas
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

### Explicação dos Parâmetros

- `pthread_mutex_t *mutex`: Ponteiro para a variável do tipo mutex que será operada.
- `PTHREAD_MUTEX_INITIALIZER`: Macro para inicialização rápida de mutexes com atributos padrão.

### Fluxo de Execução

1. **Thread A** chama `lock()`. O mutex está livre, ela entra na seção crítica.
2. **Thread B** chama `lock()`. O mutex está ocupado, Thread B é bloqueada.
3. **Thread A** executa o código e chama `unlock()`.
4. O sistema operacional acorda a **Thread B**, que agora obtém o lock e entra na seção crítica.

### Exemplo Completo em C (Soma de Vetor com Exclusão Mútua)

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 1000000
#define P 4

float soma_total = 0;
float a[N];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *somaGrupo(void *arg) {
    int id = *(int *)arg;
    int inicio = id * (N / P);
    int fim = (id == P - 1) ? N : inicio + (N / P);
    float soma_local = 0;

    /* Processamento independente */
    for (int i = inicio; i < fim; i++) {
        soma_local += a[i];
    }

    /* Seção Crítica protegida por Mutex */
    pthread_mutex_lock(&mutex);
    soma_total += soma_local;
    pthread_mutex_unlock(&mutex);

    free(arg);
    return NULL;
}

int main() {
    pthread_t threads[P];

    /* Inicializa vetor */
    for (int i = 0; i < N; i++) a[i] = 1.0;

    for (int i = 0; i < P; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i], NULL, somaGrupo, id);
    }

    for (int i = 0; i < P; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Soma total = %f\n", soma_total);
    pthread_mutex_destroy(&mutex);
    return 0;
}
```

### Explicação do Código

- **Linhas 1-12:** Definição de constantes, globais e inicialização do Mutex.
- **Linha 22:** Cada thread calcula sua `soma_local` de forma independente, sem necessidade de sincronização (paralelismo máximo).
- **Linhas 26-28:** O acesso à variável global `soma_total` é protegido. O `pthread_mutex_lock` garante que apenas uma thread adicione seu resultado parcial por vez.
- **Linha 42:** `pthread_create` dispara as threads.
- **Linha 47:** `pthread_join` garante que o programa principal espere todas as threads terminarem antes de imprimir o resultado.

### Saída Esperada

```text
Soma total = 1000000.000000
```

### Problemas Comuns

- **Deadlock (Impasse):** Ocorre quando threads ficam esperando umas pelas outras por locks que nunca serão liberados.
- **Inconsistência de Memória:** Esquecer de usar o lock em um trecho de escrita.
- **Esquecer de dar Unlock:** Se uma thread morre ou entra em loop infinito dentro da seção crítica, todas as outras threads que precisam do lock ficarão bloqueadas para sempre.

### Boas Práticas

- **Minimize a Seção Crítica:** Mantenha apenas o código essencial dentro do lock (ex: apenas a atualização da variável global, não o cálculo complexo).
- **Prefira Variáveis Locais:** Calcule tudo o que puder em variáveis locais antes de atualizar a global.
- **Sempre Destrua o Mutex:** Use `pthread_mutex_destroy` ao final para liberar recursos.

### Erros Frequentes em Provas

- Tentar dar `lock` em um mutex que a própria thread já possui (causa travamento, a menos que seja um lock recursivo).
- Não inicializar o mutex antes de usar.
- Chamar `unlock` em um mutex que a thread não possui.

--------------------------------------------------------------------------------

## 3. Estratégias de Paralelização e Balanceamento de Carga

O material identifica dois métodos principais de distribuição de tarefas:

### A. Particionamento Estático (Divisão em Blocos)

- **O que é:** O trabalho é dividido em partes iguais no início da execução.
- **Problema:** Pode gerar **desbalanceamento de carga**. Em problemas como a contagem de números primos, números maiores demoram muito mais para serem processados do que números menores.
- **Resultado:** Algumas threads terminam rápido e ficam ociosas, enquanto outras continuam trabalhando, reduzindo a eficiência.

### B. Saco de Tarefas (Task Bag / Particionamento Dinâmico)

- **O que é:** As tarefas não são pré-atribuídas. Existe um índice global (ou repositório de tarefas) que as threads consultam dinamicamente.
- **Como funciona:**
    1. A thread solicita uma tarefa (ex: pega o próximo número para testar).
    2. O acesso ao índice global é protegido por um **Mutex**.
    3. A thread processa a tarefa fora do lock.
    4. Ao terminar, a thread volta para buscar mais trabalho até que o "saco" esteja vazio.
- **Vantagem:** Balanceamento automático. Threads que pegam tarefas fáceis trabalham mais vezes, enquanto threads com tarefas pesadas fazem menos, mas todas terminam aproximadamente ao mesmo tempo.

### Comparação de Desempenho (Caso de Estudo: Números Primos)

|   |   |   |   |
|---|---|---|---|
|Estratégia|Threads|Speedup|Eficiência|
|**Blocos Estáticos**|6|4,00|0,67|
|**Saco de Tarefas**|6|4,79|0,80|

**Conclusão:** Mesmo com o overhead (custo extra) de usar um mutex para acessar o índice global no Saco de Tarefas, a eficiência é superior devido ao melhor aproveitamento dos processadores.

--------------------------------------------------------------------------------

## 4. Comparação: Execução Sequencial vs. Paralela

|   |   |   |
|---|---|---|
|Característica|Execução Sequencial|Execução Paralela|
|**Velocidade**|Limitada a um núcleo.|Pode ser até P vezes mais rápida.|
|**Complexidade**|Baixa.|Alta (exige sincronização).|
|**Overhead**|Nenhum.|Criação de threads e troca de contexto.|
|**Consistência**|Determinística.|Requer cuidado com condições de corrida.|

**Vantagens do Paralelismo:** Redução drástica no tempo de execução para grandes volumes de dados ou cálculos intensivos. **Overhead:** O custo de gerenciar threads e sincronizar acessos pode tornar o paralelismo menos eficiente se as tarefas forem pequenas demais ou se houver contenção excessiva pelo mutex.