#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "STM32F413xx.h"

// --- VARIABILI GLOBALI ---
long current_cpu_freq = 16; // Valore in MHz (16 o 80)
char menu_buffer[100];
static char buffer_invio[100]; // Per lo sniffer

// --- PROTOTIPI ---
void init(void);
void scrivi_char(char c);
void scrivi_stringa(char* str);
void set_clock_80MHz(void);
void set_clock_16MHz(void);
void stampa_intestazione(void);
void gestisci_menu_frequenza(void);
void Gestione_Sniffer_CAN(long cpu_freq); // Funzione sniffer aggiunta

int main(void) {
    init(); 
    scrivi_stringa("\n\r** STM32F413 PRONTA **\n\r");

    while(1) {
        stampa_intestazione();
        
        // Attesa carattere
        while(!(USART1_SR & 0x00000020)); 
        char scelta = (char)USART1_DR;

        switch(scelta) {
            case '1': 
                gestisci_menu_frequenza(); 
                break;
            case '2': 
                // CORRETTO: Passo la frequenza in Hz (MHz * 1000000)
                // La funzione non ritorna più (ha un while(1) interno)
                Gestione_Sniffer_CAN(current_cpu_freq * 1000000); 
                break;
            default:  
                scrivi_stringa("\n\r[ERR] Comando errato.\n\r"); 
                break;
        }
    }
}

void init(void) {
    // Clock Enable
    RCC_AHB1ENR = (long)0x00000001; 
    RCC_APB1ENR = (long)0x10000000; 
    RCC_APB2ENR = (long)0x00000010; 

    // Power
    PWR_CR = (long)0x0000C000; 

    // GPIOA Config
    GPIOA_MODER = (long)0xA8280400;  
    GPIOA_PUPDR = (long)0x00140000; 
    GPIOA_AFRH  = (long)0x00000770;  

    // USART1: 9600 baud @ 16 MHz
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
    scrivi_stringa("1. Clock | 2. CAN TX RX \n\r> ");
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

// --- IMPLEMENTAZIONE SNIFFER CAN ---
void Gestione_Sniffer_CAN(long cpu_freq) 
{
    long rx_id, rx_len, rx_data_L, rx_data_H;
    char carattere_rx;
    long timeout = 0;

    // 1. CLOCK
    // Attivo CAN1 (Bit 25 APB1) e GPIOA (Bit 0 AHB1)
    RCC_AHB1ENR |= 0x00000001; 
    RCC_APB1ENR |= 0x02000000; 

    // 2. GPIO (PA11, PA12 per CAN)
    // Non tocco USART1 (PA9/PA10)
    GPIOA_MODER &= 0xFC3FFFFF; 
    GPIOA_MODER |= 0x02800000;

    GPIOA_AFRH &= 0xFFF00FFF;
    GPIOA_AFRH |= 0x00099000; // AF9 = CAN1

    scrivi_stringa("\r\n--- SNIFFER CAN ATTIVO ---\r\n");

    // 4. CONFIGURAZIONE CAN
    CAN1_MCR |= 0x00000001;          // Init Request
    while(!(CAN1_MSR & 0x00000001)); // Wait Ack

    CAN1_MCR &= 0xFFFFFFFD; // Exit Sleep

    // Impostazione Velocità (BTR)
    // Soglia 40MHz per distinguere 16M da 80M
    if(cpu_freq > 40000000) 
    {
        // 80 MHz CPU -> 40 MHz APB1 -> 500kbps
        // Prescaler=4 (Reg=3), TS1=15 (Reg=14), TS2=4 (Reg=3)
        CAN1_BTR = 0x003E0003; 
    } 
    else 
    {
        // 16 MHz CPU -> 16 MHz APB1 -> 500kbps
        // Prescaler=2 (Reg=1), TS1=12 (Reg=11), TS2=3 (Reg=2)
        CAN1_BTR = 0x002B0001;
    }

    // Configurazione Filtri (Pass-All)
    CAN_FMR |= 1;    
    CAN_FA1R &= ~1;  
    CAN_FS1R |= 1;   
    CAN_FM1R &= ~1;  
    
    CAN_F0R1 = 0;    
    CAN_F0R2 = 0;    // Maschera 0 = Accetta Tutto

    CAN_FA1R |= 1;   
    CAN_FMR &= ~1;   

    // Avvio
    CAN1_MCR &= 0xFFFFFFFE; 
    
    timeout = 0;
    while(CAN1_MSR & 0x00000001) 
    {
        timeout++;
        if(timeout > 2000000) 
        {
            scrivi_stringa("ERRORE: CAN non sincronizzato.\r\n");
            break; 
        }
    }

    if(timeout <= 2000000) scrivi_stringa("CAN Avviato. 't' per invio.\r\n");

    // LOOP INFINITO SNIFFER
    while(1) 
    {
        // RICEZIONE
        if((CAN1_RF0R & 0x00000003) != 0) 
        {
            long raw_id = CAN1_RI0R;
            
            if(raw_id & 0x00000004) 
            {
                rx_id = (raw_id >> 3) & 0x1FFFFFFF;
                scrivi_stringa("EXT ");
            } 
            else 
            {
                rx_id = (raw_id >> 21) & 0x7FF;
                scrivi_stringa("STD ");
            }

            rx_len = CAN1_RDT0R & 0x0F;
            rx_data_L = CAN1_RDL0R;
            rx_data_H = CAN1_RDH0R;

            CAN1_RF0R |= 0x00000020; // Rilascio FIFO

            sprintf(buffer_invio, "ID: %X L: %d Dati: %08X %08X\r\n", 
                    (unsigned int)rx_id, (int)rx_len, (unsigned int)rx_data_L, (unsigned int)rx_data_H);
            scrivi_stringa(buffer_invio);
        }

        // TRASMISSIONE
        if(USART1_SR & 0x00000020) 
        {
            carattere_rx = (char)USART1_DR;
            
            if(carattere_rx == 't' || carattere_rx == 'T') 
            {
                scrivi_stringa("Invio Test...\r\n");
                
                if(CAN1_TSR & 0x04000000) 
                {
                    CAN1_TI0R = 0x24600000; // ID 0x123
                    CAN1_TDT0R = 8; 
                    CAN1_TDL0R = 0x11223344; 
                    CAN1_TDH0R = 0xAABBCCDD; 
                    CAN1_TI0R |= 0x00000001; 
                } 
                else 
                {
                    scrivi_stringa("Err: Mailbox Piena\r\n");
                }
            }
        }
    }
}