/*
 * File:   adc.c
 * Author: João P P Ceccarelli e Vinicius K S Murasaki
 *
 * Created on 10 de Maio de 2020, 17:56
 */

#include <xc.h> // include processor files - each processor file is guarded.
#include "delay.h"

//configura inicializações da conversão AD
void adc_init_0(void){
    TRISA = 0b00000001;          //configura pino A0 como entrada e os outros como saída
    ANSEL = 0b00000001;          //configura pino A0 como analógico e os outros como digital
    ANSELH = 0;                  //configura como digital os outros pinos
    ADCS0 = 0;                   //configura clock de oscilação como Fosc/32
    ADCS1 = 1;
    VCFG0 = 0;                   //tensões de referencia Vss e Vdd
    VCFG1 = 0;
    CHS0 = 0;                    //escolhe canal 0 para conversão 
    CHS1 = 0;
    CHS2 = 0;
    CHS3 = 0;
    ADFM = 0;                    //justifica à esquerda, para variar um pouco
    ADON = 1;                    //liga canal de conversão
    delay_ms(10);                //tempo de aquisição
}

//função para realizar conversão
unsigned int adc_read_0(void){
    GO = 1;                                 //inicia conversão
    while(GO);                              //espera conversão acabar
    int result = (ADRESH<<2)+(ADRESL>>6);   //calcula o resultado da conversão justificada a esquerda
    return result;                          //retorna resultado
}