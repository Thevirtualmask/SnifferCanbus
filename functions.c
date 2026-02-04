#include "STM32F413xx.h" 
#include "functions.h"
#include <stdio.h>

// Definizioni locali per gestione Power
#define PWR_CR        *(volatile uint32_t*)0x40007000
#define RCC_APB1ENR_PWREN (1 << 28) // Bit 28 per abilitare clock Power

char temp_string[50];

void init(void)

{
    // --- 1. CONFIGURAZIONE BASE (16 MHz HSI) ---

    // Abilitazione FPU

    //CPACR |= ((3 << 10*2)|(3 << 11*2)); SUPERFLUA

    // Abilitazione Clock Periferiche essenziali

    RCC_AHB1ENR |= 0x00000001; // GPIOA

    // Abilita PWR (Power) e USART2 (opzionale)

    RCC_APB1ENR |= 0x00020000 | RCC_APB1ENR_PWREN; //Questo statement abilita il clock per due moduli sul bus APB1
    RCC_APB2ENR |= 0x00008010; // USART1 (Bit 4)

    //configura il voltage regulator a Scale 1 (High Performance)

    PWR_CR |= (3 << 14); 

    // Configurazione GPIO PA9 (TX) e PA10 (RX)

    GPIOA_MODER &= ~0x003C0000U;  
    GPIOA_MODER |=  0x00280000U; // Alternate Function
    GPIOA_PUPDR &= ~0x003C0000U; 
    GPIOA_PUPDR |=  0x00140000U; // Pull-Up

    // Mappatura AF7 (USART1)

    GPIOA_AFRH  &= ~0x00000FF0; 
    GPIOA_AFRH  |=  0x00000770; 

    // Configurazione USART1 Iniziale (16 MHz)
		
    // Clock APB2 = 16 MHz (HSI)

    // Baud 9600 -> Divisore = 16M / 9600 = 1666.6 -> 0x0683

    USART1_BRR = 0x0683; 

    // Abilita TX, RX, UE

    USART1_CR1 = 0x200C; 
}

void scrivi_char(char c)

{
    while(!(USART1_SR & 0x00000080)); 
    USART1_DR = c;
}


void scrivi_stringa(char* str)

{

    while(*str)

    {
        scrivi_char(*str++);
    }
}



// --- GESTIONE CAMBIO FREQUENZA ---

// Configurazione per 80 MHz (Nuova modalità Turbo stabile)

void set_clock_100MHz(void) // Mantengo il nome per compatibilità col main, ma va a 80MHz
{

    // 1. Spegni USART per sicurezza durante il cambio

    USART1_CR1 = 0; 

    // 2. Configura Flash per 80 MHz (4 Wait States sono molto sicuri, ne basterebbero 2 o 3)

    FLASH_ACR = 0x00000704; // Latency 4 + Cache + Prefetch

    // 3. Configura PLL (16MHz HSI -> 80MHz SysClk)

    // M=16, N=160, P=2, Q=2

    // RCC_PLLCFGR = 0x02002810 (Valore calcolato per N=160, P=2, M=16)

    // Bit 22 (SRC)=0 (HSI), Bit 17:16 (P)=00 (div 2), Bit 14:6 (N)=160, Bit 5:0 (M)=16

    // 160 = 0xA0. Shiftato in pos 6 -> 0x2800. M=16 -> 0x10.

    RCC_PLLCFGR = 0x02002810; 

    // 4. Accendi PLL

    RCC_CR |= 0x01000000; 

    while(!(RCC_CR & 0x02000000)); // Attendi PLL Ready

    // 5. Configura Bus Prescalers (AHB=1, APB1=2, APB2=1)

    // APB1 andrà a 40 MHz (ok < 50), APB2 a 80 MHz (ok < 100)

    RCC_CFGR |= 0x00001000; 

    // 6. Switch System Clock al PLL

    RCC_CFGR &= ~0x00000003; 
    RCC_CFGR |= 0x00000002;  

    while ((RCC_CFGR & 0x0000000C) != 0x00000008); // Attendi switch

    // Ritardo di stabilizzazione

    for(volatile int i = 0; i < 200000; i++);

    // 7. Ricalcola Baudrate per 80 MHz

    // Clock = 80 MHz, Target = 9600

    // Divisore = 80.000.000 / (16 * 9600) = 520.8333

    // Mantissa = 520 (0x208)

    // Frazione = 0.8333 * 16 = 13.33 -> 13 (0xD)

    // Valore BRR = 0x208D

    USART1_BRR = 0x208D; 

    // 8. Riaccendi USART

    USART1_CR1 = 0x200C; 
}

// Configurazione per 16 MHz (Low Power)

void set_clock_16MHz(void)
{

    // 1. Spegni USART

    USART1_CR1 = 0;

    // 2. Torna a HSI (16 MHz) come clock di sistema

    RCC_CFGR &= ~0x00000003; 

    while ((RCC_CFGR & 0x0000000C) != 0x00000000); // Attendi switch HSI  

    // 3. Spegni PLL

    RCC_CR &= ~0x01000000; 

    // 4. Ripristina Flash a 0 Wait States (HSI è lento)

    FLASH_ACR = 0x00000000;     

    // 5. Reset Prescaler Bus (tutti a 1)

    RCC_CFGR &= ~0x00001C00; 


    // 6. Ricalcola Baudrate per 16 MHz

    // Clock = 16 MHz, Target = 9600 -> 0x0683

    USART1_BRR = 0x0683;  

    // 7. Riaccendi USART

    USART1_CR1 = 0x200C;
}

// Stub per compatibilità, non usata direttamente

void update_USART1_baudrate(uint32_t baud_div) { }

void funzione_can_transmit_sim(void) {

    scrivi_stringa("[F413] Simulazione TX CAN...\n\r");

}

void funzione_can_receive_sim(void) {

    scrivi_stringa("[F413] In ascolto CAN...\n\r");

}