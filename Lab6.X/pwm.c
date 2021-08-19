/*
 * File:   pwm.c
 * Author: João
 *
 * Created on 19 de Junho de 2020, 13:51
 */


#include <xc.h>

void pwm_init(void){
    //Inicializa RC2 e RC1 como entrada
    TRISC = 0b00000110;         
    
    //valor inicial de PR2, com base na tabela 11-4 do datasheet
    PR2 = 0xFF;
    
    //configura o módulo CCPxM para PWM
    CCP1CONbits.P1M = 0;
    CCP1CONbits.CCP1M = 1100;   
    CCP2CONbits.CCP2M = 1100;
    
    //configura os bits CCPRxL para largura do pulso adequada
    CCP1CONbits.DC1B = 0;
    CCPR1L = 128;
    DC2B1 = 0;          //por alguma razão o MPLAB não reconhece o comando
    DC2B0 = 0;          //CCP2CONbirs.DC2B (gera erro)
    CCPR2L = 128; 
    
    //configura os pinos PWM1_DIR e PWM2_DIR como saída
    TRISB1 = 0;
    TRISA1 = 0;
    RA1 = 0;
    RB1 = 0;
    
    //configura TMR2 e limpa sua flag
    TMR2IF = 0;
    T2CONbits.T2CKPS = 1;
    TMR2ON = 1;
    
    //habilita os pinos do PORT C como saída 
    while(!TMR2IF);             
    TRISC = 0;
    
}

int pwm_calc(int adc_value, int *pwm_dc1, int *pwm_dc2){
    
    //caso em que resultado da conversão é 512
    if(adc_value == 512){
        if(RA1 && RB1){
            *pwm_dc1 = -50;
            *pwm_dc2 = -50;
        }
        else{
            *pwm_dc1 = 50;
            *pwm_dc2 = 50;   
        }
        return 0;
    }
    
    //outros casos, usa-se ADC+1 para poder dividir 100/1024 = 25/256
    //sem gerar overflow
    else{
        if(RA1 && RB1){
            *pwm_dc1 = -(adc_value+1)*25/256;
            *pwm_dc2 = -(100 - (adc_value+1)*25/256);
        }
        else{
            *pwm_dc1 = (adc_value+1)*25/256;
            *pwm_dc2 = 100 - (adc_value+1)*25/256;
        }
        return 50*75/(*pwm_dc1-*pwm_dc2);
    }
}


void pwm_set(int pwm_dc1, int pwm_dc2){
    
    if (pwm_dc1 < 0 && pwm_dc2 < 0) {
        pwm_dc1 = -pwm_dc1;
        pwm_dc2 = -pwm_dc2;
    }
    
    //CCPR1L = 1024/400*pwm_dc1 = 64/25*pwm/dc1
    CCPR1L = 64*pwm_dc1/25;
    
    //DC1B são os 2 últimos bits não alocados em CCPR1L
    CCP1CONbits.DC1B = (256*pwm_dc1/25)%4;
            
    CCPR2L = 64*pwm_dc2/25;
    DC2B1 = (256*pwm_dc2/25)%4/2;
    DC2B0 = (256*pwm_dc2/25)%2;
}