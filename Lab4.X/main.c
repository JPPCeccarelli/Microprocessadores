/*
 * File:   main.c
 * Author: Jo�o e Vinicius
 *
 * Created on 20 de Maio de 2020, 17:33
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
#include "serial.h"
#include "debug.h"

// Vari�veis Globais aqui se houverem

// Fun��o para tratamento de interrup��es
void interrupt isr(void) {
    
  // Tratamento da interrup��o do Port B
  if (RBIE && RBIF) {
    
    char portB = PORTB; // leitura do port B limpa interrup��o
    
    io_sw_read(portB);      // Necess�rio para usar a chave
    if(io_sw_pressed()){    // Se a chave for pressionada inverte o estado do LED
        io_led_toggle();
    }
    
    RBIF = 0; // limpa o flag de interrup��o para poder atender nova
  } 
  
  // Tratamento de outras interrup��es aqui
}

// Programa Principal
void main(void) {
  
  //vari�veis locais
  int i;               //vari�vel auxiliar para contagem de loop
  int lcd_show;        //vari�vel auxiliar para mostragem de valor no LCD
  char rch[9];         // string para recep��o de caracteres
  char message[9];     // "string" para mensagens
  
  // Inicializa��es
  GIE = 1;             // Permite interrup��es globais
  RBIE = 1;            // Permite interrup��es com port B
  io_init();           // inicializa chave, LED e Buzzer
  lcd_init();          // inicializa LCD
  debug_init();        // inicializa LEDs para debug
  serial_init();       // inicializa serial

  // Configura��es iniciais
  
  lcd_cursor(0);       // desliga o cursor do LCD  
  lcd_puts("T5G7 JV"); // mostra mensagem inicial
  
  // Loop principal (infinito))
  while(1) {
      for(i = 0; ; i++){                                            //Loop que passa os caracteres recebidos para rch
          rch[i] = getch();                 
          if(rch[i] < 48 || rch[i] > 57)                            //quebra o loop ao receber um caracter n�o num�rico
              break;                      
        }   
    
    if(i == 3)                                                      //Quebra e soma os d�gitos de acordo com o n�mero de
        lcd_show = (rch[0]-48)*100 + (rch[1]-48)*10 + rch[2]-48;    //caracteres recebidos
    if(i == 2)
        lcd_show = (rch[0]-48)*10 + rch[1]-48;
    if(i == 1)
        lcd_show = rch[0]-48;
    
    rch[i] = 13;                        //Substitui a letra de rch pelo caracter ASCII 13
    rch[i+1] = 0;                       //Adiciona o caracter null ao final da string
    
    putst(&rch);                        //Transmiss�o de rch
    
    for(i = 0; i <= 7; i++){            //Loop para convers�o dos caracteres num�ricos em bin�rio
        if(lcd_show%2 == 1)             //Se o resultado da divis�o for 1 a posi��o 7-i no LCD vira 1
            message[7-i] = '1';
        else
            message[7-i] = '0';         //Caso contr�rio � 0
        lcd_show /= 2;                  //Divide o n�mero inteiro por 2
    }
    
    lcd_goto(64);                       //vai para casa 64 do LCD (linha de baixo)
    lcd_puts(message);                  //coloca a mensagem
  }
}