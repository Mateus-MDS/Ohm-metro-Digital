/**
 * OHMímetro com Raspberry Pi Pico
 * 
 * Este programa mede resistências usando um divisor de tensão e exibe
 * o valor e cores correspondentes em um display OLED e matriz de LEDs.
 */

/* ========== BIBLIOTECAS ========== */
#include <stdio.h>             // I/O padrão
#include <stdlib.h>            // Funções utilitárias
#include "pico/stdlib.h"       // Funções básicas Pico
#include "hardware/adc.h"      // Controle ADC
#include "hardware/i2c.h"      // Controle I2C
#include "lib/ssd1306.h"       // Controle display OLED
#include "lib/font.h"          // Fonte OLED
#include "pico/stdio_usb.h"    // Comunicação USB
#include "hardware/clocks.h"   // Controle de clocks
#include <string.h>            // Manipulação de strings
#include "animacoes_led.pio.h" // Animações LEDs PIO
#include "hardware/pio.h"      // Controle PIO
#include <math.h>              // Funções matemáticas
#include "pico/bootrom.h"      // Funções de boot

// Definições de hardware
#define I2C_PORT i2c1      // Porta I2C usada
#define I2C_SDA 14         // Pino SDA
#define I2C_SCL 15         // Pino SCL
#define endereco 0x3C      // Endereço OLED
#define ADC_PIN 28         // GPIO para o voltímetro

// Configuração da matriz de LEDs
#define NUM_PIXELS 25          // Número de LEDs na matriz
#define matriz_leds 7          // Pino de saída para matriz

// Variáveis globais
PIO pio;                      // Controlador PIO
uint sm;                      // State Machine do PIO
uint contagem = 0;            // Número a ser exibido na matriz

// Variáveis para medição de resistência
int R_conhecido = 10000;      // Resistor de 10k ohm
float R_x = 0.0;              // Resistor desconhecido (510? a 100k?)
float ADC_VREF = 3.31;        // Tensão de referência do ADC
int ADC_RESOLUTION = 4095;    // Resolução do ADC (12 bits)

// variáveis para uso das funções
float Resistor;               
float Resistor_proximo;
uint Multiplicador = 0;
int digito1;
int digito2;

// Buffers para strings
char Resistor_string[5];      // Buffer para armazenar a string
const char *cor_1, *cor_2, *cor_3;

// Tabelas de cores para resistores
const char* cores_digitos[] = {
    "Preto",     // 0
    "Marrom",    // 1
    "Vermelho",  // 2
    "Laranja",   // 3
    "Amarelo",   // 4
    "Verde",     // 5
    "Azul",      // 6
    "Violeta",   // 7
    "Cinza",     // 8
    "Branco"     // 9
};

const char* cores_multiplicador[] = {
    "Preto",     // x1
    "Marrom",    // x10
    "Vermelho",  // x100
    "Laranja",   // x1k
    "Amarelo",   // x10k
    "Verde",     // x100k
    "Azul",      // x1M
    "Violeta",   // x10M
    "Cinza",     // x100M (raro)
    "Branco"     // x1G (raro)
};

// Valores padrão da série E24
const float Resistores_E24[] = {
  510, 560, 620, 680, 750, 820, 910, 1000, 1100, 1200, 1300, 1500, 1600,1800, 2000, 2200, 2400, 2700, 3000, 
  3300, 3600, 3900, 4300, 4700, 5100, 5600, 6200, 6800, 7500, 8200, 9100, 10000, 11000, 12000, 13000, 15000, 
  16000, 18000, 20000, 22000, 24000, 27000, 30000, 33000, 36000, 39000, 43000, 47000,51000, 56000, 62000, 68000, 
  75000, 82000, 91000, 100000
};

// Configuração para modo BOOTSEL com botão B
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events) {
    reset_usb_boot(0, 0);
}

/* ========== FUNÇÕES DE MEDIÇÃO DE RESISTÊNCIA ========== */

// Encontra o valor mais próximo na série E24
void Aproximando_resistor() {
    float resistor_calculado = R_x;
    float erro_novo = 100000;
    bool cor = true;
    int tamanho = sizeof(Resistores_E24) / sizeof(Resistores_E24[0]);

    if ((resistor_calculado >= 500) && (resistor_calculado <= 100000)) {
        for (int k = 0; k < tamanho; k++) {
            float erro = fabs(resistor_calculado - Resistores_E24[k]);
            if (erro < erro_novo) {
                erro_novo = erro;
                Resistor_proximo = Resistores_E24[k];
            }
        }
    }
}

// Determina as cores do resistor com base no valor
void Determinando_cores() {
    Resistor = Resistor_proximo;
    Multiplicador = 0;
    bool cor = true;
  
    if ((Resistor >= 500) && (Resistor <= 100000)) {
        while(Resistor > 99) {
            Resistor = Resistor / 10;
            Multiplicador++;
        }

        sprintf(Resistor_string, "%1.0f", Resistor);   // Converte o float em string
        char d1 = Resistor_string[0];  // Primeiro dígito
        char d2 = Resistor_string[1];  // Segundo dígito
        
        digito1 = d1 - '0';
        digito2 = d2 - '0';

        cor_1 = cores_digitos[digito1];
        cor_2 = cores_digitos[digito2];
        cor_3 = cores_multiplicador[Multiplicador];
    }
}

/* ========== FUNÇÕES DE CONTROLE DE LEDS ========== */

// Converte nome da cor em valor RGB
uint32_t cor_resistor_para_rgb(const char* cor) {
    if (strcmp(cor, "Preto") == 0)    return 0x000000FF;
    if (strcmp(cor, "Marrom") == 0)   return 0x2AA52A00;
    if (strcmp(cor, "Vermelho") == 0) return 0x00FF0000;
    if (strcmp(cor, "Laranja") == 0)  return 0x7FFF0000;
    if (strcmp(cor, "Amarelo") == 0)  return 0xFFFF0000;
    if (strcmp(cor, "Verde") == 0)    return 0xFF000000;
    if (strcmp(cor, "Azul") == 0)     return 0x0000FF00;
    if (strcmp(cor, "Violeta") == 0)  return 0x307FFF00;
    if (strcmp(cor, "Cinza") == 0)    return 0x7F7F7F00;
    if (strcmp(cor, "Branco") == 0)   return 0xFFFFFF00;
    if (strcmp(cor, "Dourado") == 0)  return 0xFFD70000;
    if (strcmp(cor, "Prata") == 0)    return 0xC0C0C000;
    return 0x000000; // Preto por padrão
}

// Controla a matriz de LEDs para mostrar as cores do resistor
void Ligar_matriz_leds() {
    // Converte as cores em RGB
    uint32_t cor1_rgb = cor_resistor_para_rgb(cor_1);
    uint32_t cor2_rgb = cor_resistor_para_rgb(cor_2);
    uint32_t cor3_rgb = cor_resistor_para_rgb(cor_3);
    
    for (int i = 0; i < NUM_PIXELS; i++) {
        uint32_t valor_led = 0;
        int linha = i / 5; // 5x5 matriz, cada 5 elementos é uma linha
        
        if (linha == 4) {
            valor_led = cor1_rgb;
        } else if (linha == 2) {
            valor_led = cor2_rgb;
        } else if (linha == 0) {
            valor_led = cor3_rgb;
        } else {
            valor_led = 0x000000; // LED apagado (preto)
        }
        
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

/* ========== FUNÇÃO PRINCIPAL ========== */

int main() {
    // Inicializa hardware e periféricos
    stdio_init_all();
    stdio_usb_init();
    sleep_ms(2000);

    // Configuração do botão B para modo BOOTSEL
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    // Inicialização I2C para o display OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    // Configuração do display OLED
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    
    // Configuração do ADC
    adc_init();
    adc_gpio_init(ADC_PIN);
    
    // Inicialização PIO para matriz de LEDs
    pio = pio0;
    uint offset = pio_add_program(pio, &animacoes_led_program);
    sm = pio_claim_unused_sm(pio, true);
    animacoes_led_program_init(pio, sm, offset, matriz_leds);
    
    // Variáveis locais
    float tensao;
    char str_x[10], str_y[10];         // Buffers para valores numéricos
    char str_cor1[20], str_cor2[20], str_cor3[20]; // Buffers para cores
    bool cor = true;
    uint32_t last_print_time = 0;
    
    // Loop principal
    while (true) {
        // Leitura do ADC com média de 500 amostras
        adc_select_input(2);
        float soma = 0.0f;
        for (int i = 0; i < 500; i++) {
            soma += adc_read();
            sleep_ms(1);
        }
        float media = soma / 500.0f;
        
        // Cálculo da resistência desconhecida
        R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);
        
        // Verifica se o resistor está dentro da faixa válida
        if ((R_x < 500) || (R_x > 100000)) {
            // Mostra mensagem de erro no display
            ssd1306_fill(&ssd, !cor);
            ssd1306_draw_string(&ssd, "OHMIMETRO", 25, 10);
            ssd1306_draw_string(&ssd, "MEDI RESISTORES", 2, 20);
            ssd1306_draw_string(&ssd, "MAIORES QUE 500", 2, 30);
            ssd1306_draw_string(&ssd, "E MENORES QUE", 2, 40);
            ssd1306_draw_string(&ssd, "100000 OHMS", 2, 50);
            ssd1306_send_data(&ssd);
            
            // Apaga a matriz de LEDs
            for (int i = 0; i < NUM_PIXELS; i++) {
                pio_sm_put_blocking(pio, sm, 0x000000FF);
            }
        } else {
            // Processa a medição válida
            Aproximando_resistor();
            sprintf(str_x, "%1.0f", media);
            sprintf(str_y, "%1.0f", Resistor_proximo);
            Determinando_cores();
            
            // Prepara strings para exibição
            strcpy(str_cor1, cor_1);
            strcpy(str_cor2, cor_2);
            strcpy(str_cor3, cor_3);
            
            // Atualiza o display OLED
            ssd1306_fill(&ssd, !cor);
            ssd1306_rect(&ssd, 0, 0, 127, 63, cor, !cor);    // Moldura externa
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);    // Moldura interna
            ssd1306_line(&ssd, 3, 16, 123, 16, cor);          // Divisórias
            ssd1306_line(&ssd, 3, 30, 123, 30, cor);
            ssd1306_line(&ssd, 3, 46, 123, 46, cor);
            ssd1306_line(&ssd, 80, 47, 80, 60, cor);
            ssd1306_line(&ssd, 53, 3, 53, 46, cor);
            
            // Textos no display
            ssd1306_draw_string(&ssd, "COR 1", 8, 6);
            ssd1306_draw_string(&ssd, str_cor1, 60, 6);
            ssd1306_draw_string(&ssd, "COR 2", 8, 20);
            ssd1306_draw_string(&ssd, str_cor2, 60, 20);
            ssd1306_draw_string(&ssd, "COR 3", 8, 35);
            ssd1306_draw_string(&ssd, str_cor3, 60, 35);
            ssd1306_draw_string(&ssd, "RESISTOR", 8, 51);
            ssd1306_draw_string(&ssd, str_y, 85, 51);
            
            // Atualiza matriz de LEDs
            Ligar_matriz_leds();
            
            // Atualiza o display
            ssd1306_send_data(&ssd);
            sleep_ms(700);
        }
        
        // Log periódico via USB (1 segundo)
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_print_time >= 1000) {
            printf("Digito1: %d, Cor1: %s\n", digito1, cor_1);
            printf("Resistencia: %.2f\n", R_x);
            last_print_time = current_time;
        }
    }
}