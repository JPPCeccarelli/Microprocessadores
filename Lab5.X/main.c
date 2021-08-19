/*
 * File:   main.c
 * Author: Vinicius Murasaki e João Ceccarelli 
 *
 * Created on June 5, 2020, 3:33 PM
 */


// PIC16F886 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = EC        // Oscillator Selection bits (EC: I/O function on RA6/OSC2/CLKOUT pin, CLKIN on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include "always.h"
#include "delay.h"
#include "io.h"
#include "lcd.h"
#include "debug.h"

//Matriz de Estados
__EEPROM_DATA(0,1,-1,0,-1,0,0,1);
__EEPROM_DATA(1,0,0,-1,0,-1,1,0);

// Variáveis Globais aqui se houverem
volatile int contador_somador = 0;  // Contador de rotacoes 
volatile int contador_antigo = 0;   // Armazena o valor antigo do contador
volatile int contador_tempo = 0;    // Contador de tempo no Timer0
volatile int velocidade = 0;        // Velocidade 
volatile unsigned int old_state;    // Estado antigo da maquina de estado
volatile unsigned int new_state;    // Novo estado da maquina de estado

// Função para tratamento de interrupções
void interrupt isr(void) {
  
  // Tratamento da interrupção do Timer 0
   if (T0IE && T0IF) {
        TMR0 = 60;                     // Reconfigura TMR0
        contador_tempo ++;             // Contador de tempo
        
        if (contador_tempo == 100){    // 100 interrupcoes de 10ms cada, totalizando 1s
            contador_tempo = 0;   
            velocidade = contador_somador - contador_antigo;   
            contador_antigo = contador_somador;
        }
        T0IF = 0;                      // Limpa flag
   }
  // Tratamento da interrupção do Port B
  if (RBIE && RBIF) {
    char portB = PORTB;               // Leitura do port B limpa interrupção
    io_sw_read(portB);                // Necessário para usar a chave
    
    new_state = 2*RB4 + RB3;
    if (EEPROM_READ(old_state*4+new_state) == 1){
        contador_somador++;                  // Incremento do contador
    } 
    if (EEPROM_READ(old_state*4+new_state) == 255){
        contador_somador--;                  // Decremento do contador
    } 
    old_state = new_state;  // O estado antigo eh atualizado para o novo estado
    RBIF = 0; // Limpa o flag de interrupção para poder atender nova
  } 
}

void t0_init(void) {
  // Inicialização do Timer 0
    T0CS = 0;                  //Utiliza Fosc/4
    PSA = 0;                   //Pre scaler associado ao Timer 0
    PS0 = 1;
    PS1 = 1;
    PS2 = 1;                   //Prescaler 1:256, sendo interrupts a cada 10ms
    TMR0 = 60;                 //valor inicial do Timer 0
    T0IE = 1;                  //habilita interrupcoes
}

void ioc_init(void) {
  // Inicialização do Port B aqui
    TRISB3 = 1;  //Entrada em RB3
    TRISB4 = 1;  //Entrada em RB4
    ANS9 = 0;    //RB3 como digital
    ANS11 = 0;   //RB4 como digital
    nRBPU = 1;   //desabilita weak pull-ups
    IOCB3 = 1;   //habilita I-o-C RB3
    IOCB4 =1;    //habilita I-o-C RB4       
}

// Programa Principal
void main(void) {
  
  //Variáveis locais
  char message[18];     // "string" para mensagens
  
  // Inicializações
  t0_init();            // Inicializa Timer0
  ioc_init();           // Inicializa Interrupt on Change do PortB
  io_init();            // Inicializa chave, LED e Buzzer
  lcd_init();           // Inicializa LCD
  debug_init();         // Inicializa LEDs para debug
  ei();                 // Macro do XC8, equivale a GIE = 1, habilita interrupções

  // Configurações iniciais
  io_beep(100);        // Dá um beep
  
  lcd_cursor(0);       // Desliga o cursor do LCD  
  
  // Loop principal (infinito))
  while(1) {
    
    lcd_goto(0);                                          // Vai para a primeira linha do lcd
    sprintf(message, "T5G7-JV %d ", contador_somador);    // Carregamento um string formatado
    lcd_puts(message);                                    // Colocando string no display LCD
          
    lcd_goto(64);                                         // Vai para a segunda linha do lcd
    sprintf(message, "Vel %d", velocidade);               // Carregamento um string formatado
    lcd_puts(message);                                    // Colocando string no display LCD
  }
}

