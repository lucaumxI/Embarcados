# PROJETO DE SISTEMAS COMPUTACIONAIS EMBARCADOS

## Prof. Dr. Emerson Carlos Pedrino  
## Aluno: Lucas Nunes Garcia - Engenharia de Computação  
## Departamento de Computação (DC) - UFSCar

---

# Projetos de Jogos em Sistemas Embarcados

## StockCar (AT89S52) e Jogo Dino (DE10)

Este repositório reúne dois projetos desenvolvidos em linguagem C, cada um voltado para uma plataforma específica. Ambos têm como objetivo explorar conceitos de programação de baixo nível, interação com hardware e desenvolvimento de jogos simples em ambientes com recursos limitados.

**StockCar — Microcontrolador AT89S52 com Display Gráfico (GLCD)**  
Este projeto consiste em um jogo de corrida desenvolvido para o microcontrolador AT89S52, utilizando um display gráfico GLCD 128x64 como interface visual. O jogador controla um carro que deve desviar de obstáculos em uma pista com curvas geradas aleatoriamente.

### Principais características:
- Controle gráfico direto via comandos ao GLCD  
- Geração de obstáculos e curvas com uso de LFSR para pseudoaleatoriedade  
- Entrada por teclado matricial  
- Atualização da lógica do jogo com temporizador via interrupção (Timer0)  
- Exibição de placar e detecção de colisões em tempo real

---

**Jogo Dino — Plataforma DE10-SoC**  
Inspirado no jogo do dinossauro do navegador Google Chrome, este projeto foi implementado para a plataforma DE10, utilizando a interface VGA para saída gráfica e botões físicos para entrada.

### Funcionalidades:
- Interface gráfica baseada em bitmaps (sprites de 16x16 pixels)  
- Detecção de colisão com obstáculos (cactos e pássaros)  
- Simulação de pulo com velocidade e aceleração  
- Geração aleatória de nuvens e obstáculos ao longo do tempo  
- Máquina de estados: START, PLAY e GAME OVER  
- Exibição de pontuação em tempo real na tela

---

## Objetivos Didáticos

Estes projetos têm como finalidade:

- Demonstrar o controle direto de periféricos gráficos (GLCD e VGA)  
- Aplicar lógica de jogos em contextos com restrição de memória e processamento  
- Utilizar técnicas de geração de números pseudoaleatórios sem bibliotecas padrão  
- Trabalhar com interrupções, temporização e buffers gráficos  
- Estimular o desenvolvimento de sistemas interativos em plataformas embarcadas
