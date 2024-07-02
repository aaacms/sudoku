// para compilar este programa manualmente em linux:
// gcc -Wall -o sudoku sudoku.c telag2.c -lallegro_font -lallegro_color -lallegro_ttf -lallegro_primitives -lallegro
// para executar:
// ./sudoku

#include "telag2.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define MARGEM 47

// cria algumas cores globais
cor_t fucsia = { 1, 0.2, 0.8, 1 };
cor_t transparente = { 0, 0, 0, 0 };
cor_t preto = { 0, 0, 0, 1 };
cor_t branco = { 1, 1, 1, 1 };
cor_t vermelho = { 1, 0, 0, 1 };
cor_t amarelo = { 0.1, 0.6, 0.6, 1 };

typedef struct {
    int value;
    bool changeable;
    bool mark[9];
} celula;

typedef struct {
    char mat[9][9];
    int id;
    int dificuldade;
} tabuleiro;

typedef struct {
    int tamanho; //"fixo" conforme o arquivo
    int quantidade; //vai aumentando conforme vai lendo
    tabuleiro *vetor;
} vetor_de_tabuleiros;

typedef struct {
    celula tabuleiro[81];
    char playerName[20];
    bool gameover;
    bool partidaIsOver;
    int numeroAtual;
    tamanho_t tamanho_tela;
} jogo;

//função de erro

void error404(char msg[]) 
{
    printf("\nAlgo de errado não está certo %s\n", msg);
}

void inicializa_vetor_de_tabuleiros(vetor_de_tabuleiros *tabs, int tam) 
{
    tabs->tamanho = tam;
    tabs->quantidade = 0;
    tabs->vetor = malloc(tam * sizeof(tabuleiro));
    if (tabs->vetor == NULL) error404("com a inicialização do vetor de tabuleiros.");

}

void insere_tabuleiro_no_vetor(vetor_de_tabuleiros *tabs, tabuleiro tab) 
{
    if (tabs->quantidade >= tabs->tamanho) error404("com a inserção do tabuleiro no vetor.");
    tabs->vetor[tabs->quantidade] = tab;
    tabs->quantidade++;
}

void le_um_tabuleiro(FILE *arq) 
{

}

vetor_de_tabuleiros le_tabuleiros(char nome[]) 
{
    FILE *arq;
    arq = fopen(nome, "r");
    if (arq == NULL) error404("com a abertura do arquivo de tabuleiros.");
    int n_tabs;
    if (fscanf(arq, "%n", &n_tabs) != 1) error404("com a leitura do arquivo de tabuleiros.");

    vetor_de_tabuleiros tabs;
    inicializa_vetor_de_tabuleiros(&tabs, n_tabs);

    for (int i = 0; i < n_tabs; i++) 
    {
        //tabuleiro tab = le_um_tabuleiro(arq);
        //insere_tabuleiro_no_vetor(&tabs, tab);
    }

    fclose(arq);
    return tabs;
}

void desenha_tela_jogo(jogo *sudoku) 
{
    //linhas da grade do tabuleiro do sudoku
    for (int i = 0; i < 10; i++) {
        tela_linha((ponto_t){MARGEM, MARGEM + (45 * i)}, (ponto_t){452, MARGEM + (45 * i)}, 2, branco);
        tela_linha((ponto_t){MARGEM + (45 * i), MARGEM}, (ponto_t){MARGEM + (45 * i), 452}, 2, branco);
    }
    //quadrado em que aparece o número atual
    tela_retangulo((retangulo_t){{500, MARGEM}, {150, 150}}, 2, branco, transparente);
    tela_retangulo((retangulo_t){{505, MARGEM + 5}, {140, 140}}, 2, branco, transparente);

    //botao de desistir
    tela_retangulo((retangulo_t){{500, 230}, {150, 50}}, 2, branco, branco);
    tela_texto((ponto_t){545, 262}, 25, preto, "Quit!");


    tela_atualiza();
}

void desenha_tela_inicio(jogo *sudoku) 
{

}

void desenha_tela_fim(jogo *sudoku) 
{
    
}

void inicializa_jogo(jogo *sudoku) 
{
    sudoku->gameover = (bool)false;
    sudoku->tamanho_tela = (tamanho_t){ 700, 500 };
    sudoku->partidaIsOver = (bool)false;
}

int main() 
{   
    jogo sudoku;
    inicializa_jogo(&sudoku);
    
    // inicializa a tela gráfica
    tela_inicio(sudoku.tamanho_tela, "Sudoku");

    while(!sudoku.gameover) {
        while(!sudoku.partidaIsOver) {
            desenha_tela_jogo(&sudoku);
            // if (ganhou) {
            //     sudoku.partidaIsOver = true;
            // }
        }
        // if (wannaPlayAgain) {
        //     sudoku.partidaIsOver = false;
        // }
        // if(wannaQuit) {
        //     sudoku.gameover = true;
        // }
    }

}

// funcao que le arquivo e retorna um vetor de structs (INICO DO MAIN)
// OUUU alocar com 1 a mais
    // ler o arquivo com o numero de entradas atual
    // struct do aruivo inteiro
    // struct de cada "entrada": nome ntabuleiro pontuacao




// leitura do arquivo de tabuleiros
// sortear :
        //le qtos tabs tem
        //sorteia um num entre 0 e os tabs q tem 
        //chama uma funcao que le num vezes

// sortear: le todos e deixa na memória e sorteia
// sortear: deixar tds os numeros 

// EM QUALQUER CASO: funcao que recebe um arquivo ja aberto e retorna uma struct do próximo tabuleiro
