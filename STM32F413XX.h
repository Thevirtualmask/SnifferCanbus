#ifndef STM32F413XX_H
#define STM32F413XX_H

#include <stdint.h>

#define RCC_CR         *(long*)0x40023800 
#define RCC_PLLCFGR    *(long*)0x40023804 
#define RCC_CFGR       *(long*)0x40023808 
#define RCC_AHB1ENR    *(long*)0x40023830 
#define RCC_APB1ENR    *(long*)0x40023840 
#define RCC_APB2ENR    *(long*)0x40023844 

#define GPIOA_MODER    *(long*)0x40020000 
#define GPIOA_PUPDR    *(long*)0x4002000C 
#define GPIOA_AFRH     *(long*)0x40020024 

#define GPIOB_MODER  *(volatile long*)0x40020400
#define GPIOB_PUPDR  *(volatile long*)0x4002040C
#define GPIOB_IDR    *(volatile long*)0x40020410
#define GPIOB_AFRH   *(volatile long*)0x40020424

#define USART1_SR      *(long*)0x40011000 
#define USART1_DR      *(long*)0x40011004 
#define USART1_BRR     *(long*)0x40011008 
#define USART1_CR1     *(long*)0x4001100C 

#define FLASH_ACR      *(long*)0x40023C00 
#define PWR_CR         *(long*)0x40007000
#define CPACR          *(long*)0xE000ED88

#define CAN1_MCR     *(volatile long*)0x40006400
#define CAN1_MSR     *(volatile long*)0x40006404
#define CAN1_TSR     *(volatile long*)0x40006408
#define CAN1_RF0R    *(volatile long*)0x4000640C
#define CAN1_BTR     *(volatile long*)0x4000641C

// CAN1 TX Mailbox 0
#define CAN1_TI0R    *(volatile long*)0x40006580
#define CAN1_TDT0R   *(volatile long*)0x40006584
#define CAN1_TDL0R   *(volatile long*)0x40006588
#define CAN1_TDH0R   *(volatile long*)0x4000658C

// CAN1 RX FIFO 0
#define CAN1_RI0R    *(volatile long*)0x400065B0
#define CAN1_RDT0R   *(volatile long*)0x400065B4
#define CAN1_RDL0R   *(volatile long*)0x400065B8
#define CAN1_RDH0R   *(volatile long*)0x400065BC

// CAN Filtri
#define CAN_FMR      *(volatile long*)0x40006600
#define CAN_FM1R     *(volatile long*)0x40006604
#define CAN_FS1R     *(volatile long*)0x4000660C
#define CAN_FA1R     *(volatile long*)0x4000661C
#define CAN_F0R1     *(volatile long*)0x40006640
#define CAN_F0R2     *(volatile long*)0x40006644
	
// Variabile buffer per sprintf
static char buffer_invio[100];
#endif