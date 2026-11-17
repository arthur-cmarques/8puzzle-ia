# 8-puzzle com busca em largura e aprofundamento iterativo

Trabalho em grupo de Inteligência Artificial (PUC-Campinas, 2025) em C. É um jogo do 8-puzzle no terminal e, na segunda versão, dois algoritmos de busca que resolvem o puzzle sozinhos:

- **Busca em largura (BFS)**, com fila.
- **Aprofundamento iterativo (IDS)**, que roda uma busca em profundidade com limite 1, 2, ..., N, com pilha.

É um projeto de estudo, pequeno e sem pretensão além disso.

## Conteúdo

| Arquivo | O que é |
|---|---|
| `src/8puzzle.c` | Versão base: só o jogo (você move as peças). |
| `src/8puzzle_com_IA.c` | Jogo + BFS + IDS + modo de medição `--bench`. |
| `scripts/medir.sh` | Compila e roda a medição, gravando em `results/`. |
| `scripts/*.py` | Opcionais: conferem se os caminhos são ótimos e comparam os algoritmos. |
| `results/` | Saídas da medição (CSV e resumos). |

## Como compilar e rodar

Precisa de `gcc`.

```sh
gcc -Wall -Wextra -o 8puzzle_com_IA src/8puzzle_com_IA.c
./8puzzle_com_IA
```

Menu: `1` jogar (digite o número da peça que quer mover para o espaço vazio, `-1` volta ao menu), `2` busca em largura, `3` aprofundamento iterativo (pede o limite máximo N). O jogo base compila do mesmo jeito (`src/8puzzle.c`).

No Windows, o código usa `cls` e define a saída do console como UTF-8 (`SetConsoleOutputCP`). **Esse ramo não foi testado**: tudo foi compilado e executado apenas em Linux (gcc 12).

## Créditos e o que é original

- As estruturas de **pilha e fila** embutidas nos arquivos (as funções `CriaPilha`, `push`, `pop`, `CriaFila`, `InsereFila`, `RetiraFila` e auxiliares) são baseadas em `PILHA.h` e `FILA.h` da **professora Lúcia Guimarães**, da disciplina.
- O jogo parte de um **"8-puzzle original" fornecido na disciplina**. As buscas e a adaptação das estruturas para o problema foram feitas pelo grupo.
- Este repositório não afirma que tudo foi escrito do zero pelos autores.

**Autores:** Arthur Costa Marques, Fabio Barbosa Gonsalez, Felipe Salinas Caodaglio e Patrick Rovaron Franco.

**Minha contribuição:** [PREENCHER]

## O que foi alterado depois da entrega

O código entregue está no primeiro commit de `src/` (`git log`), sem alterações. Depois dele, em commits separados:

1. Fontes convertidos de Windows-1252 para UTF-8 (os acentos saíam quebrados fora do Windows).
2. `system("cls")` virou uma função `limpa_tela()` portável; `setlocale(LC_ALL, "Portuguese")` (que só funciona no Windows) foi removido.
3. O bloco de geração de filhos, repetido 4 vezes na BFS, virou a função `gera_filho_largura`.
4. `ins_fim` passou a inicializar `profundidade`; avisos do compilador corrigidos.
5. **Vazamento de memória corrigido**: os nós, filas e pilhas agora são liberados ao fim de cada busca.
6. Leitura de números validada: antes, digitar uma letra (ou fim de entrada) deixava o programa em laço infinito.
7. A opção 3 do menu se chamava "Busca em Profundidade Limitada", mas o `main` roda os limites 1..N, o que é **aprofundamento iterativo**. O menu foi renomeado. A função `busca_profundidade_limitada` mantém o nome, porque ela, isolada, é de fato limitada.
8. Adicionado o modo `--bench` (abaixo).

Comportamento preservado: com a mesma semente, BFS e IDS produzem a mesma saída que o código anterior (conferido em 40 execuções; só mudam os textos do menu renomeados), e o valgrind e o AddressSanitizer não apontam erros nem vazamentos. Não há suíte de testes automatizados.

**Decisão registrada:** a BFS **continua sem conjunto de estados visitados**; ela só evita voltar ao estado do pai. É a versão original, e a medição abaixo mostra onde isso pesa.

## Medição

```sh
scripts/medir.sh                      # 30 puzzles, sementes 1..30, limite de 5.000.000 nós por busca
scripts/medir.sh 30 1 20000000 cap20M # mesmos puzzles com limite de 20.000.000 nós
```

O `--bench` gera puzzles com um gerador próprio de semente fixa (o `rand()` do Windows e o do Linux dão sequências diferentes, então a semente não bastaria), embaralhando 40 movimentos como o jogo. Para cada puzzle registra estados visitados, tempo, pico de nós em memória e tamanho do caminho. Dados completos em `results/bench.csv` e `results/bench_cap20M.csv`.

Resolvidos pelos **dois** algoritmos (comparação pareada, `results/comparacao_pareada.txt`):

| | Limite 5 M nós (24 de 30 puzzles) | Limite 20 M nós (26 de 30 puzzles) |
|---|---|---|
| Estados visitados, mediana, BFS | 430 | 554 |
| Estados visitados, mediana, IDS (soma das iterações) | 17.031 | 30.228 |
| Estados visitados, máximo, BFS | 8.299 | 30.431 |
| Estados visitados, máximo, IDS | 4.781.643 | 26.059.193 |
| Tempo, mediana, BFS / IDS | 0,02 ms / 0,42 ms | 0,03 ms / 0,74 ms |
| Tempo, máximo, BFS / IDS | 0,8 ms / 266 ms | 1,7 ms / 1.441 ms |
| Pico de memória dos nós, máximo, BFS / IDS | 0,9 MiB / 135 MiB | 3,1 MiB / 836 MiB |
| Tamanho do caminho | igual nos dois, em todos | igual nos dois, em todos |

Resolvidos no total: BFS 29 de 30 (limite 5 M) e 30 de 30 (limite 20 M); IDS 24 de 30 e 26 de 30.

O que dá para dizer:

- Nos puzzles que os dois resolvem, **os caminhos têm o mesmo tamanho e são ótimos**. Isso foi conferido contra uma BFS independente, com conjunto de visitados (`scripts/conferir_otimo.py`): zero divergências nos 30 puzzles.
- Com este embaralhamento, o IDS visita bem mais estados que a BFS (mediana de cerca de 40 a 48 vezes mais) e usa bem mais memória. Parte disso é esperada: o IDS refaz as iterações anteriores e não tem nenhuma poda de repetidos. Os "estados visitados" dos dois também não são medidos exatamente do mesmo jeito (o IDS soma todas as iterações).
- A BFS sem visitados já dá sinal de esforço quando o puzzle é fundo: o puzzle 20 (ótimo de 26 movimentos) estourou 5 M de nós e só terminou com 20 M (3,5 milhões de estados visitados, pico de 377 MiB).
- Os puzzles que o IDS não resolveu pararam por causa do limite de nós por busca que defini, não por erro do algoritmo. O IDS não libera nós durante uma iteração, então a memória cresce com o limite de profundidade.

### Limitações

- **Amostra pequena:** 30 puzzles, sementes 1 a 30, todos de embaralhamento de 40 movimentos (ótimos de 4 a 26 movimentos; a maioria abaixo de 16). Não dá para generalizar para o 8-puzzle como um todo.
- **O gerador de puzzles é do mesmo autor do código** (é o embaralhamento do próprio jogo, com outro gerador de números aleatórios). Não é um conjunto de puzzles independente.
- **Tempo medido em uma única máquina e uma única rodada** (contêiner Linux, gcc 12.5 com `-O2`, Intel Core Ultra 9 285H). Os tempos de BFS são menores que 1 ms, perto da resolução do relógio, e variam entre rodadas; use os estados visitados como medida mais estável.
- **O limite de nós** (5 M ou 20 M por busca) muda quem "resolve"; os números de resolvidos dependem dele.
- O pico de memória é contado em nós (64 bytes cada), sem o custo do alocador.

## Licença

Código dos autores sob licença MIT (`LICENSE`). A licença cobre o que o grupo escreveu; as estruturas de pilha e fila e o 8-puzzle de partida vêm de material da disciplina, citado acima, e não são relicenciados por este repositório.
