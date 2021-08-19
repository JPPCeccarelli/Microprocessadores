/*
 * File:   main.c
 * Author: João.
 *
 * Created on June 26, 2020, 19:11 PM
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
#include "adc.h"
#include "debug.h"

// Variáveis Globais aqui se houverem

volatile unsigned int ADC;

// Função para tratamento de interrupções
void interrupt isr(void) {
  
  // Tratamento da interrupção do Timer 0
  if (T0IE && T0IF) {   
    ADC = adc_read_0();
    TMR0 = 157;             //reconfigura TMR0
    T0IF = 0;               //limpa flag
  }
  
  // Tratamento de outras interrupções aqui
}

// Inicialização do Timer 0
void t0_init(void) {
    T0CS = 0;                   //Utiliza Fosc/4
    PSA = 0;                    //Pre scaler associado ao Timer 0
    PS0 = 1;
    PS1 = 1;
    PS2 = 1;                    //Prescaler 1:256, sendo interrupts a cada 5ms
    TMR0 = 157;                 //valor inicial do Timer 0
    T0IE = 1;                   //Enabler Timer 0
}

void port_init(void) {
    
    TRISC = 0b00001111;         //Configura ports conectadas ao MOSFET como saída
    PORTC = 0b00010000;         //Inicializa apenas RB4 (ultimo digito) como 1
    TRISB = 0b00000001;         //Configura ports conectados ao LCD como saida
}

void eeprom_wr_init(void){
    //Escreve na EEPROM os valores tais quais o datasheet do display
    eeprom_write(0, 0b11111100);
    eeprom_write(1, 0b01100000);
    eeprom_write(2, 0b11011010);
    eeprom_write(3, 0b11110010);
    eeprom_write(4, 0b01100110);
    eeprom_write(5, 0b10110110);
    eeprom_write(6, 0b10111110);
    eeprom_write(7, 0b11100000);
    eeprom_write(8, 0b11111110);
    eeprom_write(9, 0b11110110);
}

// Programa Principal
void main(void) {
  
  // Inicializações
  t0_init();           // inicializa Timer 0
  io_init();           // inicializa chave, LED e Buzzer
  port_init();         // inicializa portas
  adc_init_0();        // inicializa ADC
  eeprom_wr_init();    // escreve os valores na EEPROM
  ei();                // macro do XC8, equivale a GIE = 1, habilita interrupções

  // Configurações iniciais
  
  // Loop principal (infinito))
  while(1) {
      if(RC4 == 1){
          //Se a porta RC4 estiver ativa, faz leitura apenas para ultimo digito
          //do resultado da conversao
          PORTB = eeprom_read(ADC%10)&0b11111110;
          //delay de 5ms para atualizar o hardware no SimulIDE
          delay_ms(5);
          //desativa RC4 e vai para o proximo bit
          RC4 = 0;
          RC5 = 1;
      }
      
      if(RC5 == 1){
          //análogo porém para o segundo menor digito do resultado da conversao
          PORTB = eeprom_read((ADC/10)%10)&0b11111110;
          delay_ms(5);
          RC5 = 0;
          RC6 = 1;
      }
      
      if(RC6 == 1){
          //análogo porém para o segundo maior digito do resultado da conversao
          PORTB = eeprom_read((ADC/100)%10)&0b11111110;
          delay_ms(5);
          RC6 = 0;
          RC7 = 1;
      }
      
      if(RC7 == 1){
          //análogo porém para o maior digito do resultado da conversao
          PORTB = eeprom_read(ADC/1000)&0b11111110;
          delay_ms(5);
          RC7 = 0;
          RC4 = 1;
      }
      
      //obs: ciclo total dura 20ms -> OK
  }
}
