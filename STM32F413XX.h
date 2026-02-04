#ifndef STM32F413XX_H
#define STM32F413XX_H

#include <stdint.h>

/* ============================================================================== */
/* PERIFERICHE BASE (STM32F413/423)                                               */
/* Aggiornato per USART1 su APB2                                                  */
/* ============================================================================== */

// Definizione registri RCC (Clock Enable)
#define RCC_AHB1ENR   *(volatile uint32_t*)0x40023830 
#define RCC_APB1ENR   *(volatile uint32_t*)0x40023840 
// [NUOVO] Necessario per USART1 (che sta su APB2)
#define RCC_APB2ENR   *(volatile uint32_t*)0x40023844 

// Definizione dei registri della GPIOA
#define GPIOA_MODER   *(volatile uint32_t*)0x40020000 
#define GPIOA_OTYPER  *(volatile uint32_t*)0x40020004	
#define GPIOA_OSPEEDR *(volatile uint32_t*)0x40020008	
#define GPIOA_PUPDR   *(volatile uint32_t*)0x4002000C			
#define GPIOA_ODR     *(volatile uint32_t*)0x40020014 
#define GPIOA_AFRL    *(volatile uint32_t*)0x40020020 
// [NUOVO] Necessario per configurare PA9 e PA10 (pin > 7)
#define GPIOA_AFRH    *(volatile uint32_t*)0x40020024 
	
// Registri del Timer TIM2
#define TIM2_CR1      *(volatile uint32_t*)0x40000000
#define TIM2_CR2      *(volatile uint32_t*)0x40000004
#define TIM2_SMCR     *(volatile uint32_t*)0x40000008	
#define TIM2_DIER     *(volatile uint32_t*)0x4000000C
#define TIM2_SR       *(volatile uint32_t*)0x40000010
#define TIM2_EGR      *(volatile uint32_t*)0x40000014
#define TIM2_CCMR1    *(volatile uint32_t*)0x40000018
#define TIM2_CCMR2    *(volatile uint32_t*)0x4000001C
#define TIM2_CCER     *(volatile uint32_t*)0x40000020
#define TIM2_CNT      *(volatile uint32_t*)0x40000024
#define TIM2_PSC      *(volatile uint32_t*)0x40000028
#define TIM2_ARR      *(volatile uint32_t*)0x4000002C
#define TIM2_CCR1     *(volatile uint32_t*)0x40000034
#define TIM2_CCR2     *(volatile uint32_t*)0x40000038
#define TIM2_CCR3     *(volatile uint32_t*)0x4000003C
#define TIM2_CCR4     *(volatile uint32_t*)0x40000040

// Registri USART1 (Sostituiscono USART2)
// Base Address USART1: 0x4001 1000 (Bus APB2)
#define USART1_SR     *(volatile uint32_t*)0x40011000 
#define USART1_DR     *(volatile uint32_t*)0x40011004 
#define USART1_BRR    *(volatile uint32_t*)0x40011008 
#define USART1_CR1    *(volatile uint32_t*)0x4001100C
#define USART1_CR2    *(volatile uint32_t*)0x40011010
#define USART1_CR3    *(volatile uint32_t*)0x40011014

/* ============================================================================== */
/* REGISTRI DI SISTEMA (RCC, FLASH, FPU)                                          */
/* ============================================================================== */

// RCC Base: 0x40023800
#define RCC_CR        *(volatile uint32_t*)0x40023800 
#define RCC_PLLCFGR   *(volatile uint32_t*)0x40023804 
#define RCC_CFGR      *(volatile uint32_t*)0x40023808 

// FLASH Interface Base: 0x40023C00
#define FLASH_ACR     *(volatile uint32_t*)0x40023C00 

// FPU Control
#define CPACR         *(volatile uint32_t*)0xE000ED88

#endif