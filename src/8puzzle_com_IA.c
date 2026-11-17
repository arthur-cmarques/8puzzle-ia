#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* ---- Estatísticas das buscas (usadas apenas pelo modo --bench) ---- */
static int  modo_silencioso = 0;     /* 1 = as buscas não imprimem nada */
static long max_nos = 0;             /* 0 = sem limite; senão, máximo de nós por chamada de busca */
static long stat_nos = 0;            /* nós de busca alocados (acumulado) */
static long stat_nos_pico = 0;       /* maior nº de nós vivos numa única chamada de busca */
static long stat_estados = 0;        /* estados visitados (acumulado entre chamadas) */
static long stat_estados_ultima = 0; /* estados visitados na última chamada */
static int  stat_caminho = -1;       /* movimentos da solução encontrada */
static int  stat_abortou = 0;        /* 1 = a busca parou por atingir max_nos */

/* Gerador de números aleatórios. Por padrão usa rand(); no modo --bench usa um
 * gerador próprio com semente fixa, para que os puzzles sejam os mesmos em qualquer sistema. */
static int aleatorio_fixo = 0;
static unsigned long long aleatorio_estado = 0;

static int aleatorio(void)
{
    if (aleatorio_fixo == 0)
    {
        return rand();
    }
    aleatorio_estado = aleatorio_estado * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)((aleatorio_estado >> 33) & 0x7fffffff);
}

/* Devolve 1 (e marca stat_abortou) se a chamada de busca atual já gerou nós demais. */
static int excedeu_limite_nos(long nos_no_inicio)
{
    if (max_nos > 0 && stat_nos - nos_no_inicio > max_nos)
    {
        stat_abortou = 1;
        return 1;
    }
    return 0;
}

/* Limpa a tela de forma portável (cls no Windows, clear nos demais). */
static void limpa_tela(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* Lê um inteiro. Devolve 1 se leu, 0 se a entrada não era um número
 * (a linha é descartada) e -1 se a entrada acabou. */
static int le_inteiro(int *valor)
{
    int lido = scanf("%d", valor);
    if (lido == 0)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
        {
        }
    }
    return lido == EOF ? -1 : lido;
}

typedef struct no
{
    int info[3][3];
    struct no *prox;
    struct no *pai;
    int profundidade;
}No;

typedef struct pilha
{
    No *TOPO;
} Pilha;

Pilha* CriaPilha()
{
     Pilha *p;
     p=(Pilha*)malloc(sizeof(Pilha));
     p->TOPO = NULL;
   return p;
}

int vaziaPilha(Pilha *p)
{
    if (p->TOPO==NULL)
    {
        return 1;
    }
    return 0;
}

void copia_matriz(int original[3][3], int copia[3][3])
{
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            copia[i][j] = original[i][j];
        }
    }
}

No* ins_ini (No* t, int estado_matriz[3][3], No* pai_do_no)
{
    No* aux = (No*) malloc(sizeof(No));
    stat_nos++;
    copia_matriz(estado_matriz, aux->info);
    aux->prox = t;
    aux->pai = pai_do_no;
    if(pai_do_no == NULL)
    {
        aux->profundidade = 0;
    }
    else
    {
        aux->profundidade = pai_do_no->profundidade+1;
    }
    return aux;
}

void push (Pilha* p, int matriz[3][3], No* pai_do_no)
{
    p->TOPO = ins_ini(p->TOPO,matriz, pai_do_no);
}

No* ret_ini (No* aux)
{
    No* p = aux->prox;
    free(aux);
    return p;
}

No* pop (Pilha *p)
{
    if (vaziaPilha(p) == 1)
    {
        printf("Pilha vazia\n");
        return NULL;
    }
    No*removido = p->TOPO;
    p->TOPO = p->TOPO->prox;
    removido->prox = NULL;
    return removido;
}

typedef struct fila
{
    No *inicio;
    No *fim;
} Fila;

Fila* CriaFila()
{
    Fila* f = (Fila*) malloc(sizeof(Fila));
    f->inicio = f->fim = NULL;
    return f;
}

int VaziaFila (Fila* f)
{
    if (f->inicio==NULL)
    {
        return 1;
    }
    return 0;
}

No* ins_fim (No *fim, int estado_matriz[3][3], No* pai_do_no)
{
    No *p = (No*)malloc(sizeof(No));
    stat_nos++;
    copia_matriz(estado_matriz, p->info);
    p->prox = NULL;
    p->pai = pai_do_no;
    if (pai_do_no == NULL)
    {
        p->profundidade = 0;
    }
    else
    {
        p->profundidade = pai_do_no->profundidade+1;
    }
    if (fim != NULL)
    {
        fim->prox = p;
    }
    return p;
}

void InsereFila (Fila* f, int matriz[3][3], No* pai_do_no)
{
    f->fim = ins_fim(f->fim,matriz, pai_do_no);
    if (f->inicio==NULL)
    {
        f->inicio = f->fim;
    }

}

No* retira_ini (No* inicio)
{
    No* p = inicio->prox;
    free(inicio);
    return p;
}

No* RetiraFila (Fila* f)
{
    if (VaziaFila(f))
    {
        printf("Fila vazia\n");
        exit(0);
    }
    No* aux = f->inicio;
    f->inicio = aux->prox;
    if (f->inicio == NULL)
    {
        f->fim = NULL;
    }
    aux->prox = NULL;
    return aux;
}

void imprimir_puzzle(int puzzle[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        printf("------------\n");
        for (int j = 0; j < 3; j++)
        {
            if (puzzle[i][j] == 0)
            {
                printf("   |");
            }
            else
            {
                printf(" %d |", puzzle[i][j]);
            }
        }
        printf("\n");
    }
    printf("------------\n");
}

void imprime_caminho(No* no_final)
{
    if (no_final != NULL)
    {
        Pilha* pilha_aux = CriaPilha();
        No* atual = no_final;
        while (atual != NULL)
        {
        push(pilha_aux, atual->info, NULL);
        atual = atual->pai;
        }
        printf("\t\nPasso a passo da solução:\n");
        No* passos;
        while (vaziaPilha(pilha_aux) == 0)
        {
            passos = pop(pilha_aux);

            imprimir_puzzle(passos->info);
            free(passos);

            if (vaziaPilha(pilha_aux) == 0)
            {
                printf("\n\n");
            }
        }
        free(pilha_aux);
    }
}

/* Libera todos os nós de uma lista encadeada por 'prox'. */
void libera_lista(No* lista)
{
    while (lista != NULL)
    {
        No* proximo = lista->prox;
        free(lista);
        lista = proximo;
    }
}

void acha_posicao_zero(int puzzle_antigo[3][3], int puzzle_novo[3][3], int *linha_do_zero, int *coluna_do_zero)
{
    copia_matriz(puzzle_antigo, puzzle_novo);
    *linha_do_zero = -1, *coluna_do_zero = -1;
    int achou = 0;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (puzzle_novo[i][j] == 0)
            {
                *linha_do_zero = i;
                *coluna_do_zero = j;
                achou = 1;
                break;
            }
        }
        if(achou == 1)
        {
            break;
        }
    }
}

void funcao_suc(int puzzle_antigo[3][3], int puzzle_novo[3][3], int numero_movido)
{
    int linha_do_zero, coluna_do_zero;
    acha_posicao_zero(puzzle_antigo, puzzle_novo, &linha_do_zero, &coluna_do_zero);

    if (linha_do_zero > 0 && puzzle_novo[linha_do_zero - 1][coluna_do_zero] == numero_movido)
    {
        puzzle_novo[linha_do_zero][coluna_do_zero] = numero_movido;
        puzzle_novo[linha_do_zero - 1][coluna_do_zero] = 0;
    }
    else
    {
        if (linha_do_zero < 2 && puzzle_novo[linha_do_zero + 1][coluna_do_zero] == numero_movido)
        {
            puzzle_novo[linha_do_zero][coluna_do_zero] = numero_movido;
            puzzle_novo[linha_do_zero + 1][coluna_do_zero] = 0;
        }
        else
        {
            if (coluna_do_zero > 0 && puzzle_novo[linha_do_zero][coluna_do_zero - 1] == numero_movido)
            {
                puzzle_novo[linha_do_zero][coluna_do_zero] = numero_movido;
                puzzle_novo[linha_do_zero][coluna_do_zero - 1] = 0;
            }
            else
            {
                if (coluna_do_zero < 2 && puzzle_novo[linha_do_zero][coluna_do_zero + 1] == numero_movido)
                {
                    puzzle_novo[linha_do_zero][coluna_do_zero] = numero_movido;
                    puzzle_novo[linha_do_zero][coluna_do_zero + 1] = 0;
                }
            }
        }
    }
}

int completo(int puzzle[3][3])
{
    int i,j;
    int objetivo[3][3] = {{1, 2, 3},
                          {4, 5, 6},
                          {7, 8, 0}};
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (puzzle[i][j] != objetivo[i][j])
            {
                return 0;
            }
        }
    }
    return 1;
}

void gerar_puzzle_inicial(int puzzle[3][3])
{
    int linha_zero = 0, coluna_zero = 0, achou, i, x;
    int resolvido[3][3] = {{1, 2, 3},
                           {4, 5, 6},
                           {7, 8, 0}};
    copia_matriz(resolvido, puzzle);

    for (x = 0; x < 40; x++)
    {
        achou = 0;
        for(i=0; i<3; i++)
        {
            for(int j=0; j<3; j++)
            {
                if(puzzle[i][j] == 0)
                {
                    linha_zero = i;
                    coluna_zero = j;
                    achou = 1;
                    break;
                }
            }
            if(achou == 1)
            {
                break;
            }
        }

        int movs_possiveis[4][2];
        int num_movs = 0;

        if (linha_zero > 0)
        {
            movs_possiveis[num_movs][0] = linha_zero - 1;
            movs_possiveis[num_movs][1] = coluna_zero;
            num_movs++;
        }
        if (linha_zero < 2)
        {
            movs_possiveis[num_movs][0] = linha_zero + 1;
            movs_possiveis[num_movs][1] = coluna_zero;
            num_movs++;
        }
        if (coluna_zero > 0)
        {
            movs_possiveis[num_movs][0] = linha_zero;
            movs_possiveis[num_movs][1] = coluna_zero - 1;
            num_movs++;
        }
        if (coluna_zero < 2)
        {
            movs_possiveis[num_movs][0] = linha_zero;
            movs_possiveis[num_movs][1] = coluna_zero + 1;
            num_movs++;
        }

        int mov_idx = aleatorio() % num_movs;
        int i_troca = movs_possiveis[mov_idx][0];
        int j_troca = movs_possiveis[mov_idx][1];

        puzzle[linha_zero][coluna_zero] = puzzle[i_troca][j_troca];
        puzzle[i_troca][j_troca] = 0;
    }
}

int matrizes_iguais(int a[3][3], int b[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (a[i][j] != b[i][j])
            {
                return 0;
            }
        }
    }
    return 1;
}

/* Gera o filho de 'atual' movendo 'peca' para o espaço vazio e o insere na fila,
 * a menos que seja igual ao estado do pai (única poda da busca em largura). */
void gera_filho_largura(Fila* fila, No* atual, int peca)
{
    int filho_temp[3][3];
    copia_matriz(atual->info, filho_temp);
    funcao_suc(atual->info, filho_temp, peca);

    if (atual->pai == NULL || matrizes_iguais(filho_temp, atual->pai->info) == 0)
    {
        InsereFila(fila, filho_temp, atual);
    }
}

void busca_em_largura(int puzzle_novo[3][3])
{
    long inicio_nos = stat_nos;
    Fila* fila_busca_largura = CriaFila();
    InsereFila(fila_busca_largura, puzzle_novo, NULL);
    int qtd_estados = 0, achou = 0, linha_do_zero, coluna_do_zero, filho_temp[3][3];

    No* atual;
    No* retirados = NULL; /* nós já retirados da fila; ficam vivos porque os filhos apontam para o pai */
    while (VaziaFila(fila_busca_largura) == 0 && achou == 0 && excedeu_limite_nos(inicio_nos) == 0)
    {
        atual = RetiraFila(fila_busca_largura);
        atual->prox = retirados;
        retirados = atual;
        qtd_estados++;

        if (completo(atual->info) == 1)
        {
            if (modo_silencioso == 0)
            {
                printf("%d estados visitados\n", qtd_estados);
                imprime_caminho(atual);
            }
            stat_caminho = atual->profundidade;
            achou = 1;
            break;
        }

        acha_posicao_zero(atual->info, filho_temp, &linha_do_zero, &coluna_do_zero);

        if (linha_do_zero > 0)
        {
            gera_filho_largura(fila_busca_largura, atual, atual->info[linha_do_zero-1][coluna_do_zero]);
        }
        if (linha_do_zero < 2)
        {
            gera_filho_largura(fila_busca_largura, atual, atual->info[linha_do_zero+1][coluna_do_zero]);
        }
        if (coluna_do_zero > 0)
        {
            gera_filho_largura(fila_busca_largura, atual, atual->info[linha_do_zero][coluna_do_zero-1]);
        }
        if (coluna_do_zero < 2)
        {
            gera_filho_largura(fila_busca_largura, atual, atual->info[linha_do_zero][coluna_do_zero+1]);
        }
    }
    if (achou == 0 && modo_silencioso == 0)
    {
        printf("\nNenhuma solução foi encontrada\n");
    }
    stat_estados += qtd_estados;
    stat_estados_ultima = qtd_estados;
    if (stat_nos - inicio_nos > stat_nos_pico)
    {
        stat_nos_pico = stat_nos - inicio_nos;
    }
    libera_lista(retirados);
    libera_lista(fila_busca_largura->inicio);
    free(fila_busca_largura);
}

int busca_profundidade_limitada(int puzzle_inicial[3][3], int limite_maximo)
{
    long inicio_nos = stat_nos;
    Pilha* pilha_busca_profundidade = CriaPilha();
    push(pilha_busca_profundidade, puzzle_inicial, NULL);
    int qtd_estados = 0, linha_do_zero, coluna_do_zero, filho_temp[3][3];

    int resultado = 0;
    No* atual;
    No* retirados = NULL; /* nós já retirados da pilha; ficam vivos porque os filhos apontam para o pai */
    while(vaziaPilha(pilha_busca_profundidade) == 0 && excedeu_limite_nos(inicio_nos) == 0)
    {
        atual = pop(pilha_busca_profundidade);
        atual->prox = retirados;
        retirados = atual;
        qtd_estados++;
        if(completo(atual->info) == 1)
        {
            if (modo_silencioso == 0)
            {
                printf("%d estados visitados\n", qtd_estados);
                imprime_caminho(atual);
            }
            stat_caminho = atual->profundidade;
            resultado = 1;
            break;
        }

        if (atual->profundidade < limite_maximo)
        {
            acha_posicao_zero(atual->info, filho_temp, &linha_do_zero, &coluna_do_zero);

            if (linha_do_zero > 0)
            {
                int peca = atual->info[linha_do_zero-1][coluna_do_zero];
                funcao_suc(atual->info, filho_temp, peca);
                push(pilha_busca_profundidade, filho_temp, atual);
            }
            if (linha_do_zero < 2)
            {
                int peca = atual->info[linha_do_zero+1][coluna_do_zero];
                funcao_suc(atual->info, filho_temp, peca);
                push(pilha_busca_profundidade, filho_temp, atual);
            }
            if (coluna_do_zero > 0)
            {
                int peca = atual->info[linha_do_zero][coluna_do_zero-1];
                funcao_suc(atual->info, filho_temp, peca);
                push(pilha_busca_profundidade, filho_temp, atual);
            }
            if (coluna_do_zero < 2)
            {
                int peca = atual->info[linha_do_zero][coluna_do_zero+1];
                funcao_suc(atual->info, filho_temp, peca);
                push(pilha_busca_profundidade, filho_temp, atual);
            }
        }
    }
    stat_estados += qtd_estados;
    stat_estados_ultima = qtd_estados;
    if (stat_nos - inicio_nos > stat_nos_pico)
    {
        stat_nos_pico = stat_nos - inicio_nos;
    }
    libera_lista(retirados);
    libera_lista(pilha_busca_profundidade->TOPO);
    free(pilha_busca_profundidade);
    return resultado;
}

/* ---------------- Modo de medição (--bench) ---------------- */

typedef struct
{
    int resolveu;
    int caminho;           /* movimentos da solução; -1 se não resolveu */
    int limite;            /* IDS: limite de profundidade em que parou */
    long estados;          /* estados visitados (IDS: soma de todas as iterações) */
    long estados_ultima;   /* estados visitados na última chamada de busca */
    long nos_pico;         /* maior nº de nós vivos numa chamada de busca */
    double ms;
} Medida;

static double agora_ms(void)
{
#if defined(TIME_UTC)
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) != 0)
    {
        return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
    }
#endif
    return (double)clock() * 1000.0 / CLOCKS_PER_SEC;
}

static void zera_estatisticas(void)
{
    stat_nos = stat_nos_pico = stat_estados = stat_estados_ultima = 0;
    stat_caminho = -1;
    stat_abortou = 0;
}

static Medida mede_largura(int puzzle[3][3])
{
    Medida m;
    zera_estatisticas();
    double t0 = agora_ms();
    busca_em_largura(puzzle);
    m.ms = agora_ms() - t0;
    m.resolveu = (stat_caminho >= 0 && stat_abortou == 0);
    m.caminho = m.resolveu ? stat_caminho : -1;
    m.limite = 0;
    m.estados = stat_estados;
    m.estados_ultima = stat_estados_ultima;
    m.nos_pico = stat_nos_pico;
    return m;
}

/* Mesmo laço da opção 3 do menu: limites 1, 2, ..., limite_maximo. */
static Medida mede_iterativo(int puzzle[3][3], int limite_maximo)
{
    Medida m;
    int i, achou = 0;
    zera_estatisticas();
    double t0 = agora_ms();
    for (i = 1; i <= limite_maximo; i++)
    {
        achou = busca_profundidade_limitada(puzzle, i);
        if (achou == 1 || stat_abortou == 1)
        {
            break;
        }
    }
    m.ms = agora_ms() - t0;
    m.resolveu = (achou == 1);
    m.caminho = m.resolveu ? stat_caminho : -1;
    m.limite = i;
    m.estados = stat_estados;
    m.estados_ultima = stat_estados_ultima;
    m.nos_pico = stat_nos_pico;
    return m;
}

static int compara_long(const void *a, const void *b)
{
    long x = *(const long*)a, y = *(const long*)b;
    return (x > y) - (x < y);
}

static int compara_double(const void *a, const void *b)
{
    double x = *(const double*)a, y = *(const double*)b;
    return (x > y) - (x < y);
}

/* Resumo de um algoritmo sobre os puzzles que ele resolveu (impresso em stderr). */
static void resume(const char *nome, Medida *v, int n)
{
    long estados[n], picos[n];
    double tempos[n];
    int k = 0, i;
    double soma_caminho = 0, soma_tempo = 0, soma_estados = 0;
    for (i = 0; i < n; i++)
    {
        if (v[i].resolveu)
        {
            estados[k] = v[i].estados;
            picos[k] = v[i].nos_pico;
            tempos[k] = v[i].ms;
            soma_caminho += v[i].caminho;
            soma_tempo += v[i].ms;
            soma_estados += (double)v[i].estados;
            k++;
        }
    }
    fprintf(stderr, "%s: resolveu %d de %d\n", nome, k, n);
    if (k == 0)
    {
        return;
    }
    qsort(estados, k, sizeof(long), compara_long);
    qsort(picos, k, sizeof(long), compara_long);
    qsort(tempos, k, sizeof(double), compara_double);
    fprintf(stderr, "  estados visitados: média %.0f, mediana %ld, máximo %ld\n", soma_estados / k, estados[k/2], estados[k-1]);
    fprintf(stderr, "  tempo (ms): média %.3f, mediana %.3f, máximo %.3f\n", soma_tempo / k, tempos[k/2], tempos[k-1]);
    fprintf(stderr, "  caminho: média %.2f movimentos\n", soma_caminho / k);
    fprintf(stderr, "  pico de nós vivos: máximo %ld (~%.1f MiB com sizeof(No)=%zu)\n",
            picos[k-1], picos[k-1] * (double)sizeof(No) / 1048576.0, sizeof(No));
}

/* Gera 'quantidade' puzzles com semente fixa e compara as duas buscas.
 * CSV em stdout, resumo em stderr. */
static int roda_bench(int quantidade, unsigned long long semente_base, long max_nos_por_busca, int limite_ids)
{
    if (quantidade < 1 || quantidade > 1000)
    {
        fprintf(stderr, "quantidade deve estar entre 1 e 1000\n");
        return 1;
    }
    Medida largura[quantidade], iterativo[quantidade];
    int i, j, k;

    modo_silencioso = 1;
    max_nos = max_nos_por_busca;
    printf("puzzle,semente,estado_inicial,bfs_resolveu,bfs_caminho,bfs_estados,bfs_nos_pico,bfs_ms,"
           "ids_resolveu,ids_caminho,ids_limite,ids_estados_total,ids_estados_ultima_iteracao,ids_nos_pico,ids_ms\n");
    for (i = 0; i < quantidade; i++)
    {
        int puzzle[3][3];
        unsigned long long semente = semente_base + (unsigned long long)i;

        aleatorio_fixo = 1;
        aleatorio_estado = semente * 0x9E3779B97F4A7C15ULL ^ 0xD1B54A32D192ED03ULL;
        for (k = 0; k < 4; k++)
        {
            aleatorio();
        }
        gerar_puzzle_inicial(puzzle);
        aleatorio_fixo = 0;

        largura[i] = mede_largura(puzzle);
        iterativo[i] = mede_iterativo(puzzle, limite_ids);

        printf("%d,%llu,", i + 1, semente);
        for (j = 0; j < 9; j++)
        {
            printf("%d", puzzle[j / 3][j % 3]);
        }
        printf(",%d,%d,%ld,%ld,%.3f,%d,%d,%d,%ld,%ld,%ld,%.3f\n",
               largura[i].resolveu, largura[i].caminho, largura[i].estados, largura[i].nos_pico, largura[i].ms,
               iterativo[i].resolveu, iterativo[i].caminho, iterativo[i].limite, iterativo[i].estados,
               iterativo[i].estados_ultima, iterativo[i].nos_pico, iterativo[i].ms);
    }

    fprintf(stderr, "Medição: %d puzzles (sementes %llu a %llu), embaralhamento de 40 movimentos,\n",
            quantidade, semente_base, semente_base + (unsigned long long)quantidade - 1);
    fprintf(stderr, "limite de %ld nós por chamada de busca, IDS até o limite %d.\n\n", max_nos_por_busca, limite_ids);
    resume("Busca em largura", largura, quantidade);
    resume("Aprofundamento iterativo", iterativo, quantidade);

    int ambos = 0, iguais = 0;
    for (i = 0; i < quantidade; i++)
    {
        if (largura[i].resolveu && iterativo[i].resolveu)
        {
            ambos++;
            if (largura[i].caminho == iterativo[i].caminho)
            {
                iguais++;
            }
        }
    }
    fprintf(stderr, "\nResolvidos pelos dois: %d; com caminho de mesmo tamanho: %d\n", ambos, iguais);
    return 0;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); /* os fontes são UTF-8 */
#endif
    if (argc > 1 && strcmp(argv[1], "--bench") == 0)
    {
        /* uso: --bench [quantidade=30] [semente_base=1] [max_nos=5000000] [limite_ids=40] */
        int quantidade = argc > 2 ? atoi(argv[2]) : 30;
        unsigned long long semente_base = argc > 3 ? strtoull(argv[3], NULL, 10) : 1;
        long max_nos_por_busca = argc > 4 ? atol(argv[4]) : 5000000L;
        int limite_ids = argc > 5 ? atoi(argv[5]) : 40;
        return roda_bench(quantidade, semente_base, max_nos_por_busca, limite_ids);
    }
    int puzzle[3][3], puzzle_novo[3][3], numero_movimento, opcao;

    srand(time(NULL));
    gerar_puzzle_inicial(puzzle);

    do {
        printf("=== 8-Puzzle ===\n\n");
        printf("1 - Jogar\n");
        printf("2 - Busca em Largura (IA)\n");
        printf("3 - Aprofundamento Iterativo (IA)\n");
        printf("0 - Sair\n");
        printf("Escolha uma opção: ");
        int lido = le_inteiro(&opcao);
        if (lido == -1)
        {
            opcao = 0;
        }
        else if (lido == 0)
        {
            opcao = -1;
        }
        limpa_tela();
        printf("=== 8-Puzzle ===\n");

        switch(opcao)
        {
            case 1:
                copia_matriz(puzzle, puzzle_novo);
                while (completo(puzzle_novo) == 0)
                {
                    printf("  --- Jogue ---\n\n");
                    imprimir_puzzle(puzzle_novo);
                    printf("\nDigite a peça para mover: ");
                    lido = le_inteiro(&numero_movimento);
                    if (lido == -1)
                    {
                        numero_movimento = -1;
                    }
                    else if (lido == 0)
                    {
                        numero_movimento = -2; /* não corresponde a nenhuma peça */
                    }
                    limpa_tela();
                    if(numero_movimento == -1)
                    {
                        break;
                    }

                    int temp[3][3];
                    funcao_suc(puzzle_novo, temp, numero_movimento);
                    copia_matriz(temp, puzzle_novo);
                }
                if(completo(puzzle_novo) == 1)
                {
                    printf("Parabéns, você completou o puzzle!\n\n");
                }
                break;
            case 2:
                printf("--- Busca em Largura ---\n\n");
                busca_em_largura(puzzle);
                break;
            case 3:
                printf("--- Aprofundamento Iterativo (busca em profundidade com limite 1, 2, ..., N) ---\n\n");
                int i, achou = 0, limite;
                printf("\nDigite o limite máximo N de profundidade: ");
                if (le_inteiro(&limite) != 1)
                {
                    limite = 0;
                }
                for(i=1; i<=limite; i++)
                {
                    achou = busca_profundidade_limitada(puzzle, i);
                    if(achou == 1)
                    {
                        break;
                    }
                }
                if(achou == 0)
                {
                    printf("\nNenhuma solução encontrada até o limite %d\n", limite);
                }
                break;
            case 0:
                printf("\nEncerrando\n");
                break;
            default:
                printf("\nOpção inválida\n");
        }
    }while(opcao != 0);
    return 0;
}

