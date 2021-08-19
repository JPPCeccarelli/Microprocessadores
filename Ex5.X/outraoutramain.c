/*
 * File:   main.c
 * Author: João
 *
 * Created on 29 de Junho de 2020, 17:47
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

// Variáveis Globais aqui se houverem
volatile unsigned int rch;              //recebe o valor do keypad
volatile unsigned int antigo;           //variavel de debounce

// Função para tratamento de interrupções
void interrupt isr(void) {
  
  // Tratamento da interrupção do Timer 0
  if (T0IE && T0IF) {
      //leitura do keypad
      rch = key_read();
      //debounce
      if(rch != antigo)
          antigo = rch;
      else
          rch = '\0';
      TMR0 = 60;
      T0IF = 0;
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
    TMR0 = 60;                  //valor inicial do Timer 0
    T0IE = 1;                   //Enabler Timer 0
}

void eeprom_wr_init(void){
    //Escreve na EEPROM os valores tais quais o datasheet do display
    eeprom_write(0, 0b01111110);
    eeprom_write(1, 0b00110000);
    eeprom_write(2, 0b01101101);
    eeprom_write(3, 0b01111001);
    eeprom_write(4, 0b00110011);
    eeprom_write(5, 0b01011011);
    eeprom_write(6, 0b01011111);
    eeprom_write(7, 0b01110000);
    eeprom_write(8, 0b01111111);
    eeprom_write(9, 0b01110011);
    
    //escreve na EEPROM os valores de acordo com a matriz do keypad
    eeprom_write(0x3e, '1');
    eeprom_write(0x5e, '2');
    eeprom_write(0x6e, '3');
    eeprom_write(0x3d, '4');
    eeprom_write(0x5d, '5');
    eeprom_write(0x6d, '6');
    eeprom_write(0x3b, '7');
    eeprom_write(0x5b, '8');
    eeprom_write(0x6b, '9');
    eeprom_write(0x37, '*');
    eeprom_write(0x57, '0');
    eeprom_write(0x67, '#');
}

void port_init (void) {
    TRISA = 0b00010111;         //Configura ports conectadas ao MOSFET como saída
    PORTA = 0b10000000;         //Inicializa apenas RA7
    TRISB = 0b10000000;         //Configura ports conectados ao LCD como saida
    PORTB = 0;                  //configura PORTB como 0 (não importa)
}

int key_read(void){
    //algoritmo apresentado em aula
    TRISC = 0b11110000;
    int temp1 = PORTC;
    TRISC = 0b10001111;
    return eeprom_read(temp1|PORTC);
}

// Programa Principal
void main(void) {
  
  //variáveis locais
  
  // Inicializações
  t0_init();           // inicializa Timer 0
  io_init();           // inicializa chave, LED e Buzzer
  eeprom_wr_init();    //configura eeprom
  port_init();         // inicializa portas
  ei();                // macro do XC8, equivale a GIE = 1, habilita interrupções

  // Configurações iniciais

  // Loop principal (infinito))
  
  while(1) {
      if(rch == '*'){
          //reseta PORTA, PORTB, valor maximo do contador e string de armazenamento de valores
          PORTA = 0b10000000;
          PORTB = 0b00001000;
      }
      else if(rch == '#'){
          //aumenta valor máximo do contador
          if(PORTA == 0b10000000)
              PORTA = 0b01000000;
          else if(PORTA == 0b01000000)
              PORTA = 0b00100000;
          else if(PORTA == 0b00100000)
              PORTA = 0b00001000;
          else
              PORTA = 0b10000000;
      }
      else {
          if(rch != '\0')
            PORTB = eeprom_read(rch - 0x30);
        }
      delay_ms(5);
  }
}
