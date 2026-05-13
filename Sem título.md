# Guia Técnico de Programação Paralela: Processos, Threads e Arquiteturas

Esta documentação fornece uma visão técnica e prática sobre os fundamentos da computação paralela e concorrente, com foco no padrão POSIX (pthreads) e na organização de sistemas operacionais e arquiteturas modernas.

--------------------------------------------------------------------------------

# Processos

## Definição

O processo é a unidade fundamental de execução em um sistema operacional. Diferente de um programa, que é uma entidade estática (código no disco), o processo é uma entidade dinâmica que representa o programa em execução, gerenciado pelo sistema operacional (SO).

## Objetivo

Permitir a execução de tarefas de forma independente e isolada, garantindo que múltiplos programas possam competir pelo uso do processador e recursos do sistema sem interferência direta uns nos outros.

## Funcionamento

- **Espaço de Endereçamento:** Cada processo possui uma região de memória virtual dedicada (incluindo seções `.text` para código, `.data` para dados estáticos e `.rodata` para constantes).
- **Memória:** Organizada em **Heap** (alocação dinâmica como `malloc()`) e **Pilha/Stack** (variáveis locais e retornos de função), que crescem em sentidos opostos.
- **Recursos:** Associado a registradores (PC, SP), descritores de arquivos abertos e sinais.
- **Isolamento:** Processos filhos gerados por `fork()` não compartilham o espaço de endereçamento do pai; eles recebem uma cópia exata.
- **Bloco de Controle de Processo (PCB):** Estrutura de dados que armazena o PID, estado (Novo, Pronto, Execução, Bloqueado, Saída), prioridade e contexto dos registradores para trocas de contexto.

## Sintaxe

As chamadas de sistema fundamentais em sistemas POSIX para gerenciamento de processos em C são:

```c
pid_t fork(void);
int execl(const char *path, const char *arg, ...);
pid_t wait(int *stat_loc);
```

## Parâmetros

- `fork()`: Não recebe parâmetros.
- `execl()`:
    - `path`: Caminho do arquivo executável que substituirá o processo atual.
    - `arg`: Argumentos passados para o novo programa (o primeiro deve ser o nome do programa).
- `wait()`: Ponteiro para um inteiro onde o status de término do processo filho será armazenado.

## Valor de Retorno

- `fork()`: Retorna `0` para o processo filho e o **PID** do filho para o processo pai. Retorna `-1` em caso de erro.
- `execl()`: Só retorna se houver erro (retorna `-1`). Em caso de sucesso, o código do processo é substituído.

## Exemplo Completo

O exemplo abaixo demonstra a criação de um processo filho e a substituição do seu código usando `execl`.

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    pid = fork(); /* Cria um novo processo */

    if (pid < 0) {
        /* Erro na criação */
        fprintf(stderr, "Falha no fork\n");
        return 1;
    } 
    else if (pid == 0) {
        /* Código executado apenas pelo processo FILHO */
        printf("Alô do processo filho %d!\n", getpid());
        /* Substitui o programa atual pelo comando /bin/ls */
        execl("/bin/ls", "ls", "-l", NULL);
    } 
    else {
        /* Código executado apenas pelo processo PAI */
        printf("Alô do processo pai %d!\n", getpid());
        wait(NULL); /* Aguarda o término do filho para evitar zumbis */
        printf("Filho finalizado.\n");
    }

    return 0;
}
```

## Explicação do Código

1. `fork()`: Divide a execução. O pai recebe o PID do filho; o filho recebe 0.
2. `if (pid == 0)`: Bloco de execução do filho.
3. `execl()`: Transforma o processo filho em uma execução do comando `ls`. O código original após esta linha no filho é descartado.
4. `wait(NULL)`: O pai suspende sua execução até que o filho termine, permitindo que o SO limpe os recursos do filho.

## Saída Esperada

```text
Alô do processo pai [PID_PAI]!
Alô do processo filho [PID_FILHO]!
[Listagem de arquivos do diretório...]
Filho finalizado.
```

## Possíveis Problemas

- **Processo Zumbi (Defunto):** Ocorre quando um processo filho termina, mas o pai não chama `wait()`. A entrada do filho permanece na tabela de processos para que o pai possa ler o status de saída.
- **Inconsistência:** Processos não compartilham memória automaticamente; a comunicação exige mecanismos complexos como sinais ou troca de mensagens.

## Boas Práticas

- Sempre tratar o valor de retorno do `fork()`.
- Garantir que o processo pai aguarde (`wait`) ou trate sinais `SIGCHLD` para evitar acúmulo de zumbis.
- Usar `exec` após `fork` se o objetivo for rodar um programa diferente.

## Comparação com Execução Sequencial

- **Vantagem:** Execução concorrente de tarefas independentes (ex: um servidor tratando várias requisições).
- **Overhead:** A criação de processos é "pesada" (granulosidade grossa), exigindo cópia de tabelas de memória e descritores.

## Resumo Rápido

|   |   |
|---|---|
|Recurso|Descrição|
|**fork()**|Cria cópia idêntica do processo.|
|**exec()**|Substitui imagem do processo por novo programa.|
|**wait()**|Sincroniza término do filho com o pai.|
|**Zumbi**|Filho terminado sem `wait` do pai.|

--------------------------------------------------------------------------------

# Threads (POSIX pthreads)

## Definição

Threads são fluxos de execução independentes que operam dentro do contexto de um mesmo processo. São conhecidas como "processos leves" (LWP).

## Objetivo

Explorar o paralelismo com menor custo de criação e troca de contexto do que processos, permitindo que múltiplas tarefas compartilhem os mesmos dados na memória de forma eficiente.

## Funcionamento

- **Compartilhamento:** Todas as threads de um processo compartilham o mesmo espaço de endereçamento (variáveis globais, heap), arquivos abertos e sinais.
- **Exclusividade:** Cada thread possui seu próprio ID (TID), conjunto de registradores, Apontador de Instrução (PC) e Pilha de Execução (Stack).
- **Hierarquia:** Diferente de processos, threads não são hierárquicas. Formam um grupo de mesmo nível.
- **Modelos:**
    - **User-level:** Gerenciadas por bibliotecas (rápidas, mas o SO não as vê).
    - **Kernel-level:** Gerenciadas pelo SO (permite paralelismo real em múltiplos núcleos).
    - **Híbridas (M:N):** Mapeia várias threads de usuário em várias de kernel.

## Sintaxe

Assinaturas principais da biblioteca `pthread.h`:

```c
int pthread_create(
    pthread_t *thread,
    const pthread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg
);

int pthread_join(pthread_t thread, void **retval);
void pthread_exit(void *retval);
```

## Parâmetros

- `thread`: Ponteiro para a variável que armazenará o identificador da thread.
- `attr`: Atributos da thread (usualmente `NULL` para padrão).
- `start_routine`: Função que a thread executará. Deve retornar `void*` e receber `void*`.
- `arg`: Argumento único passado para a função de execução.
- `retval`: Ponteiro para capturar o valor de retorno da thread ao finalizar.

## Valor de Retorno

As funções pthreads retornam `0` em caso de sucesso ou um código de erro diferente de zero.

## Exemplo Completo

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* Função que a thread executará */
void* tarefa(void* arg) {
    int id = *(int*)arg;
    printf("Thread %d em execução compartilhando memória...\n", id);
    pthread_exit(NULL);
}

int main() {
    pthread_t thread_id;
    int valor = 10;

    /* Criação da thread */
    if (pthread_create(&thread_id, NULL, tarefa, (void*)&valor) != 0) {
        printf("Erro ao criar thread\n");
        return 1;
    }

    /* Espera a thread terminar */
    pthread_join(thread_id, NULL);
    printf("Thread principal finalizada.\n");

    return 0;
}
```

## Explicação do Código

1. `pthread_t thread_id`: Declara o identificador da thread.
2. `pthread_create`: Dispara a função `tarefa`. O argumento `&valor` é passado via cast para `void*`.
3. `tarefa`: Executa concorrentemente. Note que ela acessa o endereço de `valor` que pertence ao processo.
4. `pthread_join`: Bloqueia a execução da `main` até que a thread criada termine.

## Saída Esperada

```text
Thread 10 em execução compartilhando memória...
Thread principal finalizada.
```

## Possíveis Problemas

- **Condição de Corrida (Race Condition):** Quando duas threads acessam e modificam a mesma variável simultaneamente sem sincronização.
- **Deadlock:** Quando duas threads ficam bloqueadas esperando recursos uma da outra eternamente.
- **Inconsistência de Memória:** Devido ao compartilhamento direto, uma thread pode ler um dado parcialmente escrito por outra.

## Boas Práticas

- Sempre utilizar mecanismos de sincronização (Mutex) ao acessar dados compartilhados.
- Usar `pthread_join` para garantir que a thread principal não termine antes das threads escravas (o término da principal encerra o processo).
- Evitar variáveis globais desnecessárias para reduzir riscos de race conditions.

## Resumo Rápido

|   |   |
|---|---|
|Função|Objetivo|
|**pthread_create**|Inicia novo fluxo de execução.|
|**pthread_join**|Sincroniza e aguarda término de uma thread.|
|**pthread_exit**|Finaliza a thread atual explicitamente.|
|**Pilha**|Única parte da memória que não é compartilhada.|

--------------------------------------------------------------------------------

# Conceitos Avançados e Sincronização

## Sincronização e Região Crítica

- **Região Crítica:** Trecho de código que acessa recursos compartilhados.
- **Mutex (Exclusão Mútua):** Primitiva que garante que apenas uma thread entre na região crítica por vez.
- **Semáforos:** Usados para sincronização e controle de acesso a recursos limitados.
- **Variáveis de Condição:** Permitem que uma thread espere até que uma condição específica seja verdadeira.

## Escalonamento de Curto Prazo

O SO decide qual thread/processo usará o processador usando algoritmos:

1. **First-Come First-Served (FCFS):** Ordem de chegada.
2. **Round Robin (RR):** Cada tarefa recebe um _quantum_ (fatia de tempo).
3. **Shortest Process Next (SPN):** Favorece tarefas curtas.
4. **Feedback:** Múltiplas filas com prioridades dinâmicas baseadas no histórico de uso.

## Arquiteturas Paralelas (Classificação de Flynn)

- **SISD:** Processadores convencionais (uma instrução, um dado).
- **SIMD:** Processadores vetoriais e GPUs (uma instrução opera sobre múltiplos dados).
- **MIMD:** Multiprocessadores e Clusters (múltiplas instruções e múltiplos dados).
    - **Memória Compartilhada (UMA):** Todos núcleos acessam a memória no mesmo tempo.
    - **Memória Compartilhada Distribuída (NUMA):** Tempos de acesso variam conforme a localização física do bloco de memória.

## Aceleradores (GPUs)

As GPUs exploram o paralelismo massivo em nível de thread.

- **Kernels:** Trechos de código intensivo enviados ao acelerador.
- **Warps:** Grupos de threads (geralmente 32) disparadas simultaneamente em uma arquitetura como a NVIDIA Hopper.
- **Hierarquia de Memória:** Registradores (mais rápidos) -> L1/Shared Memory -> L2 -> Memória Global (DRAM/HBM).

--------------------------------------------------------------------------------

## Erros Frequentes em Provas/Exercícios

1. **Confundir Fork com Thread:** Achar que variáveis alteradas no filho do `fork()` mudam no pai (falso, memórias são separadas).
2. **Esquecer o Join:** Deixar a thread principal encerrar, matando as threads secundárias antes de terminarem.
3. **Acesso Direto:** Tentar acessar variáveis globais em threads sem usar Mutex, causando resultados imprevisíveis.
4. **Zumbis:** Esquecer que processos filhos precisam ser "coletados" pelo pai via `wait()`.