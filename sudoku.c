// para compilar este programa manualmente em linux:
// gcc -Wall -o sudoku sudoku.c telag2.c -lallegro_font -lallegro_color -lallegro_ttf -lallegro_primitives -lallegro
// para executar:
// ./sudoku

#include "telag2.h"
#include "tabuleiros.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MARGEM 47
double tempo_agora;

// cria algumas cores globais
cor_t fucsia = {1, 0.2, 0.8, 1};
cor_t transparente = {0, 0, 0, 0};
cor_t preto = {0, 0, 0, 1};
cor_t branco = {1, 1, 1, 1};
cor_t vermelho = {1, 0, 0, 1};
cor_t amarelo = {0.1, 0.6, 0.6, 1};
cor_t cinza = {0.6, 0.6, 0.6, 0.3};

typedef struct
{
    int value;
    bool changeable;
    bool mark[9];
} celula;

typedef struct
{
    celula celula[9][9];
    int id;
    int dificuldade;
} tabuleiro;

typedef struct
{
    int tamanho;    //"fixo" conforme o arquivo
    int quantidade; // vai aumentando conforme vai lendo
    tabuleiro *vetor;
} vetor_de_tabuleiros;

typedef struct
{
    char texto[20];
    double inicio;
} notificacao;

typedef struct
{
    // constantes, devem ser inicializadas antes da tela
    tamanho_t tamanho_tela;
    // necessários no início de cada partida
    tabuleiro tabuleiro;
    char player_name[20];
    bool play;
    bool gameover;
    bool partida;
    bool play_again;
    double data_inicio;
    int numero_atual;
    int pontos;
    int lin_jogador;
    int col_jogador;
    // nao precisa de estado inicial
    double tempo_de_jogo;
    rato_t mouse;
    notificacao notificacao[4];
    int pos_notif;
} jogo;

// função de erro

void error404(char msg[])
{
    printf("\nAlgo de errado não está certo %s\n", msg);
}

int sorteia_numero(int x)
{
    if (x < 0)
    {
        return 0; // Retorna 0 se x for negativo, para evitar comportamento indefinido
    }
    return rand() % (x + 1); // Retorna um número entre 0 e x, inclusive
}

void inicializa_vetor_de_tabuleiros(vetor_de_tabuleiros *tabs, int tam)
{
    tabs->tamanho = tam;
    tabs->quantidade = 0;
    tabs->vetor = malloc(tam * sizeof(tabuleiro));
    if (tabs->vetor == NULL)
        error404("com a inicialização do vetor de tabuleiros.");
}

tabuleiro le_um_tabuleiro(FILE *arq)
{
    tabuleiro tab;
    fscanf(arq, "%d", &tab.id);
    fscanf(arq, "%d", &tab.dificuldade);
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            fscanf(arq, "%d", &tab.celula[i][j].value);
        }
    }
    return tab;
}

void insere_tabuleiro_no_vetor(vetor_de_tabuleiros *tabs, tabuleiro tab)
{
    if (tabs->quantidade >= tabs->tamanho)
        error404("com a inserção do tabuleiro no vetor.");
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (tab.celula[i][j].value == 0)
            {
                tab.celula[i][j].changeable = true;
                for (int k = 0; k < 9; k++)
                {
                    tab.celula[i][j].mark[k] = false;
                }
            }
            else
            {
                tab.celula[i][j].changeable = false;
            }
        }
    }
    tabs->vetor[tabs->quantidade] = tab;
    tabs->quantidade++;
}

vetor_de_tabuleiros le_tabuleiros(char nome[])
{
    FILE *arq;
    arq = fopen(nome, "r");
    if (arq == NULL)
        error404("com a abertura do arquivo de tabuleiros.");
    int n_tabs;
    if (fscanf(arq, "%d", &n_tabs) != 1)
        error404("com a leitura do arquivo de tabuleiros."); // estava %n ???

    vetor_de_tabuleiros tabs;
    inicializa_vetor_de_tabuleiros(&tabs, n_tabs);

    for (int i = 0; i < n_tabs; i++)
    {
        tabuleiro tab = le_um_tabuleiro(arq);
        insere_tabuleiro_no_vetor(&tabs, tab);
    }

    fclose(arq);
    return tabs;
}

void libera_vetor_de_tabuleiros(vetor_de_tabuleiros *tabs)
{
    if (tabs->vetor != NULL)
    {
        free(tabs->vetor);  // Libera a memória alocada para o vetor de tabuleiros
        tabs->vetor = NULL; // Prevenir uso de ponteiro inválido após a liberação
    }
}

tabuleiro sorteia_tabuleiro()
{
    tabuleiro jogo;
    vetor_de_tabuleiros vector = le_tabuleiros("tabuleiros.txt");
    int number = sorteia_numero(vector.tamanho - 1);
    jogo = vector.vetor[number];
    libera_vetor_de_tabuleiros(&vector);
    return jogo;
}

void inicializa_jogo(jogo *sudoku)
{
    sudoku->play = (bool)true;
    sudoku->gameover = (bool)false;
    sudoku->partida = (bool)true;
    sudoku->play_again = (bool)false;
    sudoku->numero_atual = (int)0;
    // sudoku->data_inicio = tela_relogio(); da segmentation fault n sei pq
    sudoku->tabuleiro = sorteia_tabuleiro();
    sudoku->lin_jogador = 0;
    sudoku->col_jogador = 0;
}

void imprime_mensagem(jogo *sudoku)
{
    int j = 0;
    for (int i = 0; i < 4; i++)
    {
        if (tela_relogio() - sudoku->notificacao[i].inicio > 5)
        {
            strcpy(sudoku->notificacao[i].texto, "");
        }
        if (strlen(sudoku->notificacao[i].texto) > 0)
        {
            tela_retangulo((retangulo_t){(ponto_t){500, 210 + (45 * j)}, {150, 40}}, 3, vermelho, transparente);
            tela_texto((ponto_t){510, 210 + (45 * j) + 27}, 20, vermelho, sudoku->notificacao[i].texto);
            j++;
        }
    }
}

void cria_mensagem(jogo *sudoku, char msg[])
{
    notificacao mensagem;
    strcpy(mensagem.texto, msg);
    mensagem.inicio = tela_relogio();
    if (sudoku->pos_notif > 3)
    {
        sudoku->pos_notif = 0;
    }
    sudoku->notificacao[sudoku->pos_notif] = mensagem;
    sudoku->pos_notif++;
}

void grava_numero_no_tabuleiro(jogo *sudoku)
{
    int linha = sudoku->lin_jogador;
    int coluna = sudoku->col_jogador;

    if (sudoku->tabuleiro.celula[linha][coluna].changeable)
    {
        // verifica se o numero ja existe na coluna
        for (int i = 0; i < 9; i++)
        {
            if (sudoku->tabuleiro.celula[i][coluna].value == sudoku->numero_atual && sudoku->tabuleiro.celula[i][coluna].value != 0)
            {
                cria_mensagem(sudoku, "Coluna!");
                return;
            }
        }

        // verifica se o numero ja existe na linha
        for (int j = 0; j < 9; j++)
        {
            if (sudoku->tabuleiro.celula[linha][j].value == sudoku->numero_atual && sudoku->tabuleiro.celula[linha][j].value != 0)
            {
                cria_mensagem(sudoku, "Linha!");
                return;
            }
        }

        // verifica se o número ja existe no quadrado 3x3
        int startlinha = linha - linha % 3;
        int startcoluna = coluna - coluna % 3;
        for (int i = startlinha; i < startlinha + 3; i++)
        {
            for (int j = startcoluna; j < startcoluna + 3; j++)
            {
                if (sudoku->tabuleiro.celula[i][j].value == sudoku->numero_atual && sudoku->tabuleiro.celula[i][j].value != 0)
                {
                    cria_mensagem(sudoku, "Bloco 3x3!");
                    return;
                }
            }
        }

        // se passou por todas as verificacoes, insire o numero
        sudoku->tabuleiro.celula[linha][coluna].value = sudoku->numero_atual;
    }
    else
    {
        cria_mensagem(sudoku, "Número fixo!");
    }
}

typedef struct
{
    retangulo_t botao;
    char texto[20];
    enum
    {
        clicado,
        emcima,
        nada
    } estado_botao;
} button;

button cria_botao(ponto_t ponto, tamanho_t tamanho, char *texto)
{
    button b;
    b.botao.inicio = ponto;
    b.botao.tamanho = tamanho;
    strcpy(b.texto, texto); // Corrigido para usar strcpy
    b.estado_botao = nada;
    return b;
}

void estado_botao(button *b, rato_t mouse)
{ // Adiciona mouse como parâmetro
    if (mouse.posicao.x >= b->botao.inicio.x && mouse.posicao.x <= b->botao.inicio.x + b->botao.tamanho.largura &&
        mouse.posicao.y >= b->botao.inicio.y && mouse.posicao.y <= b->botao.inicio.y + b->botao.tamanho.altura)
    {
        if (mouse.clicado[0])
        {
            b->estado_botao = clicado;
        }
        else
        {
            b->estado_botao = emcima;
        }
    }
    else
    {
        b->estado_botao = nada;
    }
}

void desenha_botao(button *b, cor_t cor_base, cor_t cor_emcima, cor_t cor_clicado)
{
    cor_t cor_atual = cor_base;
    if (b->estado_botao == emcima)
    {
        cor_atual = cor_emcima;
    }
    else if (b->estado_botao == clicado)
    {
        cor_atual = cor_clicado;
    }
    tela_retangulo(b->botao, 2, preto, cor_atual);
    tela_texto((ponto_t){b->botao.inicio.x + b->botao.tamanho.largura / 2, b->botao.inicio.y + b->botao.tamanho.altura / 2}, 20, preto, b->texto);
}

void atualiza_botao(button *b)
{
    rato_t mouse = tela_rato();
    estado_botao(b, mouse);
}

void desenha_numeros(jogo *sudoku)
{
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (sudoku->tabuleiro.celula[i][j].value > 0)
            {
                char valor[2];
                sprintf(valor, "%d", sudoku->tabuleiro.celula[i][j].value);
                if (sudoku->tabuleiro.celula[i][j].changeable)
                {
                    tela_texto((ponto_t){MARGEM + (45 * j) + 15, MARGEM + (45 * i) + 30}, 20, branco, valor);
                }
                else
                {
                    tela_texto((ponto_t){MARGEM + (45 * j) + 15, MARGEM + (45 * i) + 30}, 20, vermelho, valor);
                }
            }
            else
            {
                for (int k = 0; k < 9; k++)
                {
                    char num[2];
                    sprintf(num, "%d", k + 1);
                    // Calcula a posição relativa dentro do quadrado 3x3
                    int offsetX = (k % 3) * 11; // Deslocamento horizontal, 3 números por linha
                    int offsetY = (k / 3) * 11; // Deslocamento vertical, 3 linhas

                    // Posição de início ajustada pelo deslocamento calculado
                    int posX = MARGEM + (45 * j) + 8 + offsetX;
                    int posY = MARGEM + (45 * i) + 14 + offsetY;

                    // Desenha o número na posição calculada
                    tela_texto((ponto_t){posX, posY}, 10, cinza, num);

                    if (sudoku->tabuleiro.celula[i][j].mark[k])
                    {
                        // Calcula os pontos para o "X"
                        int linhaLargura = 4; // Largura da linha do "X"
                        ponto_t inicioX1 = {posX - linhaLargura + 3, posY - linhaLargura - 3};
                        ponto_t fimX1 = {posX + linhaLargura + 3, posY + linhaLargura - 3};
                        ponto_t inicioX2 = {posX + linhaLargura + 3, posY - linhaLargura - 3};
                        ponto_t fimX2 = {posX - linhaLargura + 3, posY + linhaLargura - 3};

                        // Desenha o "X" em cima do número
                        tela_linha(inicioX1, fimX1, 2, vermelho); // Desenha a primeira linha do "X"
                        tela_linha(inicioX2, fimX2, 2, vermelho); // Desenha a segunda linha do "X"
                    }
                }
            }
        }
    }
}

void desenha_numero_atual(jogo *sudoku)
{
    tela_retangulo((retangulo_t){{500, MARGEM}, {150, 150}}, 2, branco, transparente);
    tela_retangulo((retangulo_t){{505, MARGEM + 5}, {140, 140}}, 2, branco, transparente);
    if (sudoku->numero_atual == 0)
    {
        tela_texto((ponto_t){520, 130}, 27, branco, "APAGAR");
    }
    else
    {
        char numero[2];
        sprintf(numero, "%d", sudoku->numero_atual);
        tela_texto((ponto_t){545, 157}, 100, branco, numero);
    }
}

void desenha_tela_jogo(jogo *sudoku)
{
    // linhas da grade do tabuleiro do sudoku
    for (int i = 0; i < 10; i++)
    {
        if (i == 0 || i % 3 == 0)
        {
            tela_linha((ponto_t){MARGEM, MARGEM + (45 * i)}, (ponto_t){452, MARGEM + (45 * i)}, 3, branco);
            tela_linha((ponto_t){MARGEM + (45 * i), MARGEM}, (ponto_t){MARGEM + (45 * i), 452}, 3, branco);
        }
        else
        {
            tela_linha((ponto_t){MARGEM, MARGEM + (45 * i)}, (ponto_t){452, MARGEM + (45 * i)}, 1, branco);
            tela_linha((ponto_t){MARGEM + (45 * i), MARGEM}, (ponto_t){MARGEM + (45 * i), 452}, 1, branco);
        }
    }

    // números no tabuleiro
    desenha_numeros(sudoku);

    // quadrado em que aparece o número atual
    desenha_numero_atual(sudoku);

    // desenha jogador
    tela_retangulo((retangulo_t){{MARGEM + (sudoku->col_jogador * 45), MARGEM + (sudoku->lin_jogador * 45)}, {45, 45}}, 3, vermelho, transparente);

    // botao de desistir
    button quit = cria_botao((ponto_t){500, 403}, (tamanho_t){150, 50}, "Quit!");
    atualiza_botao(&quit);                            // Atualiza o estado do botão com a posição do mouse
    desenha_botao(&quit, branco, vermelho, vermelho); // Desenha com as cores base, em cima e clicado

    if (quit.estado_botao == clicado)
    {
        exit(0); // Sai do jogo se o botão for clicado
    }

    tela_atualiza();
}

void desenha_tela_inicio(jogo *sudoku)
{
    while (sudoku->play)
    {

        button start = cria_botao((ponto_t){275, 225}, (tamanho_t){150, 50}, "Play!");
        atualiza_botao(&start);
        desenha_botao(&start, branco, fucsia, fucsia);

        if (start.estado_botao == clicado)
        {
            sudoku->play = false;
        }
        tela_atualiza();
    }
}

void desenha_tela_play_again(jogo *sudoku)
{
    while (sudoku->play_again)
    {
        button play_again = cria_botao((ponto_t){275, 225}, (tamanho_t){150, 50}, "PlayAgain!");
        atualiza_botao(&play_again);
        desenha_botao(&play_again, branco, fucsia, fucsia);

        if (play_again.estado_botao == clicado)
        {
            inicializa_jogo(sudoku);
            sudoku->partida = true;
            sudoku->play_again = false;
        }
        // if (wannaQuit) {
        //     sudoku.gameover = true;
        // }
        tela_atualiza();
    }
}

void processa_teclado_jogo(jogo *sudoku)
{
    char c = tela_tecla();
    if (c != 0)
    {
        printf("Tecla pressionada: %d\n", c);
        switch (c)
        {
        case 43: // tecla +
            if (sudoku->numero_atual < 9)
            {
                sudoku->numero_atual++;
            }
            break;

        case 45: // tecla -
            if (sudoku->numero_atual > 0)
            {
                sudoku->numero_atual--;
            }
            break;

        case 'w': // tecla W, move para cima
            if (sudoku->lin_jogador > 0)
            {
                sudoku->lin_jogador--;
            }
            break;

        case 's': // tecla S, move para baixo
            if (sudoku->lin_jogador < 8)
            {
                sudoku->lin_jogador++;
            }
            break;

        case 'a': // tecla A, move para a esquerda
            if (sudoku->col_jogador > 0)
            {
                sudoku->col_jogador--;
            }
            break;

        case 'd': // tecla D, move para a direita
            if (sudoku->col_jogador < 8)
            {
                sudoku->col_jogador++;
            }
            break;

        case '\n': // tecla enter
            grava_numero_no_tabuleiro(sudoku);
            break;

        case 'm': // tecla m
            if (sudoku->tabuleiro.celula[sudoku->lin_jogador][sudoku->col_jogador].mark[sudoku->numero_atual - 1])
            {
                sudoku->tabuleiro.celula[sudoku->lin_jogador][sudoku->col_jogador].mark[sudoku->numero_atual - 1] = false;
            }
            else
            {
                sudoku->tabuleiro.celula[sudoku->lin_jogador][sudoku->col_jogador].mark[sudoku->numero_atual - 1] = true;
            }
            break;

        case '0':
            sudoku->numero_atual = 0;
            break;

        case '1':
            sudoku->numero_atual = 1;
            break;

        case '2':
            sudoku->numero_atual = 2;
            break;

        case '3':
            sudoku->numero_atual = 3;
            break;

        case '4':
            sudoku->numero_atual = 4;
            break;

        case '5':
            sudoku->numero_atual = 5;
            break;

        case '6':
            sudoku->numero_atual = 6;
            break;

        case '7':
            sudoku->numero_atual = 7;
            break;

        case '8':
            sudoku->numero_atual = 8;
            break;

        case '9':
            sudoku->numero_atual = 9;
            break;

        default:
            // n acontece nd
            break;
        }
    }
}

void processa_tempo(jogo *sudoku)
{
    sudoku->tempo_de_jogo = tela_relogio() - sudoku->data_inicio;
}

void processa_pontuacao(jogo *sudoku)
{
    sudoku->pontos = 1285 + 97418 * (sudoku->tabuleiro.dificuldade + 1) / sudoku->tempo_de_jogo;
}

void ganhou(jogo *sudoku)
{
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (sudoku->tabuleiro.celula[i][j].value == 0)
                return;
        }
    }
    sudoku->partida = false;
    sudoku->play_again = true;
}

void joga_partida(jogo *sudoku)
{
    while (sudoku->partida)
    {
        processa_teclado_jogo(sudoku);
        imprime_mensagem(sudoku);
        desenha_tela_jogo(sudoku);
        ganhou(sudoku);
    }
}

int main()
{
    srand(time(NULL));
    jogo sudoku;
    sudoku.tamanho_tela = (tamanho_t){700, 500};
    inicializa_jogo(&sudoku);

    // inicializa a tela gráfica
    tela_inicio(sudoku.tamanho_tela, "Sudoku");

    desenha_tela_inicio(&sudoku);

    while (!sudoku.gameover)
    {
        joga_partida(&sudoku);
        processa_tempo(&sudoku);
        processa_pontuacao(&sudoku);
        // grava_record(&sudoku);
        desenha_tela_play_again(&sudoku);
    }

    // encerra a tela gráfica
    tela_fim();
}

// funcao que le arquivo e retorna um vetor de structs (INICO DO MAIN)
// OUUU alocar com 1 a mais
// ler o arquivo com o numero de entradas atual
// struct do aruivo inteiro
// struct de cada "entrada": nome ntabuleiro pontuacao

// leitura do arquivo de tabuleiros
// sortear :
// le qtos tabs tem
// sorteia um num entre 0 e os tabs q tem
// chama uma funcao que le num vezes

// sortear: le todos e deixa na memória e sorteia
// sortear: deixar tds os numeros

// EM QUALQUER CASO: funcao que recebe um arquivo ja aberto e retorna uma struct do próximo tabuleiro
