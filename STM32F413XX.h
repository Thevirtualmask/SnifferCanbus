#ifndef STM32F413XX_H //per evitare inclusione ricorsiva
#define STM32F413XX_H

#include <stdint.h>

#define RCC_CR         *(long*)0x40023800 //abilito osc
#define RCC_PLLCFGR    *(long*)0x40023804 //si varia il PLL (moltiplicatore/divisore freq)
#define RCC_CFGR       *(long*)0x40023808 //gestisce PLL (prescaler ecc)
#define RCC_AHB1ENR    *(long*)0x40023830 //enable clk porte 
#define RCC_APB1ENR    *(long*)0x40023840 //enable clk CAN1
#define RCC_APB2ENR    *(long*)0x40023844 //enable clk USART1

#define GPIOA_MODER    *(long*)0x40020000 
#define GPIOA_PUPDR    *(long*)0x4002000C  //gestisce resistenze di pu e pd (off)
#define GPIOA_AFRH     *(long*)0x40020024 //af high

#define GPIOB_MODER  *(volatile long*)0x40020400
#define GPIOB_PUPDR  *(volatile long*)0x4002040C
#define GPIOB_IDR    *(volatile long*)0x40020410 //stato logico del pin 
#define GPIOB_AFRH   *(volatile long*)0x40020424

#define USART1_SR      *(long*)0x40011000 
#define USART1_DR      *(long*)0x40011004 
#define USART1_BRR     *(long*)0x40011008 
#define USART1_CR1     *(long*)0x4001100C //controlla trasm, ric e periferica

#define FLASH_ACR      *(long*)0x40023C00 //per i wait states
#define PWR_CR         *(long*)0x40007000

#define CAN1_BASE    0x40006400 //boundary
#define CAN1_ESR     *(volatile long*)(CAN1_BASE + 0x18) //debug (ACK, esempio)
#define CAN1_MCR     *(volatile long*)0x40006400 //controllo, contiene init 
#define CAN1_MSR     *(volatile long*)0x40006404 //entrata o uscita da init
#define CAN1_TSR     *(volatile long*)0x40006408 //successo tx
#define CAN1_RF0R    *(volatile long*)0x4000640C //gestisce messaggi in coda 
#define CAN1_BTR     *(volatile long*)0x4000641C //durata bit, attivazione loopback, baudrate

//TX 
#define CAN1_TI0R    *(volatile long*)0x40006580 //manda mssg
#define CAN1_TDT0R   *(volatile long*)0x40006584 //definisce DLC
#define CAN1_TDL0R   *(volatile long*)0x40006588 //primi 4 byte
#define CAN1_TDH0R   *(volatile long*)0x4000658C //restanti byte 

//RX 
#define CAN1_RI0R    *(volatile long*)0x400065B0 //ID mssg arrivato 
#define CAN1_RDT0R   *(volatile long*)0x400065B4
#define CAN1_RDL0R   *(volatile long*)0x400065B8 //primi 4
#define CAN1_RDH0R   *(volatile long*)0x400065BC //ultimi 4

//filtri (acc filt)
#define CAN_FMR      *(volatile long*)0x40006600 //filt init
#define CAN_FM1R     *(volatile long*)0x40006604 //ident
#define CAN_FS1R     *(volatile long*)0x4000660C //scale (dim)
#define CAN_FA1R     *(volatile long*)0x4000661C //attivazione
#define CAN_F0R1     *(volatile long*)0x40006640 //lascia passare
#define CAN_F0R2     *(volatile long*)0x40006644 //maschera (decide cosa controllare)
	
//buffer per sprintf
static char buffer_invio[100];
#endif