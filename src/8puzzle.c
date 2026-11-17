#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void funcao_suc(int puzzle_antigo[3][3], int puzzle_novo[3][3], int numero_movido)
{
    copia_matriz(puzzle_antigo, puzzle_novo);
    int linha_zero = -1, coluna_zero = -1;
    int achou = 0;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (puzzle_novo[i][j] == 0)
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

    if (linha_zero > 0 && puzzle_novo[linha_zero - 1][coluna_zero] == numero_movido)
    {
        puzzle_novo[linha_zero][coluna_zero] = numero_movido;
        puzzle_novo[linha_zero - 1][coluna_zero] = 0;
    }
    else
    {
        if (linha_zero < 2 && puzzle_novo[linha_zero + 1][coluna_zero] == numero_movido)
        {
            puzzle_novo[linha_zero][coluna_zero] = numero_movido;
            puzzle_novo[linha_zero + 1][coluna_zero] = 0;
        }
        else
        {
            if (coluna_zero > 0 && puzzle_novo[linha_zero][coluna_zero - 1] == numero_movido)
            {
                puzzle_novo[linha_zero][coluna_zero] = numero_movido;
                puzzle_novo[linha_zero][coluna_zero - 1] = 0;
            }
            else
            {
                if (coluna_zero < 2 && puzzle_novo[linha_zero][coluna_zero + 1] == numero_movido)
                {
                    puzzle_novo[linha_zero][coluna_zero] = numero_movido;
                    puzzle_novo[linha_zero][coluna_zero + 1] = 0;
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

    for (x = 0; x < 100; x++)
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

        int mov_idx = rand() % num_movs;
        int i_troca = movs_possiveis[mov_idx][0];
        int j_troca = movs_possiveis[mov_idx][1];

        puzzle[linha_zero][coluna_zero] = puzzle[i_troca][j_troca];
        puzzle[i_troca][j_troca] = 0;
    }
}


int main()
{
    int puzzle[3][3];
    int puzzle_novo[3][3];
    int numero_movimento;

    srand(time(NULL));
    gerar_puzzle_inicial(puzzle);

    while (completo(puzzle) == 0)
    {
        limpa_tela();
        imprimir_puzzle(puzzle);

        printf("\nDigite o numero que voce quer mover para o espaco vazio: ");
        int lido = le_inteiro(&numero_movimento);
        if (lido == -1)
        {
            return 0; /* a entrada acabou: encerra sem declarar vitória */
        }
        if (lido == 0)
        {
            numero_movimento = -2; /* não corresponde a nenhuma peça */
        }

        funcao_suc(puzzle, puzzle_novo, numero_movimento);

        copia_matriz(puzzle_novo, puzzle);
    }

    limpa_tela();
    imprimir_puzzle(puzzle);
    printf("\nvoce completou o puzzle\n");

    return 0;
}

