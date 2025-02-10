# Tarefa U4C6 - Interfaces de Comunicação Serial com RP2040 - UART, SPI e I2C

### Vídeo de demonstração

https://youtu.be/Gf8aVeyHDuA?si=6pa8-_a7YIFAgIt5

### Descrição

Este projeto tem como objetivo consolidar os conceitos de comunicação serial no microcontrolador RP2040, explorando as funcionalidades da placa de desenvolvimento BitDogLab. A implementação envolve o uso de UART e I2C, manipulação de LEDs comuns e endereçáveis, bem como o controle de botões com interrupções e debounce. Produzido para a capacitação EmbarcaTech.

### Funcionalidades Implementadas

1. Modificação da Biblioteca font.h para adição de caracteres minúsculos no display SSD1306.

2. Entrada de Caracteres via PC digitados pelo Serial Monitor do VS Code.

3. Exibição dos caracteres no display SSD1306.

4. Exibição de números de 0 a 9 na matriz 5x5 WS2812.

5. Interação com o Botão A para alternar o estado do LED RGB Verde (ligado/desligado).
   
6. Interação com o Botão B para alternar o estado do LED RGB Azul (ligado/desligado).

7. Exibição de mensagens informativas no Serial Monitor e no Display SSD1306.

### Instruções
1. Clone este repositório:
   ```sh
   git clone https://github.com/Mescxll/Tarefa_Interfaces.git
   ```
2. Compile e carregue o código no Raspberry Pi Pico W utilizando o VS Code e o Pico SDK.
3. Utilize a placa BitDogLab para testar o funcionamento do código.
4. Verifique a saída da porta serial para confirmação das mensagens.

