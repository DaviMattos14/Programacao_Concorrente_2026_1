# Guia de Estudo: Programação Paralela e Concorrente – Arquiteturas, Processos e Threads
# 1. Visão Geral

O objetivo central deste conteúdo é compreender como o hardware e o software se organizam para executar múltiplas tarefas simultaneamente, visando ganho de desempenho e eficiência energética. A programação paralela é aplicada hoje desde supercomputadores para previsões climáticas até dispositivos móveis e aceleradores de Inteligência Artificial (IA).

### Diferenciação Essencial

- **Concorrência:** Gerenciamento de múltiplos fluxos de execução que podem ou não ocorrer ao mesmo tempo (comum em sistemas multitarefa).
- **Paralelismo:** Execução simultânea de tarefas em múltiplos recursos de processamento (núcleos ou processadores).
- **Distribuição:** Execução em múltiplos nós computacionais independentes que se comunicam via rede (clusters).

### Níveis de Execução

1. **Instrução (Granulosidade Fina):** Execução simultânea de instruções via hardware (pipeline, superescalares). Transparente ao programador.
2. **Thread (Granulosidade Média):** Múltiplos fluxos dentro de um processo. Requer intervenção do programador ou compilador.
3. **Processo (Granulosidade Grossa):** Unidade computacional independente em multiprocessadores ou multicomputadores. Requer bibliotecas como MPI ou suporte do SO.

--------------------------------------------------------------------------------

# 2. Conceitos Teóricos Fundamentais

### Classificação de Flynn

Proposta em 1966, organiza os computadores baseando-se no fluxo de instruções e de dados:

| Categoria | Descrição                              | Exemplo                                                 |
| --------- | -------------------------------------- | ------------------------------------------------------- |
| **SISD**  | Uma instrução, um dado.                | Processadores convencionais.                            |
| **SIMD**  | Uma instrução, múltiplos dados.        | Processadores vetoriais, GPUs, arquiteturas sistólicas. |
| **MIMD**  | Múltiplas instruções, múltiplos dados. | Multiprocessadores e Clusters.                          |

### Organização de Memória (MIMD)

- **Memória Compartilhada:** Todos os processadores acessam um espaço de endereçamento global.
    - **UMA (Uniform Memory Access):** Tempo de acesso igual para todos os núcleos.
    - **NUMA (Non-Uniform Memory Access):** Memória dividida em blocos locais; acesso à memória local é mais rápido que à remota.
- **Memória Distribuída:** Cada nó possui sua própria memória local. A comunicação ocorre exclusivamente por **troca de mensagens**.

### O Ciclo de Vida do Processo

Um processo é uma entidade dinâmica que possui:

- **Espaço de Endereçamento:** Código (.text), dados estáticos (.data), dados sem valor inicial (.bss), heap e pilha (stack).
- **PCB (Bloco de Controle de Processo):** Estrutura de dados do SO que armazena o PID, estado, registradores e privilégios.

--------------------------------------------------------------------------------

# 3. Implementação e Controle de Processos

Nos sistemas baseados em POSIX, a criação de novos fluxos de execução de processos baseia-se no binômio `fork()` e `exec()`.

### Mecanismos de Criação

- `**fork()**`**:** Cria uma cópia exata do processo pai. O filho herda variáveis e descritores de arquivos, mas possui seu próprio espaço de endereçamento.
- `**exec()**`**:** Substitui o código e o espaço de endereço do processo atual por um novo programa. Não cria um novo PID, apenas "reveste" o processo existente com outro código.

#### Exemplo Funcional em C (`fork` e `execl`)

O código abaixo demonstra a criação de um processo filho que troca sua imagem de execução.

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> /* Necessário para a função wait() */

int main () {
    /* fork() retorna 0 para o processo filho e o PID do filho para o pai */
    if (fork() == 0) {  
        /* Trecho executado apenas pelo FILHO */
        printf("Alô do processo filho %d!\n", getpid());
        
        /* execl substitui o programa atual pelo executável "./teste" */
        execl("./teste", "teste", (char *)0); 
        
        /* Se execl funcionar, esta linha nunca será alcançada */
    } else {
        /* Trecho executado apenas pelo PAI */
        printf("Alô do processo pai %d!\n", getpid());
        
        /* O pai aguarda o término do filho para evitar processos zumbis */
        wait(NULL); 
    }
    return 0;
}
```

**Análise de Execução:**

1. O `fork()` duplica o processo. Agora existem dois fluxos independentes.
2. Na memória, o filho recebe uma cópia do estado do pai.
3. O filho executa `execl`, limpando seu espaço de endereçamento original e carregando o programa "teste".
4. O pai entra em `wait()`, suspendendo sua execução até que o filho envie seu status de término.

--------------------------------------------------------------------------------

# 4. Escalonamento: O Coração da Concorrência

O escalonador decide qual processo ou thread utiliza o processador.

### Objetivos do Escalonamento

- **Throughput (Vazão):** Maximizar tarefas por unidade de tempo.
- **Turnaround:** Minimizar o tempo total (espera + execução).
- **Justiça:** Garantir que todos os processos recebam tempo de CPU.

### Algoritmos Comuns

1. **First-Come First-Served (FCFS):** Ordem de chegada (simples, mas pode gerar filas longas).
2. **Round Robin (RR):** Cada processo usa um _quantum_ (fatia de tempo) e volta para o fim da fila. Essencial para sistemas interativos.
3. **Shortest Process Next (SPN):** Prioriza processos curtos (minimiza espera média, mas pode causar _starvation_ de processos longos).
4. **Feedback:** Múltiplas filas com diferentes prioridades. Processos que usam muita CPU perdem prioridade; processos de E/S ganham.

--------------------------------------------------------------------------------

# 5. Threads: Fluxos de Execução Leves

Threads são fluxos que operam **dentro do mesmo processo**, compartilhando o espaço de memória.

### Por que usar Threads em vez de Processos?

- **Troca de contexto mais rápida:** Menos informações para salvar/restaurar.
- **Comunicação facilitada:** Podem ler e escrever nas mesmas variáveis globais (sem necessidade de mecanismos complexos do SO).

### Modelos de Gerenciamento

- **Threads de Usuário:** Gerenciadas por bibliotecas. O SO não as "vê". Se uma bloqueia por E/S, o processo inteiro para.
- **Threads de Kernel:** O SO conhece cada thread. Se uma bloqueia, o escalonador pode rodar outra do mesmo processo.
- **Modelos de Multithreading:**
    - **1:1:** Uma thread de usuário para uma de kernel (aproveita múltiplos núcleos).
    - **N:1:** Várias de usuário em uma de kernel (rápida, mas sem paralelismo real).
    - **M:N:** Híbrido, combinando flexibilidade e desempenho.

--------------------------------------------------------------------------------

# 6. Dicas para Exercícios e Provas

1. **Identificando Regiões Críticas:** Sempre que duas ou mais threads/processos acessarem a mesma variável para escrita, há uma região crítica.
2. **Detectando Condição de Corrida:** Se o resultado final depende da ordem de execução das threads, o código está incorreto.
3. **Processos Zumbi:** Ocorrem quando o pai não chama `wait()`. O filho terminou, mas sua entrada permanece na tabela de processos.
4. **Starvation vs. Deadlock:** _Starvation_ (Inanição) é quando um processo nunca consegue rodar por baixa prioridade. _Deadlock_ é um bloqueio mútuo onde ninguém avança.

--------------------------------------------------------------------------------

# 7. Resumo Final (Checklist de Estudos)

### Mapa Mental Textual

- **Arquitetura:** Flynn (SISD, SIMD, MIMD) -> Memória (UMA, NUMA, Distribuída).
- **Processos:** Programa estático vs. Processo dinâmico -> PCB -> Estados (Novo, Pronto, Execução, Bloqueado, Saída).
- **Threads:** Contexto leve -> Compartilhamento de memória -> Modelos (Usuário vs. Kernel).
- **Escalonamento:** Preemptivo vs. Não-preemptivo -> Algoritmos (RR, FCFS, SPN, Feedback).

### Quiz de Autoavaliação

1. Qual a principal diferença entre arquiteturas UMA e NUMA?
2. O que acontece com o espaço de endereçamento de um processo após a chamada `exec()`?
3. Por que o uso de threads de usuário pode ser problemático em operações de Entrada e Saída (E/S)?
4. Explique o conceito de "envelhecimento" (_aging_) no escalonamento por prioridade.

### Glossário Técnico

- **Context Switch (Troca de Contexto):** Salvar o estado do processo atual no PCB e carregar o estado do próximo.
- **Quantum:** Fatia de tempo máxima que um processo pode ocupar a CPU no Round Robin.
- **Warp:** Grupo de 32 threads disparadas simultaneamente em GPUs NVIDIA.
- **Affinity (Afinidade):** Tendência de escalonar uma thread no mesmo núcleo para aproveitar o cache.
--------------------------------------------------------------------------------

## Glossário de Termos

- **Acelerador:** Hardware especializado (GPU, FPGA) para tarefas paralelas intensivas.
- **Barramento:** Canal físico de comunicação entre componentes (ex: CPU e RAM).
- **Coerência de Cache:** Protocolo que garante que dados em caches de diferentes núcleos sejam iguais.
- **Deadlock:** Situação de espera circular onde processos ficam travados aguardando uns aos outros.
- **ELF (Executable and Linking Format):** Formato padrão de arquivos binários em sistemas Unix/Linux.
- **Escalonador:** Componente do SO que decide a ordem de execução de tarefas.
- **FPGA:** Circuito integrado que pode ser reprogramado para funções específicas.
- **HIP (Heterogeneous Interface for Portability):** API da AMD para portar código CUDA.
- **Kernel:** Núcleo central do sistema operacional que gerencia hardware e processos.
- **Multicore:** Processador com múltiplos núcleos de processamento no mesmo chip.
- **NUMA:** Arquitetura de memória onde o tempo de acesso depende da localização física do dado.
- **PCB (Process Control Block):** Estrutura de dados que descreve um processo para o SO.
- **Pipeline:** Técnica de sobrepor fases de execução de instruções para aumentar o desempenho.
- **Preempção:** Capacidade do SO de interromper uma tarefa em favor de outra.
- **Quantum:** Intervalo de tempo fixo concedido a um processo no escalonamento Round Robin.
- **ROCm:** Pilha de software aberta da AMD para computação em GPU.
- **SM (Streaming Multiprocessor):** Unidade de processamento básica em arquiteturas de GPU.
- **Throughput (Vazão):** Quantidade de trabalho ou processos concluídos em um período.
- **UMA:** Arquitetura onde todos os processadores acessam a memória com a mesma latência.
- **Warp:** Grupo básico de threads (geralmente 32) que executam juntas em uma GPU NVIDIA.