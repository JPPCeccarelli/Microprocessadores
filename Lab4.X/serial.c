/*
 * File:   serial.c
 * Author: João e Vinicius
 *
 * Created on 20 de Maio de 2020, 17:33
 */

#ifndef SERIAL_C
#define SERIAL_C

#include <xc.h>
#include <conio.h>
#include <stdio.h>
#include "always.h"
#include "delay.h"

void serial_init(void) {
  // escreva a função de inicialização aqui
    BRG16 = 0;                          //Fosc/(16(n+1)) sem usar SPBRGH
    BRGH = 1;                           //High speed
    SPBRG = 64;                         //Seta [SPBRG] = 64
    SPBRGH = 0;                             
    SYNC = 0;                           //Modo assíncrono
    SPEN = 1;                           //Necessário para realizar transmissão e recepção
    CREN = 1;                           //Necessário para recepção
    TXEN = 1;                           //Necessário para transmissão
}

unsigned char chkchr (void) {
    if(RCREG = 0){                      //Caso não receba nada retorna 255
        return 255;                     //e assim getch() permanece no loop
    }
    else{
        return RCREG;                   //Caso contrário recebe o caracter em RCREG
    }
}

// Retira um caractere do buffer de Rx
// não sai enquanto no chegar caractere no buffer de Rx.
unsigned char getch(void) {
  while(!RCIF); 
  unsigned char c = 255;
  while (c == 255) c = chkchr();
  return c;
}

// Escreve um caractere no canal serial
void putch(unsigned char c) {
  // escreva sua função aqui
    while(!TXIF);                               //Espera flag de transimissão ser limpa para passar o caracter
    TXREG = c;                                  //Caracter é transmitido
}

// Envia um string pelo canal serial
// se o string contiver <CR>, adiciona <LF>
// se o string contiver <LF>, adiciona <CR>
void putst(register const char *str) {
  while((*str)!=0) {
    putch(*str);
    if (*str==13) putch(10);
    if (*str==10) putch(13);
    str++;
  }
}

#endif
