# Programação Paralela com Pthreads em C: Guia Técnico de Referência

Este documento provê uma síntese técnica exaustiva sobre a utilização da biblioteca **POSIX Threads (pthreads)** em linguagem C, abordando desde a criação básica de threads até estratégias avançadas de comunicação e sincronização.

--------------------------------------------------------------------------------

## Preparação do Ambiente e Compilação

Para utilizar a API Pthreads, é obrigatória a inclusão do cabeçalho `<pthread.h>`. Durante a compilação, o vinculador (linker) deve ser instruído a incluir a biblioteca pthreads através da flag `-lpthread`.

**Comando de Compilação:** `gcc programa.c -o programa -lpthread`

--------------------------------------------------------------------------------

## pthread_create

### Definição

É a função fundamental responsável por criar uma nova unidade de execução (thread) dentro do processo atual.

### Objetivo

Permitir a execução paralela de funções, resolvendo o problema da limitação de fluxo único de controle. Permite que o programa realize múltiplas tarefas simultaneamente, aproveitando arquiteturas multinúcleo.

### Funcionamento

- **Memória:** A nova thread compartilha o espaço de endereçamento (variáveis globais, heap), mas possui sua própria pilha (stack) para variáveis locais.
- **Sincronização:** A criação é assíncrona; a thread chamadora continua sua execução imediatamente após a chamada.
- **Execução:** O sistema operacional escalona a thread para executar a função de entrada (`start_routine`).
- **Riscos:** Se o processo principal (main) terminar antes das threads filhas sem o devido tratamento, todas as threads são destruídas abruptamente.

### Sintaxe

```c
int pthread_create(
    pthread_t *thread,
    const pthread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg
);
```

### Parâmetros

- **thread**: Ponteiro para uma variável do tipo `pthread_t` onde o ID da thread será armazenado.
- **attr**: Ponteiro para atributos da thread. Usa-se `NULL` para configurações padrão (tamanho de pilha, escalonamento).
- **start_routine**: Endereço da função que a thread executará. Deve ter o protótipo `void *func(void *)`.
- **arg**: Ponteiro `void *` para o argumento único da função.

### Valor de Retorno

Retorna `0` em caso de sucesso. Em caso de falha, retorna um código de erro numérico e o ID da thread em `*thread` é indefinido.

### Exemplo Completo (Passagem de Struct)

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    int valor;
} ThreadData;

void *tarefa(void *arg) {
    /* Cast do argumento para o tipo correto */
    ThreadData *data = (ThreadData *)arg;
    printf("Thread %d executando com valor %d\n", data->id, data->valor);
    return NULL;
}

int main() {
    pthread_t thread;
    ThreadData td;

    td.id = 1;
    td.valor = 100;

    /* Criando a thread */
    if (pthread_create(&thread, NULL, tarefa, &td) != 0) {
        perror("Falha ao criar thread");
        return 1;
    }

    /* Aguarda a finalização para evitar fim do processo */
    pthread_join(thread, NULL);
    return 0;
}
```

### Explicação do Código

1. **Linhas 5-8:** Define uma `struct` para encapsular múltiplos parâmetros.
2. **Linha 10:** A função `tarefa` segue a assinatura exigida pela API.
3. **Linha 12:** Realiza-se o _cast_ de `void *` para `ThreadData *` para acessar os membros.
4. **Linha 25:** `pthread_create` inicia a thread passando o endereço da struct `&td`.
5. **Linha 31:** `pthread_join` garante que a `main` espere a thread terminar.

### Saída Esperada

`Thread 1 executando com valor 100`

### Possíveis Problemas

- **Condição de Corrida (Race Condition):** Ocorre se múltiplas threads tentarem ler/escrever no mesmo endereço de memória (como o `&td`) simultaneamente sem sincronização.
- **Inconsistência de Memória:** Se a variável `td` for local e a função `main` sair do escopo antes da thread ler os dados, a thread acessará memória inválida.

### Boas Práticas

- Sempre verifique o valor de retorno de `pthread_create`.
- Para múltiplos argumentos, utilize estruturas (structs).
- Evite passar endereços de variáveis que mudam rapidamente (como contadores de laço `for`).

### Erros Frequentes em Provas

- Esquecer a flag `-lpthread`.
- Passar o endereço da variável de controle do laço `for` (`&i`), fazendo com que todas as threads leiam o valor final de `i`.

--------------------------------------------------------------------------------

## pthread_join

### Definição

Função de sincronização que bloqueia a thread chamadora até que a thread alvo termine sua execução.

### Objetivo

Resolver a necessidade de sincronização entre threads e permitir a coleta de resultados. Funciona como o `wait()` de processos, evitando que recursos fiquem retidos em "zombie threads".

### Funcionamento

- **Sincronização:** A thread que chama `pthread_join` suspende sua execução.
- **Memória:** Quando a thread alvo termina (via `return` ou `pthread_exit`), o sistema operacional retém seu valor de retorno. O `pthread_join` copia esse endereço para a variável da thread chamadora.
- **Recursos:** Após o join, os recursos da thread (pilha, estruturas internas) são liberados pelo sistema.

### Sintaxe

```c
int pthread_join(
    pthread_t thread,
    void **retval
);
```

### Parâmetros

- **thread**: O identificador (ID) da thread a ser aguardada.
- **retval**: Ponteiro para ponteiro (`void **`). É o endereço onde o valor de retorno da thread alvo será escrito.

### Valor de Retorno

Retorna `0` em sucesso; caso contrário, um código de erro (ex: se a thread já recebeu join ou não é "joinable").

### Exemplo Completo (Retorno via Heap)

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *calcular_quadrado(void *arg) {
    int n = *(int *)arg;
    int *resultado = malloc(sizeof(int)); /* Alocação no Heap */
    *resultado = n * n;
    return (void *)resultado;
}

int main() {
    pthread_t tid;
    int numero = 5;
    void *ret; /* Ponteiro para receber o endereço do resultado */

    pthread_create(&tid, NULL, calcular_quadrado, &numero);

    /* O Join recebe o endereço do ponteiro 'ret' */
    pthread_join(tid, &ret);

    printf("Resultado retornado: %d\n", *(int *)ret);
    
    free(ret); /* Obrigatório liberar memória alocada pela thread */
    return 0;
}
```

### Explicação do Código

1. **Linha 7:** Aloca-se memória no **heap** para o resultado. Memória local (pilha) seria destruída ao fim da função.
2. **Linha 18:** `void *ret` é declarada para armazenar o endereço que a thread devolverá.
3. **Linha 21:** `pthread_join` recebe `&ret`. Internamente, a biblioteca localiza o valor retornado pela thread (ex: `0x500`) e sobrescreve o valor de `ret` com esse endereço.
4. **Linha 25:** Libera-se a memória para evitar _memory leak_.

### Saída Esperada

`Resultado retornado: 25`

### Possíveis Problemas

- **Zumbi (Zombie Thread):** Threads que terminam mas não recebem `join` mantêm recursos ocupados.
- **Dangling Pointer:** Retornar o endereço de uma variável local da thread. O ponteiro resultante no `join` apontará para memória inválida.

### Boas Práticas

- Sempre execute `pthread_join` ou `pthread_detach` para cada thread criada.
- Se a thread alocou memória no heap para o retorno, a responsabilidade pelo `free` é da thread que chamou o `join`.

--------------------------------------------------------------------------------

## pthread_exit e pthread_detach

### Resumo de Funções Auxiliares

|   |   |   |
|---|---|---|
|Função|O que é|Quando usar|
|**pthread_exit**|Termina a thread atual sem encerrar o processo.|Quando uma thread termina e quer retornar um valor ou quando a `main` deve esperar as filhas sem bloquear.|
|**pthread_detach**|Desvincula a thread, liberando recursos automaticamente ao fim.|Para threads de serviço (logging, monitoramento) das quais não se espera resultado.|

**Nota sobre** `**pthread_exit**` **na Main:** Se a função `main` chamar `pthread_exit`, o fluxo principal termina, mas o processo permanece vivo até que todas as threads filhas terminem.

--------------------------------------------------------------------------------

## Estratégias de Comunicação: Struct Bidirecional

Esta é a abordagem recomendada para escalabilidade e segurança.

### Vantagens

- Encapsula entrada e saída.
- Evita múltiplos `malloc/free`.
- Sem condições de corrida se o acesso ocorrer após o `pthread_join`.

### Exemplo de Implementação

```c
typedef struct {
    /* Entrada */
    int a, b;
    /* Saída */
    int soma;
} Dados;

void *somar(void *arg) {
    Dados *d = (Dados *)arg;
    d->soma = d->a + d->b;
    return NULL;
}

/* No Main: */
/* Dados d[10]; pthread_create(... &d[i]); pthread_join(tid[i], NULL); */
```

--------------------------------------------------------------------------------

## Comparação: Execução Sequencial vs. Paralela

|   |   |   |
|---|---|---|
|Característica|Execução Sequencial|Execução Paralela (Pthreads)|
|**Desempenho**|Limitado a um núcleo.|Pode utilizar múltiplos núcleos.|
|**Overhead**|Mínimo.|Criação e sincronização de threads consomem tempo e memória.|
|**Complexidade**|Baixa.|Alta (risco de deadlocks e race conditions).|
|**Determinismo**|Garantido.|Não determinístico (depende do escalonador do SO).|

**Conclusão:** O paralelismo é vantajoso para tarefas intensivas em processamento (CPU-bound) ou I/O, desde que o ganho de velocidade supere o overhead de gerenciamento das threads.