#ifndef STM32F413XX_H //all'evenienza, non viene caricato due volte lo stesso documento
#define STM32F413XX_H
#include <stdint.h>

//clock enable
#define RCC_AHB1ENR   *(long*)0x40023830 
#define RCC_APB1ENR   *(long*)0x40023840
#define RCC_APB2ENR   *(long*)0x40023844 

//porta A
#define GPIOA_MODER   *(long*)0x40020000 
#define GPIOA_OTYPER  *(long*)0x40020004	
#define GPIOA_OSPEEDR *(long*)0x40020008	
#define GPIOA_PUPDR   *(long*)0x4002000C			
#define GPIOA_ODR     *(long*)0x40020014 
#define GPIOA_AFRL    *(long*)0x40020020 
#define GPIOA_AFRH    *(long*)0x40020024 

//boundary USART1: 0x40011000
#define USART1_SR     *(long*)0x40011000 
#define USART1_DR     *(long*)0x40011004 
#define USART1_BRR    *(long*)0x40011008 
#define USART1_CR1    *(long*)0x4001100C
#define USART1_CR2    *(long*)0x40011010
#define USART1_CR3    *(long*)0x40011014

//boundary: 0x40023800
#define RCC_CR        *(long*)0x40023800 
#define RCC_PLLCFGR   *(long*)0x40023804 
#define RCC_CFGR      *(long*)0x40023808 

// FLASH Interface Base: 0x40023C00
#define FLASH_ACR     *(long*)0x40023C00 

// FPU Control
#define CPACR         *(long*)0xE000ED88

#endif