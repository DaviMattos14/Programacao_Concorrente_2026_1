# Guia Técnico de Programação Paralela e Concorrente: Processos e Pthreads

Este documento sintetiza os fundamentos da programação paralela e concorrente, com foco na arquitetura de sistemas operacionais do tipo UNIX e na utilização da biblioteca POSIX Threads (pthreads) em linguagem C.

--------------------------------------------------------------------------------

## 1. Fundamentos da Execução: Sequencial, Concorrente e Paralela

### Definição

A computação moderna evoluiu de modelos estritamente lineares para sistemas que gerenciam múltiplos fluxos de execução simultaneamente.

- **Execução Sequencial:** As tarefas são executadas em ordem estrita; uma ação só inicia após o término da anterior.
- **Execução Concorrente:** Múltiplas tarefas estão em progresso simultaneamente através da intercalação temporal (time-sharing) em um único processador.
- **Execução Paralela:** Tarefas são executadas verdadeiramente ao mesmo tempo em diferentes unidades de processamento (múltiplos núcleos ou CPUs).

### Objetivo

Resolver problemas de desempenho, capturar a estrutura lógica de problemas naturalmente concorrentes (como interfaces gráficas ou servidores web) e compartilhar recursos de forma controlada.

### Comparação de Desempenho

|   |   |   |
|---|---|---|
|Modelo|Hardware|Comportamento|
|**Sequencial**|1 CPU|Uma tarefa por vez (T1 -> T2 -> T3).|
|**Concorrente**|1 CPU|Intercalação de fatias de tempo (_quantum_).|
|**Paralelo**|Múltiplas CPUs|Execução simultânea real.|

--------------------------------------------------------------------------------

## 2. Processos: A Unidade de Isolamento

### O que é

Um processo é uma entidade dinâmica que representa um programa em execução. Enquanto o programa é um arquivo estático no disco, o processo é o contêiner que armazena todas as informações necessárias para a execução pelo sistema operacional (SO).

### Funcionamento Interno

O SO gerencia os processos através do **Bloco de Controle de Processo (PCB)** ou Descritor de Processo.

- **Comportamento na Memória:** Cada processo possui seu próprio espaço de endereçamento virtual isolado.
- **Seções de Memória (Formato ELF):**
    - `.text`: Instruções do programa (apenas leitura).
    - `.rodata`: Dados somente de leitura (constantes).
    - `.data`: Dados estáticos inicializados (leitura/escrita).
    - `.bss`: Dados estáticos sem valor inicial.
    - **Heap:** Espaço para alocação dinâmica (`malloc`). Cresce "para cima".
    - **Stack (Pilha):** Armazena variáveis locais, parâmetros de funções e endereços de retorno. Cresce "para baixo".

### Estados de um Processo

1. **Novo:** Processo criado, recursos reservados, mas não admitido na fila de execução.
2. **Pronto:** Aguardando na fila para usar o processador.
3. **Execução:** Instruções sendo processadas.
4. **Bloqueado:** Aguardando evento externo (E/S ou sincronização).
5. **Encerrado/Saída:** Execução finalizada, recursos em fase de liberação.

--------------------------------------------------------------------------------

## 3. Criação de Processos: fork()

### O que é

Chamada de sistema que cria um novo processo clonando o processo atual.

### Para que serve

Permitir que um programa execute múltiplas tarefas de forma independente com isolamento total de memória.

### Funcionamento

O processo **filho** é uma cópia exata do **pai**, herdando código, dados, pilha e heap. No entanto, o filho recebe um novo PID (Process ID) e as alterações na memória de um não afetam o outro.

### Sintaxe

```c
#include <sys/types.h>
#include <unistd.h>

pid_t fork(void);
```

### Parâmetros e Retorno

- **Parâmetros:** Nenhum.
- **Retorno:**
    - **0:** Retornado no espaço do processo filho.
    - **PID do filho (>0):** Retornado no espaço do processo pai.
    - **-1:** Erro na criação.

### Exemplo Completo: fork() e wait()

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

/* Exemplo de criação de processo e sincronização */
int main() {
    pid_t pid = fork();

    if (pid < 0) {
        /* Erro ao criar processo */
        perror("Falha no fork");
        exit(1);
    } else if (pid == 0) {
        /* Código executado apenas pelo FILHO */
        printf("Sou o processo filho (PID: %d)\n", getpid());
        exit(0);
    } else {
        /* Código executado apenas pelo PAI */
        printf("Sou o processo pai (PID: %d). Esperando o filho...\n", getpid());
        wait(NULL); /* Sincronização: espera o filho terminar */
        printf("O processo filho terminou.\n");
    }

    return 0;
}
```

**Explicação do Código:**

- A linha `pid = fork()` divide o fluxo.
- O pai recebe o PID do filho e entra no `else`, onde o `wait(NULL)` bloqueia sua execução até o filho encerrar.
- O filho recebe 0, entra no `else if`, imprime sua mensagem e encerra com `exit(0)`.

**Saída Esperada:**

```text
Sou o processo pai (PID: 1234). Esperando o filho...
Sou o processo filho (PID: 1235)
O processo filho terminou.
```

--------------------------------------------------------------------------------

## 4. Substituição de Imagem: A Família exec()

### O que é

Um conjunto de funções que substitui o código e os dados do processo atual por um novo programa executável.

### Quando usar

Quando se deseja que o processo filho execute um programa diferente do programa original do pai.

### Funcionamento Interno

O SO descarta o espaço de endereçamento atual (código, pilha, heap) e carrega o novo executável no mesmo PID. Se bem-sucedido, o programa original deixa de existir.

### Variantes Comuns

- `execl(path, arg0, ..., NULL)`: Lista de argumentos manual.
- `execv(path, argv[])`: Vetor de argumentos.
- `execvp(file, argv[])`: Usa a variável `PATH` para localizar o executável.

--------------------------------------------------------------------------------

## 5. Threads: Fluxos Leves de Execução (pthreads)

### O que é

As _threads_ (ou processos leves - LWP) são fluxos de execução independentes que operam dentro do **mesmo processo**.

### Objetivo

Permitir a computação paralela com baixo custo de criação e comunicação facilitada.

### Funcionamento

- **Compartilhamento:** Todas as threads de um processo compartilham o **Espaço de Endereçamento** (variáveis globais, heap), arquivos abertos e sinais.
- **Contexto Privado:** Cada thread possui seu próprio **ID (TID)**, **PC (Contador de Programa)**, **Registradores** e **Stack (Pilha)**.
- **Risco:** Como a memória é compartilhada, o acesso simultâneo a variáveis globais sem sincronização causa condições de corrida.

### Sintaxe: pthread_create

```c
int pthread_create(
    pthread_t *thread,           /* Identificador da thread */
    const pthread_attr_t *attr,  /* Atributos (NULL para padrão) */
    void *(*start_routine)(void *), /* Função que a thread executará */
    void *arg                    /* Argumento único para a função */
);
```

### Sintaxe: pthread_join

```c
int pthread_join(pthread_t thread, void **retval);
```

- **Para que serve:** Faz a thread chamadora (geralmente a principal) esperar o término da thread especificada. Equivale ao `wait()` de processos.

### Exemplo Completo em C (pthreads)

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Estrutura para passar múltiplos argumentos, se necessário */
typedef struct {
    int id;
    char* msg;
} t_arg;

/* Função executada pelas threads */
void* tarefa(void* arg) {
    t_arg* dados = (t_arg*) arg;
    printf("Thread %d executando: %s\n", dados->id, dados->msg);
    pthread_exit(NULL); /* Finaliza a thread explicitamente */
}

int main() {
    pthread_t thread1, thread2;
    t_arg a1 = {1, "Processando dados A"};
    t_arg a2 = {2, "Processando dados B"};

    /* Criação das threads */
    if (pthread_create(&thread1, NULL, tarefa, (void*) &a1) != 0) {
        printf("Erro ao criar thread 1\n");
        return 1;
    }

    if (pthread_create(&thread2, NULL, tarefa, (void*) &a2) != 0) {
        printf("Erro ao criar thread 2\n");
        return 1;
    }

    /* Sincronização: Aguarda as threads terminarem */
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Todas as threads finalizaram.\n");

    return 0;
}
```

**Explicação do Código Linha por Linha:**

1. `pthread_t thread1`: Declara a variável que armazenará o identificador da thread.
2. `void* tarefa(void* arg)`: Define a rotina da thread. Ela deve retornar `void*` e aceitar `void*` como parâmetro.
3. `pthread_create(...)`: O SO cria um novo fluxo de execução que começa na função `tarefa`. O controle retorna imediatamente ao `main`.
4. `pthread_join(thread1, NULL)`: O `main` bloqueia sua execução até que `thread1` termine. Isso é vital, pois se o `main` encerrar, o processo inteiro (e todas as threads) morre.
5. `pthread_exit(NULL)`: Termina a thread atual de forma limpa.

**Saída Esperada (A ordem das threads pode variar):**

```text
Thread 1 executando: Processando dados A
Thread 2 executando: Processando dados B
Todas as threads finalizaram.
```

--------------------------------------------------------------------------------

## 6. Problemas Comuns em Programação Concorrente

- **Condição de Corrida (Race Condition):** Ocorre quando o resultado final depende da ordem imprevisível de execução de múltiplas threads acessando o mesmo dado.
- **Deadlock (Impasse):** Duas ou mais threads ficam bloqueadas para sempre, cada uma esperando por um recurso que a outra possui.
- **Starvation (Inanição):** Uma thread nunca consegue acesso aos recursos necessários (ou CPU) porque outras threads de maior prioridade monopolizam o sistema.
- **Processo Zumbi:** Ocorre quando um processo filho termina, mas o pai não chama `wait()`. O filho permanece na tabela de processos como "morto", consumindo recursos.
- **Inconsistência de Memória:** Threads leem valores desatualizados em caches locais antes que a memória principal seja atualizada.

--------------------------------------------------------------------------------

## 7. Escalonamento: Decidindo a Execução

O escalonador decide qual processo/thread em estado **Pronto** ganhará a CPU.

### Algoritmos Comuns

1. **FCFS (First-Come, First-Served):** Simples, mas causa filas longas se a primeira tarefa for pesada.
2. **Round Robin (RR):** Cada tarefa recebe um _quantum_ de tempo. Garante justiça e interatividade.
3. **Shortest Process Next (SPN):** Favorece tarefas curtas, minimizando o tempo médio de espera, mas pode causar inanição de tarefas longas.
4. **Prioridade:** Executa quem tem maior valor de prioridade. Risco de inanição (mitigado pela técnica de _Aging_ ou envelhecimento).

### Tipos de Escalonamento

- **Preemptivo:** O SO pode interromper um processo à força (ex: fim do _quantum_ ou chegada de prioridade maior).
- **Não-Preemptivo:** O processo executa até terminar ou se bloquear voluntariamente por E/S.

--------------------------------------------------------------------------------

## 8. Boas Práticas e Erros Frequentes

### Boas Práticas

1. **Sempre usar** `**wait**` **ou** `**waitpid**`**:** Evita a criação de zumbis e libera recursos do sistema.
2. **Sempre usar** `**pthread_join**`**:** Garante que a thread principal não termine antes das secundárias.
3. **Minimizar variáveis globais:** Prefira passar dados via argumentos na criação da thread para evitar condições de corrida.
4. **Verificar retornos:** Funções como `pthread_create` e `fork` podem falhar (ex: falta de recursos). Sempre trate o erro.

### Erros Frequentes em Provas/Exercícios

- **Esquecer que o** `**fork()**` **retorna duas vezes:** Tentar ler o PID no processo filho (onde é 0) para identificar o próprio filho.
- **Encerrar o** `**main**` **prematuramente:** Usar `return 0` no `main` sem `pthread_join`, matando as threads que ainda estavam trabalhando.
- **Confundir Processo com Thread:** Dizer que threads têm memória isolada ou que processos compartilham memória global nativamente.
- **Zombie accumulation:** Criar centenas de processos filhos em um loop sem chamar `wait()`, esgotando a tabela de PIDs do sistema.