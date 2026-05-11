## Sumário Executivo

Este documento sintetiza os fundamentos das arquiteturas paralelas, o ciclo de vida da transformação de programas e a gestão de processos e threads em sistemas operacionais modernos. O estudo abrange desde os níveis de paralelismo (instrução, thread e processo) até a taxonomia de Flynn, detalhando arquiteturas de memória compartilhada (UMA/NUMA) e distribuída. Um foco considerável é dado aos aceleradores modernos, como as GPUs NVIDIA Hopper e AMD Instinct, explorando suas hierarquias de memória e núcleos de processamento. Adicionalmente, o guia descreve a mecânica interna dos sistemas operacionais: a compilação de código, o formato ELF, a criação de processos via `fork()` e `exec()`, o fenômeno dos processos zumbis e as complexas políticas de escalonamento que garantem a eficiência computacional. Por fim, diferencia-se o funcionamento de threads de usuário e de kernel, fundamentais para a concorrência moderna.

--------------------------------------------------------------------------------

## 1. Fundamentos de Arquiteturas Paralelas

O paralelismo em computação é explorado em diferentes níveis de granulosidade, dependendo da intervenção necessária do hardware, do compilador ou do programador:

- **Nível de Instrução (Granulosidade Fina):** Explorada por processadores _pipelined_, superescalares ou VLIW. É transparente ao usuário e gerida pelo hardware ou compilador.
- **Nível de Thread (Granulosidade Média):** Encontrado em arquiteturas multithreading, multicore e aceleradores. Requer intervenção do programador ou compilador para criar e coordenar múltiplas threads.
- **Nível de Processo (Granulosidade Grossa):** Utilizado em multiprocessadores e multicomputadores, exigindo bibliotecas específicas (como MPI) para coordenação explícita.

### Classificação de Flynn

A taxonomia de Flynn organiza os computadores com base no fluxo de instruções e dados:

1. **SISD (Single Instruction, Single Data):** Processadores convencionais que executam um fluxo por vez.
2. **SIMD (Single Instruction, Multiple Data):** Uma instrução opera sobre diversos dados. Exemplos incluem processadores vetoriais (Cray, instruções AVX) e arquiteturas sistólicas (TPUs do Google).
3. **MIMD (Multiple Instruction, Multiple Data):** Múltiplos fluxos de instruções operam sobre múltiplos dados. Subdivide-se em:
    - **Memória Compartilhada:** Todos os processadores acessam um espaço global. Exemplos: arquiteturas **UMA** (tempo de acesso uniforme) e **NUMA** (tempo de acesso não uniforme, comum em servidores com vários CPUs Xeon ou EPYC).
    - **Memória Distribuída:** Cada nó possui sua memória local. A comunicação ocorre por troca de mensagens (ex: clusters utilizando MPI).

### Quiz 1: Arquiteturas Paralelas

1. **O que caracteriza o paralelismo de nível de instrução?**
2. **Qual a diferença fundamental entre arquiteturas UMA e NUMA?**
3. **Como funciona uma arquitetura sistólica?**
4. **O que define a categoria SIMD na classificação de Flynn?**
5. **Quais são as vantagens e desvantagens de sistemas MIMD com memória distribuída?**
6. **Qual o papel do framework OpenMP em arquiteturas de memória compartilhada?**
7. **O que são instruções vetoriais e onde são encontradas modernamente?**
8. **Como a coerência de cache é mantida em sistemas multiprocessadores?**
9. **Por que a escalabilidade é um desafio em sistemas UMA com barramento único?**
10. **O que caracteriza um cluster de multicomputadores?**

#### Chave de Respostas - Quiz 1

1. É um tipo de paralelismo de granulosidade fina encontrado em processadores com pipeline ou superescalares. Ele é executado de forma transparente ao usuário, sendo explorado diretamente pelo hardware ou pelo compilador.
2. Na UMA, todos os processadores têm o mesmo tempo de acesso à memória global única. Na NUMA, a memória é dividida em blocos locais, onde o acesso à memória local de um nó é muito mais rápido que o acesso à memória remota de outros nós.
3. É uma estrutura onde os dados são "bombeados" através de células de hardware interconectadas que realizam operações simples e repassam aos vizinhos. Esse fluxo contínuo, análogo ao sistema circulatório, é altamente eficiente para cálculos matriciais massivos em IA.
4. Caracteriza-se por uma única instrução operando simultaneamente sobre múltiplos dados. É a base de processadores vetoriais e unidades funcionais que executam operações em massa sobre vetores ou matrizes.
5. As vantagens incluem alta escalabilidade e facilidade para construir sistemas massivamente paralelos. As desvantagens são a complexidade de programação, a necessidade de troca explícita de mensagens (MPI) e o risco de deadlocks.
6. O OpenMP atua como o framework básico para explorar o paralelismo em threads, facilitando a coordenação via variáveis em memória compartilhada. Ele permite extrair paralelismo de laços e gerenciar a sincronização com operações atômicas.
7. São instruções que operam sobre conjuntos lineares de valores, como vetores de N elementos, em uma única iteração. Processadores modernos da Intel (AVX-512) e AMD (AVX2) as utilizam para otimizar cálculos matemáticos.
8. É mantida através de algoritmos implementados em hardware, como a técnica de _snooping_. Esses mecanismos garantem que os dados nas caches locais dos diversos núcleos permaneçam consistentes entre si.
9. Devido à contenção no barramento; conforme se adicionam processadores, o tráfego aumenta até que o barramento se torne um gargalo. Além disso, o comprimento físico limitado do barramento restringe o número de dispositivos que podem ser conectados.
10. Caracterizam-se por diversos nós independentes, onde cada um possui seu próprio processador e memória local. Não existe memória global, exigindo que a comunicação entre os nós ocorra obrigatoriamente por redes de alta velocidade e troca de mensagens.

--------------------------------------------------------------------------------

## 2. Aceleradores e GPUs

Aceleradores são dispositivos de hardware especializados (GPUs, FPGAs, Manycores) que executam trechos intensivos de código (**kernels**) para obter alto desempenho com eficiência energética.

### GPUs Modernas: Comparativo de Arquiteturas

As GPUs utilizam um conjunto massivo de multiprocessadores de fluxo para processamento paralelo.

|   |   |   |
|---|---|---|
|Característica|NVIDIA Hopper (H100)|AMD Instinct (MI300X)|
|**Arquitetura Base**|Hopper|CDNA 3 (baseada em chiplets)|
|**Núcleos de Cálculo**|128 núcleos CUDA FP32 por SM|Oito Accelerator Complex Dies (XCDs)|
|**Escalonamento**|Grupos de 32 threads (**warps**)|Unidades de computação em paralelo|
|**Memória Global**|Até 80 GB (HBM3)|192 GB HBM3|
|**Largura de Banda**|2.000 a 3.000 GB/s|Até 5,3 TB/s|
|**Cache L2**|50 MB (ponto de unificação)|256 MB (Last Level Cache)|
|**Software**|CUDA / OpenACC|ROCm / HIP|

### Desafios de Desempenho em Aceleradores

A movimentação de dados entre o hospedeiro (CPU) e o acelerador é um gargalo crítico, pois ocorre via barramentos (como PCI Express) que são mais lentos que a memória interna do dispositivo. Estratégias como a **Memória Unificada** tentam simplificar a programação, mas o gerenciamento eficiente da transferência de dados continua essencial.

### Quiz 2: Aceleradores e GPUs

1. **O que são "kernels" no contexto de aceleradores?**
2. **Qual a função do escalonador de warp na arquitetura Hopper?**
3. **Como a tecnologia HBM difere das memórias GDDR tradicionais?**
4. **O que é um processo de "offloading"?**
5. **Como as FPGAs podem ser utilizadas para acelerar aplicações específicas?**
6. **Qual a importância da cache L2 em uma GPU como a NVIDIA H100?**
7. **O que são instruções "shuffle" em GPUs?**
8. **Como o framework HIP da AMD auxilia desenvolvedores que usam CUDA?**
9. **Por que a movimentação de dados é considerada um desafio em aceleradores?**
10. **O que caracteriza a arquitetura de chiplets da AMD MI300X?**

#### Chave de Respostas - Quiz 2

1. Kernels são trechos de código computacionalmente intensivos e com alto potencial de paralelismo que são enviados para execução no acelerador. Eles operam sobre dados transferidos para a memória do dispositivo.
2. O escalonador dispara threads em grupos de 32, chamados de warps. Na arquitetura Hopper, cada SM possui quatro desses escalonadores, permitindo a execução concorrente de múltiplos grupos.
3. A HBM (High Bandwidth Memory) é empilhada verticalmente em 3D e conectada diretamente ao processador. Isso permite uma largura de banda e densidade muito superiores às memórias GDDR convencionais.
4. É o ato de transferir a execução de partes específicas de um programa do processador principal (hospedeiro) para um acelerador externo. Frameworks como OpenMP offloading automatizam essa gestão de código e dados.
5. FPGAs são dispositivos programáveis por software que podem ser configurados para implementar arquiteturas sistólicas ou circuitos lógicos personalizados. Elas são ideais para acelerar criptografia, processamento de sinais e simulações científicas.
6. A cache L2 de 50MB atua como o ponto primário de unificação de dados entre os diversos SMs da GPU. Ela serve operações de carga e armazenamento, provendo compartilhamento de dados eficiente e de alta velocidade.
7. São instruções otimizadas para a troca de dados entre threads dentro de um mesmo warp. Elas melhoram significativamente o desempenho de algoritmos como a Transformada de Fourier (FFT).
8. O HIP é uma API que facilita a portabilidade, permitindo que códigos escritos originalmente para a plataforma CUDA da NVIDIA sejam convertidos e executados em GPUs AMD. Isso oferece flexibilidade e transparência aos desenvolvedores.
9. Porque o barramento que conecta a CPU ao acelerador (como o PCIe) possui largura de banda muito inferior à das memórias internas dos dispositivos. Isso torna a transferência de dados um gargalo que pode anular o ganho de velocidade do processamento paralelo.
10. A GPU não é uma pastilha monolítica, mas sim composta por vários dies (XCDs e IODs) interconectados pela tecnologia Infinity Fabric. Essa modularidade permite maior eficiência no gerenciamento de memória e escalonamento de núcleos.

--------------------------------------------------------------------------------

## 3. Ciclo de Vida do Software: Da Compilação ao Processo

A transformação de um código de alto nível em algo executável pelo hardware segue um fluxo rigoroso:

1. **Compilador:** Traduz a linguagem de alto nível para **linguagem de montagem** (_assembly_), específica da arquitetura (ex: x86).
2. **Montador (**_**Assembler**_**):** Converte o _assembly_ em código de máquina, gerando o **programa objeto**.
3. **Ligador (**_**Linker**_**):** Une o programa objeto a bibliotecas (estáticas ou dinâmicas) para criar o **programa executável**.

### O Formato ELF (_Executable and Linking Format_)

Padrão em sistemas Linux, o arquivo ELF contém cabeçalhos e seções essenciais:

- `.text`: Instruções do programa (apenas leitura).
- `.rodata`: Dados somente de leitura (constantes).
- `.data`: Dados estáticos com valores iniciais (leitura/escrita).
- `.bss`: Dados estáticos sem valor inicial (zerados na carga).
- `.symtab`: Tabela de símbolos para depuração.

### Layout de Memória do Processo

Ao ser carregado, o processo organiza sua memória virtual:

- **Pilha (**_**Stack**_**):** Armazena variáveis locais e parâmetros de funções; cresce do final para o início da memória.
- **Heap:** Espaço para alocação dinâmica (via `malloc()`); cresce do início para o fim.

### Quiz 3: Compilação e Formato ELF

1. **Qual a diferença entre bibliotecas estáticas e dinâmicas?**
2. **O que é o "programa objeto"?**
3. **Quais informações são encontradas no cabeçalho ELF?**
4. **Para que serve a seção** `**.bss**` **em um executável?**
5. **Como a pilha (**_**stack**_**) e a** _**heap**_ **se comportam na memória?**
6. **Qual o papel do "carregador" (**_**loader**_**)?**
7. **O que acontece se o carregador não encontrar uma referência de biblioteca dinâmica?**
8. **O que diferencia a linguagem de montagem da linguagem de máquina?**
9. **Por que o compilador realiza a tradução em diversas etapas?**
10. **O que é o endereço de entrada de um programa?**

#### Chave de Respostas - Quiz 3

1. Bibliotecas estáticas são integradas permanentemente ao executável pelo ligador, gerando arquivos maiores. Bibliotecas dinâmicas são carregadas na memória apenas uma vez e compartilhadas por vários programas, economizando espaço.
2. É o arquivo em linguagem de máquina gerado pelo montador a partir do código _assembly_. Ele contém o código binário, mas ainda carece da ligação com as rotinas das bibliotecas para ser executável.
3. Contém informações gerais como o tipo do arquivo (executável ou objeto), a arquitetura do processador, o endereço de entrada e os deslocamentos para as tabelas de cabeçalho de programa e seções.
4. Ela armazena dados estáticos que não possuem valor inicial definido. Durante a carga do programa, o sistema operacional reserva o espaço necessário e geralmente preenche esses endereços com zeros.
5. Ambas crescem dinamicamente durante a execução, mas em sentidos opostos. A _heap_ (alocação dinâmica) cresce do início para o fim, enquanto a pilha (variáveis locais e retornos) cresce do final da memória para o início.
6. O carregador é responsável por colocar o programa executável na memória principal. Ele verifica referências a bibliotecas dinâmicas, realiza as ligações necessárias e prepara o ambiente para o processador iniciar a execução.
7. Se uma referência externa não puder ser resolvida no momento da carga, o carregador emite uma mensagem de erro e cancela a execução do programa antes que ele inicie.
8. A linguagem de montagem usa mnemônicos legíveis por humanos para representar as instruções. A linguagem de máquina é o conjunto puramente binário de instruções que o processador entende e executa diretamente.
9. Para garantir eficiência na tradução e facilitar a adaptação das ferramentas a diferentes arquiteturas de processadores. Isso permite que partes do compilador sejam reaproveitadas mudando apenas o gerador de código final.
10. É o endereço de memória onde reside a primeira instrução que o processador deve executar ao iniciar o programa. Essa informação é especificada no cabeçalho do arquivo ELF.

--------------------------------------------------------------------------------

## 4. Gerenciamento de Processos e Escalonamento

Um **processo** é uma entidade dinâmica composta pelo código executável, dados, recursos do sistema e seu estado de execução.

### Bloco de Controle de Processo (PCB)

O PCB é o "descritor" do processo no kernel, contendo:

- PID (Identificador Único).
- Estado (Novo, Pronto, Execução, Bloqueado, Saída).
- Contexto (Registradores, Contador de Programa, Ponteiro de Pilha).
- Privilégios e informações contábeis.

### Criação: `fork()` vs `exec()`

- `**fork()**`**:** Cria um novo processo que é uma cópia exata do pai. O filho recebe um novo PID.
- `**exec()**`**:** Substitui o conteúdo (código e dados) do processo atual por um novo programa. Não cria um novo processo, mas altera sua identidade.
- **Processo Zumbi:** Um processo que terminou a execução, mas permanece na tabela de processos porque o pai ainda não coletou seu _status_ de término via `wait()`.

### Escalonamento de Curto Prazo (Algoritmos)

O escalonador decide qual processo na fila de prontos utilizará o CPU:

- **FCFS (First-Come, First-Served):** Ordem de chegada; simples, mas pode causar esperas longas.
- **Round Robin:** Cada processo recebe um _quantum_ de tempo; excelente para sistemas interativos.
- **Shortest Process Next (SPN):** Favorece processos curtos para minimizar o tempo médio de espera.
- **Feedback:** Utiliza múltiplas filas com prioridades que se ajustam conforme o comportamento do processo (CPU-bound vs. I/O-bound).

### Quiz 4: Processos e Escalonamento

1. **O que é uma "troca de contexto"?**
2. **Quais são os cinco estados básicos de um processo?**
3. **O que acontece no estado "Pronto Suspenso"?**
4. **Qual a diferença entre escalonamento preemptivo e não-preemptivo?**
5. **O que é o "quantum" em um algoritmo Round Robin?**
6. **Quais eventos podem causar o término forçado de um processo?**
7. **O que caracteriza um processo "zumbi"?**
8. **Como o algoritmo "Highest Response Ratio Next" (HRRN) evita a inanição?**
9. **Qual a função do comando** `**wait()**` **para um processo pai?**
10. **Por que o sistema operacional precisa manter uma tabela de processos?**

#### Chave de Respostas - Quiz 4

1. É o procedimento onde o processador salva o estado do processo atual no seu PCB e carrega o estado de um novo processo a partir do PCB deste. Isso permite alternar a execução entre múltiplos processos.
2. Novo (criado), Pronto (aguardando CPU), Execução (em processamento), Bloqueado (aguardando E/S ou evento) e Encerrado (finalizado).
3. Ocorre quando a memória principal está cheia e um processo no estado "pronto" é movido para a memória secundária (_disk_). Ele aguarda que haja espaço na memória principal para voltar ao estado "pronto" e competir pelo CPU.
4. No não-preemptivo, o processo detém o CPU até terminar ou bloquear. No preemptivo, o sistema operacional pode interromper o processo (por fim de tempo ou chegada de prioridade maior) para dar lugar a outro.
5. É a fatia de tempo pré-determinada que cada processo pode utilizar o processador continuamente. Se o tempo expirar antes do processo terminar, ele é recolocado no fim da fila de prontos.
6. Erros de execução (divisão por zero, violação de memória), interrupção manual pelo usuário (comando `kill`), consumo excessivo de recursos ou o término do processo pai.
7. É um processo que já finalizou sua execução, mas cujas informações (PID e status) permanecem na tabela do sistema porque o pai ainda não as coletou. Se acumulados, podem esgotar os recursos de identificação do sistema.
8. Ele calcula uma razão que considera tanto o tempo de execução esperado quanto o tempo de espera na fila. Como a prioridade aumenta com o tempo de espera, processos longos eventualmente conseguem ser executados.
9. Serve para que o processo pai aguarde a finalização de seus filhos e colete o código de status de término. Essa ação permite que o kernel remova as entradas dos filhos da tabela de processos, evitando zumbis.
10. Para gerenciar e armazenar os PCBs de todos os processos ativos. Essa estrutura permite ao kernel monitorar o estado, os recursos alocados, as prioridades e as informações necessárias para o escalonamento e troca de contexto.

--------------------------------------------------------------------------------

## 5. Threads e Concorrência

Threads são fluxos de execução independentes dentro de um mesmo processo. Elas compartilham o espaço de endereçamento (memória), arquivos abertos e variáveis globais, o que torna a comunicação muito mais rápida do que entre processos.

### Threads de Usuário vs. Kernel

- **Usuário:** Gerenciadas por bibliotecas. São rápidas (sem chamadas ao kernel), mas se uma thread bloquear em E/S, todo o processo bloqueia.
- **Kernel:** O sistema operacional conhece cada thread. Se uma bloquear, outras do mesmo processo continuam executando. São mais lentas para criar e gerenciar devido às trocas de modo do processador.

### Modelos de Multithreading

- **N:1:** Várias threads de usuário mapeadas em uma de kernel. Troca rápida, mas sem ganho em multinúcleos.
- **1:1:** Cada thread de usuário tem uma correspondente no kernel. Aproveita todos os núcleos, mas a gestão é mais pesada.
- **M:N:** Mapeia M threads de usuário em N threads de kernel. Combina rapidez de troca com aproveitamento de hardware.

### Quiz 5: Threads

1. **O que as threads compartilham dentro de um processo?**
2. **Quais são os elementos exclusivos de cada thread?**
3. **Por que a troca de contexto entre threads é mais rápida que entre processos?**
4. **Qual a principal limitação das threads de usuário puras?**
5. **O que é o fenômeno da "inanição" (**_**starvation**_**) no escalonamento de threads?**
6. **Como a técnica de "envelhecimento" (**_**aging**_**) resolve a inanição?**
7. **O que acontece com o processo se a thread principal termina?**
8. **Quais as vantagens das threads de kernel para aplicações de E/S?**
9. **Descreva o modelo de multithreading M:N.**
10. **O que é um TCB (**_**Thread Control Block**_**)?**

#### Chave de Respostas - Quiz 5

1. Compartilham o espaço de endereçamento de memória, variáveis globais, os arquivos abertos e outros recursos globais que compõem o contexto do processo.
2. Cada thread possui seu próprio identificador único (TID), estado da pilha de execução, apontador de instrução (PC) e valores dos registradores de uso geral.
3. Porque elas compartilham a mesma memória; não é necessário atualizar tabelas de páginas ou limpar caches de memória virtual, apenas salvar e restaurar registradores e ponteiros de pilha.
4. Se uma thread de usuário realizar uma operação bloqueante (como leitura de disco), o kernel, que só vê o processo como um todo, bloqueia o processo inteiro, impedindo as outras threads de rodar.
5. Ocorre quando threads de baixa prioridade nunca conseguem tempo de processador porque novas threads com prioridades mais altas entram continuamente no sistema, mantendo a fila de prontos sempre ocupada.
6. Ela aumenta gradualmente o valor da prioridade de uma thread conforme ela permanece aguardando na fila. Com o tempo, sua prioridade torna-se alta o suficiente para que ela seja selecionada pelo escalonador.
7. Dependendo da implementação da biblioteca de threads, o processo inteiro e todas as suas threads associadas podem ser encerrados automaticamente quando a thread principal finaliza.
8. Como o kernel conhece cada thread individualmente, ele pode colocar apenas a thread que solicitou a E/S em estado bloqueado, permitindo que o escalonador execute outras threads do mesmo processo.
9. É um esquema híbrido onde um número arbitrário de threads de usuário é mapeado em um número arbitrário de threads de kernel. Isso une trocas de contexto rápidas com a capacidade de usar múltiplos núcleos físicos.
10. É o bloco de controle de thread, uma estrutura de dados no kernel (para threads de kernel) que armazena informações específicas da thread, como seu estado, prioridade e contexto de registradores.

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