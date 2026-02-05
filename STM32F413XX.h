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

#define USART1_SR      *(long*)0x40011000 
#define USART1_DR      *(long*)0x40011004 
#define USART1_BRR     *(long*)0x40011008 
#define USART1_CR1     *(long*)0x4001100C 

#define FLASH_ACR      *(long*)0x40023C00 
#define PWR_CR         *(long*)0x40007000
#define CPACR          *(long*)0xE000ED88

#endif