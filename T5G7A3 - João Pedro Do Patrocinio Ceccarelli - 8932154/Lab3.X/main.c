/*
 * File:   main.c
 * Author: Jo�o P P Ceccarelli e Vinicius K S Murasaki
 *
 * Created on 10 de Maio de 2020, 17:56
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
#include "lcd.h"
#include "adc.h"

// Vari�veis Globais aqui se houverem

unsigned volatile int ADC = 0;  //variavel que recebe valor da convers�o

// Fun��o para tratamento de interrup��es
void interrupt isr(void) {
  
  // Tratamento da interrup��o do Timer 0
    if (T0IE && T0IF) {
        ADC = adc_read_0();     //inicia conversao e devolve resultado para ADC
        TMR0 = 157;             //reconfigura TMR0
        T0IF = 0;               //limpa flag
  }
}

// Inicializa��o do Timer 0
void t0_init(void) {
    T0CS = 0;                   //Utiliza Fosc/4
    PSA = 0;                    //Pre scaler associado ao Timer 0
    PS0 = 1;
    PS1 = 1;
    PS2 = 1;                    //Prescaler 1:256, sendo interrupts a cada 5ms
    TMR0 = 157;                  //valor inicial do Timer 0
    T0IE = 1;
}

// Programa Principal
void main(void) {
  
  //vari�veis locais
  char message[4];     // "string" para mensagens
  
  // Inicializa��es
  t0_init();           // inicializa Timer 0
  lcd_init();          // inicializa LCD
  adc_init_0();        // inicializa ADC
  delay_ms(10);        // espera tempo de aquisi��o
  GIE = 1;             // macro do XC8, equivale a GIE = 1, habilita interrup��es

  // Configura��es iniciais
  lcd_cursor(0);       // desliga cursor
  lcd_goto(0);         // coloca LCD na posi��o inicial
  lcd_puts("T5G7-JV"); // mostra mensagem inicial
  
  // Loop principal (infinito)
  while(1) {
    
    if(ADC > 850){
        sprintf(message, "%03d", 0);                    //se o valor da convers�o for maior que 850 a dist�ncia � 0
    }
    else{
        sprintf(message, "%03d", (300 - (ADC*6)/17));   //coloca dist�ncia em mm no vetor distancia, o numerador
                                                        //e o denominador devem ser baixos para n�o dar overflow (> 16 bits)
    }   
    lcd_goto(64);                                       //vai para linha de baixo
    lcd_puts(message);                                  //escreve mensagem na tela
    delay_ms(200);                                      //delay de 200ms
  }
}