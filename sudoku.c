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
    char player_name[10];
    int id_tab;
    int pontos;
} player;

typedef struct
{
    int tamanho;    //"fixo" conforme o arquivo
    int quantidade; // vai aumentando conforme vai lendo
    player *vetor;
} vetor_de_recordes;

typedef struct
{
    // constantes, devem ser inicializadas antes da tela
    tamanho_t tamanho_tela;
    // necessários no início de cada partida
    tabuleiro tabuleiro;
    player player_atual;
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
    if (fscanf(arq, "%d", &tab.id) != 1)
        error404("com a leitura do ID do tabuleiro.");
    if (fscanf(arq, "%d", &tab.dificuldade) != 1)
        error404("com a leitura da dificuldade do tabuleiro.");
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (fscanf(arq, "%d", &tab.celula[i][j].value) != 1)
                error404("com a leitura de uma célula do tabuleiro.");
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
            tab.celula[i][j].changeable = (tab.celula[i][j].value == 0);
            for (int k = 0; k < 9; k++)
            {
                tab.celula[i][j].mark[k] = false;
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

tabuleiro sorteia_tabuleiro()
{
    vetor_de_tabuleiros vector = le_tabuleiros("tabuleiros.txt");
    if (vector.quantidade == 0)
    {
        error404(": não há nenhum tabuleiro disponível.");
        tabuleiro tab_vazio = {0};
        return tab_vazio;
    }

    int number = sorteia_numero(vector.tamanho - 1);
    tabuleiro jogo = vector.vetor[number];
    free(vector.vetor);
    return jogo;
}

void inicializa_vetor_de_recordes(vetor_de_recordes *records, int tam)
{
    records->tamanho = tam;
    records->quantidade = 0;
    records->vetor = malloc(tam * sizeof(player));
    if (records->vetor == NULL)
        error404("com a inicialização do vetor de records.");
}

player le_um_record(FILE *arq)
{
    player pl;
    if (fscanf(arq, "%s %d %d", pl.player_name, &pl.id_tab, &pl.pontos) != 3)
        error404("com a leitura de um record.");
    return pl;
}

void insere_player_no_vetor(vetor_de_recordes *records, player pl)
{
    if (records->quantidade >= records->tamanho)
        error404("com a inserção do player no vetor.");

    records->vetor[records->quantidade] = pl;
    records->quantidade++;
}

vetor_de_recordes le_recordes()
{
    FILE *arq;
    arq = fopen("recordes.txt", "r");
    if (arq == NULL)
    {
        error404("com a abertura do arquivo de recordes.");
    }
    int n_records;
    if (fscanf(arq, "%d", &n_records) != 1)
        error404("com a leitura do arquivo de recordes."); // estava %n ???

    vetor_de_recordes records;
    inicializa_vetor_de_recordes(&records, n_records + 1);

    for (int i = 0; i < n_records; i++)
    {
        player pl = le_um_record(arq);
        insere_player_no_vetor(&records, pl);
    }

    fclose(arq);
    return records;
}

void grava_record_atual(vetor_de_recordes *records, jogo *sudoku)
{
    FILE *arq;
    arq = fopen("recordes.txt", "w");
    if (arq == NULL)
        error404("com a abertura do arquivo de recordes.");

    fprintf(arq, "%d\n", records->quantidade + 1);

    // imprimir no arquivo
    bool adicionou = false;
    for (int i = 0; i < records->quantidade; i++)
    {
        if (!adicionou && sudoku->player_atual.pontos > records->vetor[i].pontos)
        {
            fprintf(arq, "%s %d %d\n", sudoku->player_atual.player_name, sudoku->player_atual.id_tab, sudoku->player_atual.pontos);
            adicionou = true;
        }
        fprintf(arq, "%s %d %d\n", records->vetor[i].player_name, records->vetor[i].id_tab, records->vetor[i].pontos);
    }
    if (!adicionou)
    {
        fprintf(arq, "%s %d %d\n", sudoku->player_atual.player_name, sudoku->player_atual.id_tab, sudoku->player_atual.pontos);
    }
    fclose(arq);

    // atualizar vetor com a nova lista de records no aqrquivo
    *records = le_recordes();
}

void inicializa_jogo(jogo *sudoku)
{
    sudoku->play = (bool)true;
    sudoku->gameover = (bool)false;
    sudoku->partida = (bool)true;
    sudoku->play_again = (bool)false;
    sudoku->numero_atual = (int)0;
    sudoku->tabuleiro = sorteia_tabuleiro();
    sudoku->lin_jogador = 0;
    sudoku->col_jogador = 0;
    strcpy(sudoku->player_atual.player_name, "");
    sudoku->player_atual.id_tab = sudoku->tabuleiro.id;
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

button cria_botao(ponto_t ponto, tamanho_t tamanho, char *texto)
{
    button b;
    b.botao.inicio = ponto;
    b.botao.tamanho = tamanho;
    strcpy(b.texto, texto); // Corrigido para usar strcpy
    b.estado_botao = nada;
    return b;
}

void desenha_botao(button *b, cor_t cor_base, cor_t cor_emcima)
{
    cor_t cor_atual = cor_base;
    if (b->estado_botao == emcima)
    {
        cor_atual = cor_emcima;
    }
    tela_retangulo(b->botao, 2, preto, cor_atual);
    tela_texto((ponto_t){b->botao.inicio.x + 15, b->botao.inicio.y + 30}, 20, preto, b->texto);
}

void atualiza_botao(button *b, jogo *sudoku)
{
    if (sudoku->mouse.posicao.x >= b->botao.inicio.x && sudoku->mouse.posicao.x <= b->botao.inicio.x + b->botao.tamanho.largura &&
        sudoku->mouse.posicao.y >= b->botao.inicio.y && sudoku->mouse.posicao.y <= b->botao.inicio.y + b->botao.tamanho.altura)
    {
        if (sudoku->mouse.clicado[0])
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
    atualiza_botao(&quit, sudoku);
    desenha_botao(&quit, branco, cinza);

    if (quit.estado_botao == clicado)
    {
        sudoku->partida = false;
        sudoku->gameover = true;
    }

    tela_atualiza();
}

void le_player_name(jogo *sudoku)
{
    char c = tela_tecla();
    if (c != 0)
    {
        if (c == '\n')
        {
            sudoku->data_inicio = tela_relogio();
            sudoku->play = false;
        }
        else if (c == '\b')
        {
            if (strlen(sudoku->player_atual.player_name) > 0)
            {
                sudoku->player_atual.player_name[strlen(sudoku->player_atual.player_name) - 1] = '\0';
            }
        }
        else if (c == ' ')
        {
            tela_texto((ponto_t){320, 315}, 10, fucsia, "Sem espaços!");
        }
        else
        {
            if (strlen(sudoku->player_atual.player_name) < 10)
            {
                size_t len = strlen(sudoku->player_atual.player_name);
                sudoku->player_atual.player_name[len] = c;
                sudoku->player_atual.player_name[len + 1] = '\0';
            }
        }
    }
}

void desenha_tela_inicio(jogo *sudoku)
{
    button start = cria_botao((ponto_t){275, 330}, (tamanho_t){150, 50}, "Play! [Enter]");
    while (sudoku->play)
    {
        tela_texto((ponto_t){290, 70}, 20, branco, "Welcome to");
        tela_texto((ponto_t){180, 160}, 80, branco, "SUDOKU");
        le_player_name(sudoku);
        tela_texto((ponto_t){235, 230}, 20, branco, "Enter the player name:");
        tela_retangulo((retangulo_t){(ponto_t){240, 250}, (tamanho_t){220, 50}}, 3, branco, transparente);
        tela_texto((ponto_t){255, 280}, 20, fucsia, sudoku->player_atual.player_name);

        sudoku->mouse = tela_rato();
        desenha_botao(&start, branco, cinza);
        atualiza_botao(&start, sudoku);

        if (start.estado_botao == clicado)
        {
            sudoku->data_inicio = tela_relogio();
            sudoku->play = false;
        }

        tela_atualiza();
    }
}

void imprime_score(vetor_de_recordes *records, jogo *sudoku)
{
    tela_texto((ponto_t){210, 70}, 30, branco, "SCORE");
    tela_texto((ponto_t){60, 120}, 20, branco, "Player:");
    tela_texto((ponto_t){260, 120}, 20, branco, "Tabuleiro:");
    tela_texto((ponto_t){400, 120}, 20, branco, "Pontos:");
    int now;
    for (int j = 0; j < records->quantidade; j++)
    {
        if (records->vetor[j].pontos == sudoku->player_atual.pontos)
        {
            now = j;
        }
    }

    if (records->quantidade <= 11 || now <= 11)
    {
        for (int i = 0; i < records->quantidade; i++)
        {
            cor_t cor = branco;
            if (sudoku->player_atual.pontos == records->vetor[i].pontos)
            {
                cor = fucsia;
            }
            char tab[10];
            sprintf(tab, "%d", records->vetor[i].id_tab);
            char score[10];
            sprintf(score, "%d", records->vetor[i].pontos);
            tela_texto((ponto_t){60, 150 + (30 * i)}, 20, cor, records->vetor[i].player_name);
            tela_texto((ponto_t){260, 150 + (30 * i)}, 20, cor, tab);
            tela_texto((ponto_t){400, 150 + (30 * i)}, 20, cor, score);
            if (i == 10)
                break;
        }
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            char tab[10];
            sprintf(tab, "%d", records->vetor[i].id_tab);
            char score[10];
            sprintf(score, "%d", records->vetor[i].pontos);
            tela_texto((ponto_t){60, 150 + (30 * i)}, 20, branco, records->vetor[i].player_name);
            tela_texto((ponto_t){260, 150 + (30 * i)}, 20, branco, tab);
            tela_texto((ponto_t){400, 150 + (30 * i)}, 20, branco, score);
        }
        tela_texto((ponto_t){210, 300}, 50, branco, "...");

        for (int k = now - 1; k <= now + 1; k++)
        {
            cor_t cor = branco;
            if (sudoku->player_atual.pontos == records->vetor[k].pontos)
            {
                cor = fucsia;
            }
            char tab[10];
            sprintf(tab, "%d", records->vetor[k].id_tab);
            char score[10];
            sprintf(score, "%d", records->vetor[k].pontos);
            tela_texto((ponto_t){60, 320 + (30 * k)}, 20, cor, records->vetor[k].player_name);
            tela_texto((ponto_t){260, 320 + (30 * k)}, 20, cor, tab);
            tela_texto((ponto_t){400, 320 + (30 * k)}, 20, cor, score);
        }
    }
}

void desenha_tela_play_again(jogo *sudoku, vetor_de_recordes *records)
{
    button play_again = cria_botao((ponto_t){500, 343}, (tamanho_t){150, 50}, "PlayAgain!");
    button quit = cria_botao((ponto_t){500, 403}, (tamanho_t){150, 50}, "Quit!");
    while (sudoku->play_again)
    {
        sudoku->mouse = tela_rato();

        // Score
        imprime_score(records, sudoku);

        // botao de play again
        atualiza_botao(&play_again, sudoku);
        desenha_botao(&play_again, branco, cinza);

        if (play_again.estado_botao == clicado)
        {
            char pl[10];
            strcpy(pl, sudoku->player_atual.player_name);
            inicializa_jogo(sudoku);
            strcpy(sudoku->player_atual.player_name, pl);
            sudoku->data_inicio = tela_relogio();
            sudoku->partida = true;
            sudoku->play_again = false;
        }

        // botao de desistir
        atualiza_botao(&quit, sudoku);
        desenha_botao(&quit, branco, cinza);

        if (quit.estado_botao == clicado)
        {
            sudoku->play_again = false;
            sudoku->gameover = true;
        }

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
    sudoku->player_atual.pontos = (1285 + (97418 * (sudoku->tabuleiro.dificuldade + 1))) / sudoku->tempo_de_jogo;
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
    processa_tempo(sudoku);
    processa_pontuacao(sudoku);
    sudoku->play_again = true;
}

void joga_partida(jogo *sudoku)
{
    while (sudoku->partida)
    {
        sudoku->mouse = tela_rato();
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

    // inicializa a tela gráfica
    tela_inicio(sudoku.tamanho_tela, "Sudoku");
    inicializa_jogo(&sudoku);

    desenha_tela_inicio(&sudoku);

    while (!sudoku.gameover)
    {
        joga_partida(&sudoku);
        if (!sudoku.gameover)
        {
            vetor_de_recordes records = le_recordes();
            grava_record_atual(&records, &sudoku);
            desenha_tela_play_again(&sudoku, &records);
            free(records.vetor);
        }
    }

    // encerra a tela gráfica
    tela_fim();
}
