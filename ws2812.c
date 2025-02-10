#include <stdio.h>
#include <stdlib.h>
#include <pico/stdlib.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"

// Varáveis do Display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDR 0x3C
ssd1306_t ssd;

#define LEDS 25     // Quantidade de LEDs
#define PINO_LEDS 7 // Matriz de LEDs 5x5
#define LED_G 11    // LED verde
#define LED_B 12    // LED azul
#define BOTAO_A 5   // Botão A
#define BOTAO_B 6   // Botão B

// Variáveis que definem a cor da Matriz
double r = 1.0, g = 0.0, b = 1.0;

// Protótipo da função
static void gpio_irq_handler(uint gpio, uint32_t events);

//Sm e outras variaveis definidas como estáticas para não estourar o limite de Sm por PIO
static PIO pio = pio0;
static uint sm;
static uint offset;

// Váriavel que armazena o tempo do último evento
static volatile uint32_t ultimo_tempo = 0; 

// Matrizes dos números de 0 a 9
double numeroZero[25] = {   
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0
};
double numeroUm[25] = {     
    0.0, 0.0, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.8, 0.0
};
double numeroDois[25] = {   
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.0, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0
};
double numeroTres[25] = {   
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0
};
double numeroQuatro[25] = { 
    0.0, 0.8, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.8, 0.0
};
double numeroCinco[25] = {  
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.0, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0
};
double numeroSeis[25] = {   
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.0, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0
};
double numeroSete[25] = {   
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.8, 0.0
};
double numeroOito[25] = {   
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0
};
double numeroNove[25] = {   
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.8, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0,
    0.0, 0.8, 0.0, 0.0, 0.0,
    0.0, 0.8, 0.8, 0.8, 0.0
};

//Função para configurar o PIO
void configurar_pio() {
    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, PINO_LEDS);
}

// Função para mostrar o estado dos LEDs no dispaly
void atualizar_mensagens() {
    ssd1306_fill(&ssd, false); 
    char mensagem_G[16];
    char mensagem_B[16];

    if (gpio_get(LED_G)) {
        sprintf(mensagem_G, "Verde Ligado");
    } else {
        sprintf(mensagem_G, "Verde Desligado");
    }
    if (gpio_get(LED_B)) {
        sprintf(mensagem_B, "Azul Ligado");
    } else {
        sprintf(mensagem_B, "Azul Desligado");
    }

    ssd1306_draw_string(&ssd, mensagem_G, 20, 30);
    ssd1306_draw_string(&ssd, mensagem_B, 20, 30);
    ssd1306_send_data(&ssd); 
}

 //Função para definir a intensidade dos leds
 uint32_t definirLeds(double intVermelho, double intVerde, double intAzul){
     unsigned char vermelho, verde, azul;
 
     vermelho = intVermelho*255;
     verde = intVerde*255;
     azul = intAzul*255;
 
     return (verde << 24) | (vermelho << 16) | (azul << 8);
 }

//Função para fazer com que a matriz seja ativada, em uma cor especificada
void ligarMatriz(double r, double g, double b, double *desenho){
    uint32_t valorLed;
    for (int16_t i = 0; i < LEDS; i++) { 
        if(r != 0.0){
            valorLed = definirLeds(desenho[24-i], g, b);           
        }else if(g != 0.0){
            valorLed = definirLeds(r, desenho[24-i], b);    
        }else if(b != 0.0){
            valorLed = definirLeds(r, g, desenho[24-i]);           
        }
        pio_sm_put_blocking(pio, sm, valorLed);
    }
}

// Função principal
int main(){
    // Chama a função que configura o PIO
    configurar_pio();
 
    // Inicializações e definições (botões e leds)
    stdio_init_all();

    gpio_init(LED_G);
    gpio_set_dir(LED_G, 1);
    gpio_put(LED_G, 0);

    gpio_init(LED_B);
    gpio_set_dir(LED_B, 1);
    gpio_put(LED_B, 0);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, 0);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, 0);
    gpio_pull_up(BOTAO_B);     

    // Inicializações e definições (SSD1306)
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL); 

     // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);    

    // Configuração do display
    ssd1306_fill(&ssd, 0); 
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&ssd);    
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    

    // Mensagem inicial
    printf("RP2040 inicializado. Envie digitos para serem lidos no display.\n");

    // Loop Infinito
    while (true) {
        sleep_ms(1000);
        bool cor = true; 
        if (stdio_usb_connected()){
            char c;
            // Lê um caractere
            if (scanf("%c", &c) == 1){
                printf("Caractere recebido: '%c'\n", c);
                // Exibe os números na matriz caso sejam digitados
                if(c>=0 && c<=9){
                    if(c==0){
                        ligarMatriz(r, g, b, numeroZero);
                    }
                    if(c==1){
                        ligarMatriz(r, g, b, numeroUm);
                    }
                    if(c==2){
                        ligarMatriz(r, g, b, numeroDois);
                    }
                    if(c==3){
                        ligarMatriz(r, g, b, numeroTres);
                    }
                    if(c==4){
                        ligarMatriz(r, g, b, numeroQuatro);
                    }
                    if(c==5){
                        ligarMatriz(r, g, b, numeroCinco);
                    }
                    if(c==6){
                        ligarMatriz(r, g, b, numeroSeis);
                    }
                    if(c==7){
                        ligarMatriz(r, g, b, numeroSete);
                    }
                    if(c==8){
                        ligarMatriz(r, g, b, numeroOito);
                    }
                    if(c==9){
                        ligarMatriz(r, g, b, numeroNove);
                    }
                }
                // Altera o estado da variável
                cor = !cor;

                // Exibe o caractere no display
                ssd1306_fill(&ssd, !cor);              
                ssd1306_draw_char(&ssd, c, 50, 30); 
                ssd1306_send_data(&ssd);              
            }
            // Atraso para eviatar sobrecargas
            sleep_ms(40);           
        }
    }
}

// Função de interrupção com debouncing
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t tempo_real = to_us_since_boot(get_absolute_time()); // Obtém o tempo real
    // Debouncing     
    if (tempo_real - ultimo_tempo > 200000) { //Verifica se passou 200ms desde o último evento
        if (gpio == BOTAO_A) { 
            if (gpio_get(LED_B)==1) { // Botão A inverte o estado do LED azul
                gpio_put(LED_B, !gpio_get(LED_B)); 
                printf("LED azul desligado\n");
            }
        }
        if (gpio == BOTAO_B) {
            if (gpio_get(LED_G)==1) { 
                gpio_put(LED_G, !gpio_get(LED_G)); // Botão B inverte o estado do LED verde
                printf("LED verde desligado\n");
            } 
        }  
        // Mostra no display
        atualizar_mensagens();   
    }
    // Atualiza o tempo
    ultimo_tempo = tempo_real; 
}


