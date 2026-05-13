# Guia Técnico: Fundamentos de Arquiteturas Paralelas e Avaliação de Desempenho

Este documento sintetiza os fundamentos de arquitetura e os modelos teóricos necessários para a exploração do paralelismo, com foco em sistemas de memória compartilhada e execução em nível de threads, servindo como base para a implementação de sistemas concorrentes.

--------------------------------------------------------------------------------

## 1. Paralelismo em Nível de Thread (Granulosidade Média)

### Definição

O paralelismo em nível de thread consiste na execução simultânea de múltiplos fluxos de execução (threads) dentro de uma mesma aplicação. É classificado como de granulosidade média e é a base para a programação multithreaded.

### Objetivo

Resolver a necessidade de intervenção do programador ou do compilador para aproveitar o potencial de hardware de arquiteturas multithreading, multicore e aceleradores.

### Funcionamento

- **Comportamento na Memória:** As threads operam em arquiteturas onde a comunicação ocorre através de variáveis na memória compartilhada (ou memória cache em multicores).
- **Sincronização:** Exige coordenação explícita do programador para evitar acessos conflitantes aos dados.
- **Execução:** Várias threads são disparadas para executar simultaneamente em diferentes núcleos ou unidades funcionais.
- **Compartilhamento de Recursos:** Threads dentro do mesmo encapsulamento compartilham níveis de memória cache e a memória principal global.
- **Riscos:** A falta de coordenação resulta em inconsistência de dados.

### Comparação com Execução Sequencial

Diferente da execução sequencial (SISD), o nível de thread exige que o código seja decomposto. Enquanto o paralelismo de instrução é transparente (hardware/compilador), o nível de thread impõe um overhead de gerenciamento e sincronização que deve ser compensado pelo ganho de velocidade.

--------------------------------------------------------------------------------

## 2. Arquiteturas MIMD de Memória Compartilhada

### Definição

As arquiteturas MIMD (_Multiple Instruction Streams, Multiple Data Streams_) de Memória Compartilhada são sistemas onde todos os processadores têm acesso a um espaço de endereçamento global único.

### Objetivo

Eliminar a necessidade de movimentação física de dados entre processadores e facilitar a adaptação de técnicas de programação sequencial.

### Funcionamento Interno

- **Comunicação:** Realizada via variáveis na memória compartilhada, resultando em alta eficiência.
- **Organização (UMA):** Nas arquiteturas _Uniform Memory Access_, todos os processadores possuem o mesmo tempo de acesso à memória global, geralmente via barramento compartilhado.
- **Organização (NUMA):** Nas arquiteturas _Non-Uniform Memory Access_, a memória é blocada e conectada localmente a nós. O acesso à memória local é mais rápido que o acesso remoto.
- **Coerência de Cache:** Utiliza algoritmos como _snooping_ (em hardware) para manter a consistência dos dados nas caches locais.

### Possíveis Problemas

- **Contenção de Memória:** Limita a escalabilidade; após certo número de processadores, a adição de novos núcleos não aumenta o desempenho.
- **Contenção de Barramento:** O barramento único possui limite físico e de largura de banda.
- **Necessidade de Sincronização:** Exige o uso de primitivas especiais (como operações atômicas) ao acessar regiões compartilhadas.

### Boas Práticas

- Utilizar hierarquias de cache (L1, L2, L3) para mitigar a contenção no barramento.
- Em sistemas NUMA, priorizar a localidade de dados para minimizar o acesso remoto.
- Combinar modelos (ex: OpenMP/threads internamente e MPI para comunicação entre nós).

--------------------------------------------------------------------------------

## 3. Aceleradores e Kernels (GPUs)

### Definição

Aceleradores são dispositivos de hardware especializados (GPUs, Manycores, FPGAs) que executam trechos de código intensivos chamados _kernels_.

### Objetivo

Alcançar alto grau de paralelismo com foco no _throughput_ (vazão) e eficiência energética, delegando tarefas pesadas a núcleos especializados.

### Funcionamento

- **Memória:** Possuem memória própria separada do hospedeiro (CPU).
- **Transferência:** Dados e códigos devem ser transferidos da memória do hospedeiro para o acelerador via barramentos (frequentemente o gargalo do sistema).
- **Arquitetura Hopper (NVIDIA):** Organizada em Multiprocessadores de Fluxo (SM). O escalonador dispara threads em grupos de 32 (chamados de _warps_).
- **Arquitetura Instinct (AMD):** Baseada em _chiplets_ e memória HBM3 de alta largura de banda.

### Fluxo de Execução

1. Identificação de trechos paralelizáveis (_kernels_).
2. Transferência de dados para a memória do acelerador.
3. Disparo concorrente de milhares de threads.
4. Retorno dos resultados para a memória do hospedeiro.

### Erros Frequentes em Exercícios

- Ignorar o tempo de movimentação de dados entre CPU e acelerador no cálculo de ganho de desempenho.
- Confundir a hierarquia de cache local do SM com a memória global da GPU.

--------------------------------------------------------------------------------

## 4. Métricas: Speedup (Aceleração)

### Definição

O _Speedup_ mede a razão entre o tempo de execução em um único processador e o tempo em P processadores.

### Objetivo

Quantificar o ganho real de desempenho obtido com a paralelização.

### Sintaxe (Fórmula)

S(P) = \frac{T(1)}{T(P)}

### Parâmetros

- **T(1)****:** Tempo gasto na execução com um único processador.
- **T(P)****:** Tempo gasto na execução com P processadores.

### Comportamento do Speedup

|   |   |
|---|---|
|Tipo|Descrição|
|**Linear**|S(P) = P (Ideal).|
|**Sublinear**|S(P) < P (Comum, devido a overhead de comunicação/sincronização).|
|**Superlinear**|S(P) > P (Ocorre por melhor uso de cache ou redução de buscas em bases de dados).|
|**Retorno Negativo**|Adição de processadores aumenta o tempo total.|

--------------------------------------------------------------------------------

## 5. Lei de Amdahl

### Definição

Modelo teórico que estabelece o limite máximo de aceleração de um programa baseado em sua fração sequencial (não paralelizável).

### Objetivo

Prever a viabilidade de paralelizar uma aplicação.

### Sintaxe (Fórmula)

S(P) = \frac{P}{1 + (P - 1)f_s} _Onde_ _f_s_ _é a fração sequencial._

### Exemplo de Cálculo (Baseado no Contexto)

- **Cenário:** Programa leva 20 horas no total. A parte sequencial fixa leva 1 hora.
- **Fração Sequencial (****f_s****):** 1 / 20 = 0,05 (5%).
- **Cálculo com Processadores Infinitos:** S(P) \approx 1 / f_s = 1 / 0,05 = 20.
- **Conclusão:** Mesmo com infinitos processadores, o ganho máximo é de 20 vezes.

--------------------------------------------------------------------------------

## 6. Lei de Gustafson

### Definição

Também conhecida como lei de Gustafson–Barsis, propõe que problemas maiores podem ser paralelizados eficientemente se o tamanho do problema crescer com o número de processadores.

### Objetivo

Oferecer um contraponto otimista à Lei de Amdahl para problemas de escala variável.

### Funcionamento

Assume que o tempo de execução paralelo é mantido constante enquanto o volume de dados aumenta. Isso resulta em uma diminuição proporcional da fração sequencial.

### Sintaxe (Fórmula)

S(P) = P - f_s \cdot (P - 1)

### Comparação de Resultados (Exemplo do Material)

Para uma parte sequencial de 5% (f_s = 0,05):

- Com **2** processadores: Speedup = 1,95.
- Com **20** processadores: Speedup = 19,05.
- Com **200** processadores: Speedup = 190,05.

--------------------------------------------------------------------------------

## 7. Problemas Comuns e Boas Práticas

### Problemas Comuns

1. **Condição de Corrida:** Acesso simultâneo a variáveis compartilhadas sem travas.
2. **Deadlock:** Espera circular por mensagens ou recursos (comum em troca de mensagens e sincronização mal feita).
3. **Contenção de Memória:** Muitos núcleos tentando acessar o mesmo barramento/módulo simultaneamente.
4. **Granularidade Inadequada:** Muitas mensagens pequenas gerando mais overhead que computação.

### Boas Práticas

- **Balanceamento de Carga:** Garantir que todos os processadores tenham volume de trabalho homogêneo.
- **Redução de Mensagens:** Preferir enviar poucas mensagens grandes do que muitas pequenas (reduz latência de rede).
- **Sobreposição:** Sempre que possível, sobrepor computação com comunicação ou cópia de dados (em aceleradores).
- **Uso de Memória Unificada:** Em aceleradores, simplifica a programação, embora o gerenciamento subjacente deva ser monitorado.