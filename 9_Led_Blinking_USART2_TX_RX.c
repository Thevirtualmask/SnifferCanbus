#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "STM32F413xx.h"

long current_cpu_freq = 16;
char menu_buffer[100];

//prototipi
void init(void);
void scrivi_char(char c);
void scrivi_stringa(char* str);
void set_clock_80MHz(void);
void set_clock_16MHz(void);
void stampa_intestazione(void);
void gestisci_menu_frequenza(void);
void funzione_can_transmit_sim(void);
void funzione_can_receive_sim(void);

int main(void) {
    init(); 
    scrivi_stringa("\n\r** STM32F413 PRONTA **\n\r");

    while(1) {
        stampa_intestazione();
        while(!(USART1_SR & 0x00000020)); 
        char scelta = (char)USART1_DR;

        switch(scelta) {
            case '1': gestisci_menu_frequenza(); break;
            case '2': funzione_can_transmit_sim(); break;
            case '3': funzione_can_receive_sim(); break;
            default:  scrivi_stringa("\n\r[ERR] Comando errato.\n\r"); break;
        }
    }
}


void init(void) {
	
    RCC_AHB1ENR = (long)0x00000001; 
    RCC_APB1ENR = (long)0x10000000; 
    RCC_APB2ENR = (long)0x00000010; 

    //scale 1
    PWR_CR = (long)0x0000C000; 

    GPIOA_MODER = (long)0xA8280400;  

    GPIOA_PUPDR = (long)0x00140000; 

    GPIOA_AFRH  = (long)0x00000770;  

	//USART1: 9600 brr, 16 MHz
    USART1_BRR = (long)0x00000683;  
    USART1_CR1 = (long)0x0000200C; 
}

void scrivi_char(char c) {
    while(!(USART1_SR & 0x00000080)); 
    USART1_DR = c;
}

void scrivi_stringa(char* str) {
    while(*str) scrivi_char(*str++);
}

void set_clock_80MHz(void) {
    USART1_CR1 = (long)0x00000000; 
    FLASH_ACR = (long)0x00000704; 

    RCC_PLLCFGR = (long)0x02002810; 
    RCC_CR = (long)0x01000081; 
    while(!(RCC_CR & 0x02000000)); 

    RCC_CFGR = (long)0x00001002; 
    while ((RCC_CFGR & 0x0000000C) != 0x00000008);

    for(int i = 0; i < 200000; i++); 
			USART1_BRR = (long)0x0000208D; 
			USART1_CR1 = (long)0x0000200C;
}

void set_clock_16MHz(void) {
    USART1_CR1 = (long)0x00000000;
    RCC_CFGR = (long)0x00000000; 
    while ((RCC_CFGR & 0x0000000C) != 0x00000000);

    RCC_CR = (long)0x00000081; 
    FLASH_ACR = (long)0x00000000;

    USART1_BRR = (long)0x00000683; 
    USART1_CR1 = (long)0x0000200C;
}

void stampa_intestazione(void) {
    scrivi_stringa("\n\r--- F413 DASHBOARD ---\n\r");
    sprintf(menu_buffer, "Clock: %ld MHz\n\r", current_cpu_freq);
    scrivi_stringa(menu_buffer);
    scrivi_stringa("1. Clock | 2. CAN TX | 3. CAN RX\n\r> ");
}

void gestisci_menu_frequenza(void) {
    scrivi_stringa("\n\r(a) 16MHz (b) 80MHz > ");
    while(!(USART1_SR & 0x00000020));
    char sub = (char)USART1_DR;

    if(sub == 'a' && current_cpu_freq != 16) {
        set_clock_16MHz(); current_cpu_freq = 16;
        scrivi_stringa("\n\r[OK] 16 MHz.\n\r");
    } else if(sub == 'b' && current_cpu_freq != 80) {
        set_clock_80MHz(); current_cpu_freq = 80;
        scrivi_stringa("\n\r[OK] 80 MHz.\n\r");
    }
}

void funzione_can_transmit_sim(void) { scrivi_stringa("\n\r[CAN] TX Sim...\n\r"); }
void funzione_can_receive_sim(void) { scrivi_stringa("\n\r[CAN] RX Sim...\n\r"); }