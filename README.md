<h1>Sudoku</h1>
<p>Este jogo foi desenvolvido nas aulas de Laboratório de Programação I, curso de Sistemas de Informação, UFSM, com o professor Benhur Stein.</p>
<p>O programa utiliza-se do arquivo <b>telag2.c</b>, desenvolvido pelo professor para auxiliar no desenvolvimento do jogo.</p>
<h2>Rodando o programa e dependências</h2>
<p>O jogo foi desenvolvido em um ambiente Linux (WSL - Ubuntu) e faz uso da biblioteca <a href="https://www.ufsm.br/pet/sistemas-de-informacao/2018/08/28/como-instalar-o-allegro-5">Allegro 5</a>.</p>
<ul>
  <li>Compilação manual: <b>gcc -Wall -o sudoku sudoku.c telag2.c -lallegro_font -lallegro_color -lallegro_ttf -lallegro_primitives -lallegro</b></li>
  <li>Para executar: <b>./sudoku</b></li>
</ul>
<h2>Como jogar?</h2>
<p>Basicamente, o jogo funciona com comandos do teclado, e os botões com os clicks do mouse.</p>
<ul>
  <li>Enter
    <ul>
      <li>Na tela de início => inicia o jogo;</li>
      <li>Na tela de jogo => atribui o número atual à casa no tabuleiro onde o player está.</li>
    </ul>
  </li>
  <li>A, W, S, D => controlam a movimentação do player no tabuleiro;</li>
  <li>M => controla as marcações. (on/off, com base no número atual);</li>
  <li>+ e - => aumenta/diminui o número atual;</li>
  <li>0 => coloca o número atual no modo de "apagar";</li>
  <li>1, 2, 3, 4, 5, 6, 7, 8 e 9 => controlam qual será o número atual.</li>
</ul>
<h2>Imagens</h2>
<p>Seguem abaixo algumas imagens da interface do jogo.</p>
<a href="img/cover.png"></a>
<a href="img/game_screen.png"></a>
<a href="img/playing_screen.png"></a>
<a href="img/score.png"></a>
