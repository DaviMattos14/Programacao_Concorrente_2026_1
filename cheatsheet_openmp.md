# Cheat Sheet — OpenMP (Programação Concorrente)

> Referência de consulta rápida. Organizado por: Regiões Paralelas → Worksharing → Escopo de Dados → Schedule → Sincronização → Tasks → Funções de Biblioteca / Variáveis de Ambiente.

---

## 1. Modelo Fork-Join

O programa começa com **1 thread (master)**. Ao encontrar `#pragma omp parallel`, o runtime cria um **time de threads** (fork) que executam o mesmo bloco de código. Ao final do bloco, há uma **barreira implícita**: todas esperam, e voltam a ser 1 thread só (join).

```
1 thread ──[fork: parallel]──► N threads executam o bloco ──[join]──► 1 thread
```

---

## 2. Diretivas de Região Paralela

| Diretiva | O que faz | Barreira implícita? |
|---|---|---|
| `#pragma omp parallel` | Cria o time de threads; **todo** o bloco é replicado e executado por **cada** thread | Sim, no fim do bloco |
| `#pragma omp parallel for` | Forma combinada: cria o time **e** distribui as iterações do laço seguinte entre as threads | Sim, no fim do laço |
| `#pragma omp parallel sections` | Forma combinada de `parallel` + `sections` (distribui blocos de código distintos entre threads) | Sim |

**Cláusulas comuns de `parallel`:**

| Cláusula | Efeito |
|---|---|
| `num_threads(N)` | Define quantas threads o time terá (sobrepõe `OMP_NUM_THREADS` para essa região) |
| `if(expr)` | Se `expr` for falsa, a região roda sequencialmente (só a thread master) |
| `default(shared\|none)` | Define o escopo padrão de variáveis não listadas explicitamente. `none` obriga você a escopar tudo manualmente (boa prática!) |
| `shared(list)` / `private(list)` / `firstprivate(list)` | Ver seção 4 |

---

## 3. Diretivas de Worksharing (divisão de trabalho)

Diretivas que **dividem** trabalho entre as threads de um time já criado (usadas dentro de `#pragma omp parallel`).

| Diretiva | O que faz | Barreira implícita? |
|---|---|---|
| `#pragma omp for` | Divide as iterações de um laço `for` entre as threads do time (cada iteração roda **uma vez só**, por **uma** thread) | Sim (a menos que use `nowait`) |
| `#pragma omp single` | Apenas **uma** thread do time (a primeira a chegar, não necessariamente a 0) executa o bloco | Sim (a menos que use `nowait`) |
| `#pragma omp master` | Apenas a thread **0** (master) executa o bloco. As demais **não esperam** — seguem direto | **Não** |
| `#pragma omp sections` / `#pragma omp section` | Divide blocos de código **distintos** entre as threads (cada `section` roda em uma thread diferente) | Sim, no fim de `sections` |

**Analogia:** `master` = "só o anfitrião compra o bolo, ninguém espera"; `single` = "quem chegar primeiro compra o bolo, todo mundo espera essa pessoa voltar".

**Cláusulas de `for` / `parallel for`:**

| Cláusula | Efeito |
|---|---|
| `schedule(tipo[, chunk])` | Como as iterações são distribuídas — ver seção 5 |
| `collapse(n)` | Colapsa `n` laços aninhados em um único espaço de iteração antes de dividir entre threads (melhora balanceamento em laços aninhados) |
| `ordered` | Habilita o uso da construção `#pragma omp ordered` dentro do laço |
| `nowait` | Remove a barreira implícita do fim do `for`/`single`/`sections` |
| `reduction(op:var)` | Ver seção 6 |
| `private`, `firstprivate`, `lastprivate` | Ver seção 4 |

---

## 4. Escopo de Dados (Data Scoping)

**Regra padrão:** variáveis declaradas **fora** da região paralela são `shared` por padrão — **exceto** a variável de controle de um `for` paralelizado, que é `private` automaticamente.

| Cláusula | Comportamento |
|---|---|
| `shared(list)` | Todas as threads leem/escrevem na **mesma** posição de memória. Sem proteção = risco de condição de corrida (race condition) |
| `private(list)` | Cada thread recebe uma cópia **própria e não-inicializada**. O valor de fora **não** é herdado, e ao sair da região, o valor original de fora permanece **indefinido/inalterado** |
| `firstprivate(list)` | Como `private`, mas a cópia de cada thread é **inicializada com o valor que a variável tinha antes** da região começar |
| `lastprivate(list)` | Como `private`, mas ao final do laço/seção, o valor da **última iteração/seção logicamente executada** é copiado de volta para a variável original |
| `default(shared\|none)` | Define o escopo padrão das variáveis não listadas. `none` força escopar tudo manualmente (evita erros silenciosos) |

**Por que `shared` sem proteção é perigoso:** uma linha como `resultado = resultado + tid` não é uma operação atômica — envolve **ler**, **somar** e **escrever** de volta na memória, em passos separados. Se duas threads fazem isso ao mesmo tempo na mesma variável, uma pode sobrescrever o resultado da outra → **condição de corrida**, resultado imprevisível.

---

## 5. Cláusula `schedule` (Escalonamento de Laços)

| Tipo | Quando decide a divisão | Comportamento | Melhor para |
|---|---|---|---|
| `static[, chunk]` | **Antes** de rodar (compile/início) | Divide em blocos fixos e previsíveis. Sem `chunk`: blocos de tamanho ≈N/threads. Com `chunk`: distribui em rodízio (round-robin) de `chunk` em `chunk` | Iterações com custo **uniforme** — menor overhead |
| `dynamic[, chunk]` | **Durante** a execução | Cada thread pega um pedaço (chunk, padrão=1); ao terminar, pede o próximo pedaço disponível na fila | Iterações com custo **desigual/imprevisível** — mas mais overhead de coordenação |
| `guided[, chunk]` | Híbrido | Começa com pedaços **grandes**, diminuindo progressivamente até o mínimo `chunk` | Meio-termo: carga desigual, mas com menos overhead que `dynamic` puro |
| `auto` | Implementação decide | Deixa o runtime/compilador escolher a melhor estratégia | Quando você não sabe qual usar |
| `runtime` | Variável de ambiente `OMP_SCHEDULE` | Usa o que estiver definido em `OMP_SCHEDULE` em tempo de execução, sem recompilar | Testes/tuning sem recompilar |

⚠️ **Importante:** `schedule` controla **quais** iterações cada thread executa, mas **nunca** garante a ordem de saída (ex: prints no terminal), a menos que se use `ordered`.

---

## 6. Sincronização

| Diretiva | O que faz |
|---|---|
| `#pragma omp critical[(nome)]` | Só **uma thread por vez** pode executar o bloco (região de exclusão mútua). Se usar o mesmo `nome` em vários blocos, eles competem pelo mesmo "cadeado"; nomes diferentes = cadeados independentes |
| `#pragma omp atomic [read\|write\|update\|capture]` | Protege **uma única operação de memória simples** (mais leve que `critical`) <br>• `read`: leitura atômica <br>• `write`: escrita atômica <br>• `update` (padrão): operação tipo `x++`, `x += y` <br>• `capture`: lê e atualiza numa única operação atômica (ex: `z = x++`) |
| `#pragma omp barrier` | Todas as threads do time esperam até que **todas** cheguem nesse ponto antes de continuar |
| `#pragma omp ordered` | Dentro de um `for` com cláusula `ordered`, força que aquele trecho específico execute **na mesma ordem sequencial** do laço original |
| `nowait` (cláusula) | Remove a barreira implícita do fim de `for`, `single` ou `sections` — usar com cuidado, pois pode gerar condições de corrida se as threads seguintes dependerem do resultado |
| `reduction(operador:var)` | Cada thread mantém uma cópia privada da variável, acumula seu resultado parcial, e no final o runtime combina tudo com o `operador` (ex: `+`, `*`, `max`, `min`) de forma segura, sem condição de corrida |

**`critical` vs `atomic`:** use `atomic` quando a operação é simples (uma leitura/escrita/incremento de uma variável escalar) — é mais rápido. Use `critical` quando o bloco protegido tem **múltiplas instruções** ou lógica mais complexa.

---

## 7. Tasks (Tarefas)

Tasks são **unidades independentes de trabalho** (código + ambiente de dados + variáveis de controle), úteis quando o padrão de paralelismo não é um laço regular (ex: recursão, listas encadeadas, grafos de dependência).

### 7.1 Criação de tarefas

| Construção | Quando cria tarefa(s) |
|---|---|
| Início de região `parallel` | Cria tarefas **implícitas** (uma por thread) |
| `#pragma omp task` | Cria **uma** tarefa explícita |
| `#pragma omp taskloop` | Cria tarefas explícitas, uma para cada bloco (chunk) do laço |
| `#pragma omp target` | Cria uma tarefa **target** (para dispositivos, ex: GPU) |

O **runtime decide quando** as tarefas realmente executam — podem ser adiadas (colocadas num "pool de tarefas") ou executadas imediatamente. A thread que executa a tarefa pode ser **diferente** da thread que a criou.

### 7.2 Cláusulas de `#pragma omp task`

| Cláusula | Efeito |
|---|---|
| `if(expr)` | Se falsa, a tarefa é executada de forma **sequencial e imediata** (undeferred) pela thread que a encontrou |
| `final(expr)` | Se verdadeira, a tarefa não terá tarefas-filhas (força execução sequencial dos descendentes) — útil para "cortar" recursão excessivamente fina |
| `mergeable` | Permite que o ambiente de dados da tarefa seja **mesclado** com o da tarefa geradora (otimização) |
| `untied` | A tarefa pode ser retomada por **qualquer** thread do time (não precisa ser a que começou a executá-la). Padrão é `tied` (vinculada à mesma thread) |
| `priority(valor)` | Sugere prioridade de execução (valor numérico não-negativo; maior = recomendado executar antes) |
| `default(shared\|firstprivate\|none)` | Escopo padrão das variáveis dentro da tarefa |
| `private(list)` / `firstprivate(list)` / `shared(list)` | Mesmo conceito da seção 4, aplicado ao escopo da tarefa |
| `depend(in: list)` | Esta tarefa **depende** de tarefas anteriores que escreveram (`out`/`inout`) nessas variáveis |
| `depend(out: list)` | Esta tarefa **produz** dados nessas variáveis; tarefas futuras com `in`/`inout` sobre elas dependerão desta |
| `depend(inout: list)` | Combina leitura e escrita — depende de qualquer tarefa anterior (`in`, `out` ou `inout`) sobre a variável, e futuras tarefas dependerão desta |

### 7.3 Regras de escopo padrão em tasks

| Situação | Escopo padrão |
|---|---|
| Variável estática ou global | `shared` |
| Variável local (automática) declarada **fora** da task | `firstprivate` (se a task for "órfã", sem task-pai) |
| Variável local declarada **dentro** do bloco `task` | `private` |
| Variável `static` declarada dentro do bloco `task` | `shared` |
| Objeto alocado dinamicamente (`malloc`, `new`) apontado de fora | `shared` (o ponteiro pode ser firstprivate, mas o conteúdo apontado é compartilhado) |
| Task com task-pai | Herda os atributos de compartilhamento da tarefa pai |

### 7.4 Sincronização de tarefas

| Diretiva | O que faz |
|---|---|
| `#pragma omp taskwait` | A tarefa atual espera suas **tarefas-filhas diretas** terminarem (não espera netos/descendentes mais profundos) |
| `#pragma omp taskgroup` | Espera **todas as tarefas descendentes** (filhas e seus descendentes) dentro do bloco terminarem — sincronização mais profunda que `taskwait`. Pode ter cláusula `task_reduction` |
| `#pragma omp taskyield` | Ponto explícito onde a tarefa atual pode ser suspensa, liberando a thread para executar outra tarefa (evita bloqueios) |
| Barreira implícita (`barrier`, fim de `parallel`) | Garante que **todas** as tarefas criadas por qualquer thread do time terminem |

### 7.5 `taskloop` — paralelizar laços via tasks

```c
#pragma omp taskloop [simd] [cláusulas]
for (...) { ... }
```

| Cláusula | Efeito |
|---|---|
| `grainsize(n)` | Cada bloco (chunk) de tarefa terá entre `n` e `2n` iterações |
| `num_tasks(n)` | Cria exatamente `n` tarefas para cobrir todas as iterações |
| `nogroup` | Por padrão, `taskloop` cria uma região `taskgroup` implícita; essa cláusula remove essa barreira |
| `collapse(n)` | Mesmo conceito da seção 3, aplicado a laços aninhados dentro do taskloop |
| Também aceita: `shared`, `private`, `firstprivate`, `lastprivate`, `default`, `final`, `if`, `untied`, `mergeable`, `depend`, `priority` | Mesmo significado das seções 4 e 7.2 |

`taskloop simd`: além de dividir em tarefas, aplica vetorização SIMD dentro de cada bloco.

---

## 8. Funções de Biblioteca (Runtime API)

| Função | O que retorna/faz |
|---|---|
| `omp_get_num_threads()` | Número de threads **no time atual** (só é significativo dentro de uma região paralela) |
| `omp_get_thread_num()` | O `id` (tid) da thread que chama a função, de 0 a N-1 |
| `omp_get_num_procs()` | Número de processadores/núcleos disponíveis na máquina |
| `omp_in_parallel()` | Retorna se o código está atualmente dentro de uma região paralela ativa |
| `omp_get_wtime()` | Retorna o tempo (em segundos, `double`) desde uma referência fixa — usado para medir tempo de execução |
| `omp_set_num_threads(n)` | Define o número de threads a usar nas próximas regiões paralelas |
| `omp_init_lock(&lock)` | Inicializa uma trava (lock) manual |
| `omp_set_lock(&lock)` | Adquire a trava (bloqueia se outra thread já a possui) |
| `omp_unset_lock(&lock)` | Libera a trava |

---

## 9. Variáveis de Ambiente

| Variável | Efeito |
|---|---|
| `OMP_NUM_THREADS` | Define o número padrão de threads (sobreposta por `num_threads()` no código) |
| `OMP_SCHEDULE` | Define o tipo/chunk de `schedule` usado quando a cláusula do código é `schedule(runtime)` |
| `OMP_STACKSIZE` | Tamanho da pilha de cada thread |
| `OMP_PLACES` | Define o mapeamento de threads para localizações físicas (núcleos, sockets) |
| `OMP_PROC_BIND` | Controla se/como as threads ficam "fixadas" a processadores específicos |

---

## 10. Compilação

```bash
gcc -fopenmp arquivo.c -o programa   # habilita suporte a OpenMP no GCC
./programa                            # executa
OMP_NUM_THREADS=4 ./programa          # define threads via variável de ambiente
```

---

## 11. Resumo mental rápido

- **`parallel`** → cria threads. **`for`** → divide iterações. **`parallel for`** → os dois juntos.
- **`single`** → uma thread qualquer, com barreira. **`master`** → sempre a thread 0, sem barreira.
- **`shared`** → risco de condição de corrida. **`private`** → cópia isolada, não inicializada. **`firstprivate`** → cópia isolada, inicializada com valor de fora.
- **`static`** → previsível, baixo overhead. **`dynamic`** → adaptável, mais overhead. **`guided`** → híbrido.
- **`atomic`** → protege 1 operação simples. **`critical`** → protege um bloco inteiro (mais caro).
- **`task`** → unidade de trabalho independente, útil fora do padrão de laço regular (recursão, listas, grafos de dependência via `depend`).
- **`taskwait`** → espera só filhos diretos. **`taskgroup`** → espera toda a descendência.
