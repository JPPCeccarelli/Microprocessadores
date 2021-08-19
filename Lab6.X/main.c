/*
 * File:   main.c
 * Author: João e Vinicius
 *
 * Created on 19 de Junho de 2020, 09:35
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
#include "pwm.h"

// Variáveis Globais aqui se houverem

//Variavel que carrega resultado da conversão AD
unsigned volatile int ADC;              

// Função para tratamento de interrupções
void interrupt isr(void) {
  
  // Tratamento da interrupção do Timer 0
    if (T0IE && T0IF) {
      ADC = adc_read_0();
      TMR0 = 157;
      T0IF = 0;
  }
    
    if (RBIE && RBIF){
    char portB = PORTB; // leitura do port B limpa interrupção
    
    io_sw_read(portB);      // Necessário para usar a chave
    if(io_sw_pressed()){    // Se a chave for pressionada inverte o estado do LED
        RA1 = ~RA1;
        RB1 = ~RB1;
        }
    
    RBIF = 0; // limpa o flag de interrupção para poder atender nova 
    }
}

// Inicialização do Timer 0
void t0_init(void) {
    T0CS = 0;                   //Utiliza Fosc/4
    PSA = 0;                    //Pre scaler associado ao Timer 0
    PS0 = 1;
    PS1 = 1;
    PS2 = 1;                    //Prescaler 1:256, sendo interrupts a cada 5ms
    TMR0 = 157;                  //valor inicial do Timer 0
    T0IE = 1;
}

void ioc_init(void) {
    //permite interrupção ao apertar a chave
    TRISB6 = 1;                 
    WPUB = 0;
    IOCB6 = 1;
    RBIE = 1;
}

// Programa Principal
void main(void) {
  
    //variáveis locais
    char message[9];     // "string" para mensagens
    int pwm_dc1 = 50;    // valor de PWM1 e PWM2 em porcentagem   
    int pwm_dc2 = 50;   
    int R;               // Raio que o robô percorre
    
    // Inicializações
    t0_init();           // inicializa Timer 0
    ioc_init();          //permite interrupções no PORTB
    io_init();           // inicializa chave, LED e Buzzer
    lcd_init();          // inicializa LCD
    adc_init_0();        // inicializa o módulo de conversão AD
    pwm_init();          // inicializa módulo PWM
    ei();                // macro do XC8, equivale a GIE = 1, habilita interrupções
  
    
    // Configurações iniciais
    lcd_cursor(0);        //desliga o cursor
    lcd_goto(0);       
    lcd_puts("T5G7 JV");  // mostra mensagem inicial
    delay_ms(2000);       // deixa a mensagem por 2 segundos e limpa tela
    lcd_clear();          
    
    // Loop principal (infinito)
    while(1) {
        //calcula o raio e aloca os novos valores percentuais em pwm_dc1 e pwm_dc2
        R = pwm_calc(ADC, &pwm_dc1, &pwm_dc2);
        
        //muda o valor do PWM1 e PWM2
        pwm_set(pwm_dc1, pwm_dc2);
        
        lcd_goto(0);
        
        //Caso o raio não seja infinito, aloca o valor numérico em 'message'
        if(R != 0){
            sprintf(message, "%5d mm", R);
        }
            
        //caso seja infinito, aloca infinito na string 'message'
        else{
            message[0] = ' ';
            message[1] = ' ';
            message[2] = ' ';
            message[3] = ' ';
            message[4] = 0xF3;
            message[5] = ' ';
            message[6] = 'm';
            message[7] = 'm';
            message[8] = 0;     
        }
        
        //escreve o valor do raio, em mm, na tela
        lcd_puts(message);
        
        lcd_goto(64);
        
        //escreve o valor de porcentagem de PWM na tela
        sprintf(message, "1:%4d", pwm_dc1);
        
        //separa valores
        message[6] = ' ';  
        message[7] = '%';
        lcd_puts(message);
        delay_ms(100);
        
        lcd_goto(64);
        
        //mostra PWM2 em porcentagem
        sprintf(message, "2:%4d", pwm_dc2);
        message[6] = ' ';  
        message[7] = '%';
        lcd_puts(message);
        delay_ms(100);
        }
   }
