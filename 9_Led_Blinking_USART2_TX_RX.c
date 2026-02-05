#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "STM32F413XX.h"


// Per diagnostica pin GPIOA
#ifndef GPIOA_IDR
#define GPIOA_IDR    *(volatile long*)0x40020010
#define GPIOA_PUPDR  *(volatile long*)0x4002000C
#endif

// --- VARIABILI GLOBALI ---
long current_cpu_freq = 16; 
char menu_buffer[100];
static char buffer_invio[100];

// --- PROTOTIPI ---
void init(void);
void scrivi_char(char c);
void scrivi_stringa(char* str);
void set_clock_80MHz(void);
void set_clock_16MHz(void);
void stampa_intestazione(void);
void gestisci_menu_frequenza(void);
void Gestione_Sniffer_CAN(long cpu_freq);
long leggi_numero_hex(void); 

int main(void) {
    init(); 
    scrivi_stringa("\n\r** STM32F413 READY **\n\r");

    while(1) {
        stampa_intestazione();
        while(!(USART1_SR & 0x00000020)); 
        char scelta = (char)USART1_DR;

        switch(scelta) {
            case '1': gestisci_menu_frequenza(); break;
            case '2': Gestione_Sniffer_CAN(current_cpu_freq * 1000000); break;
            default:  scrivi_stringa("\n\r[ERR] Comando errato.\n\r"); break;
        }
    }
}

void init(void) {
    RCC_AHB1ENR = (long)0x00000001; 
    RCC_APB1ENR = (long)0x10000000; 
    RCC_APB2ENR = (long)0x00000010; 
    PWR_CR = (long)0x0000C000; 
    GPIOA_MODER = (long)0xA8280400;  
    GPIOA_PUPDR = (long)0x00140000; 
    GPIOA_AFRH  = (long)0x00000770;  
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

long leggi_numero_hex(void) {
    char buf[9]; 
    int idx = 0;
    char c;
    long valore = 0;
    
    while(1) {
        while(!(USART1_SR & 0x00000020)); 
        c = (char)USART1_DR;
        if(c == '\r') break;
        scrivi_char(c);
        if(idx < 8) buf[idx++] = c;
    }
    buf[idx] = 0; 
    
    valore = 0;
    for(int i=0; i<idx; i++) {
        char ch = buf[i];
        int digit = 0;
        if(ch >= '0' && ch <= '9') digit = ch - '0';
        else if(ch >= 'A' && ch <= 'F') digit = ch - 'A' + 10;
        else if(ch >= 'a' && ch <= 'f') digit = ch - 'a' + 10;
        valore = (valore << 4) | digit; 
    }
    scrivi_stringa("\r\n"); 
    return valore;
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

void Gestione_Sniffer_CAN(long cpu_freq) 
{
    long rx_id, rx_len, rx_data_L, rx_data_H;
    char carattere_rx;
    long timeout = 0;
    long user_id, user_dlc, user_dataL, user_dataH;

    // 1. CLOCK (GPIOA per UART, GPIOB per CAN, CAN1)
    RCC_AHB1ENR |= 0x00000003; 
    RCC_APB1ENR |= 0x02000000; 

    // 2. CONFIGURAZIONE GPIO SU PORTA B (PB8=RX, PB9=TX)
    GPIOB_MODER &= ~0x000F0000; 
    GPIOB_MODER |= 0x000A0000; // AF Mode

    // --- RITORNO AD AF9 (0x99) - CORRETTO PER F413 ---
    GPIOB_AFRH &= ~0x000000FF;
    GPIOB_AFRH |= 0x00000099;  // AF9 (CAN1)

    // ATTIVO PULL-UP INTERNO SU PB8 (RX) PER SICUREZZA
    // Se il transceiver non spinge bene a 3.3V, questo aiuta l'Init a passare.
    // Maschera: 0x000F0000. Scrittura: 0x00010000 (01 su bit 17:16)
    GPIOB_PUPDR &= ~0x000F0000;
    GPIOB_PUPDR |= 0x00010000;

    scrivi_stringa("\r\n--- SNIFFER CAN (PB8/PB9 - AF9) ---\r\n");

    // DIAGNOSTICA REALE
    long pin_rx_val = (GPIOB_IDR & (1 << 8)); 
    if(pin_rx_val) {
        scrivi_stringa("[HW CHECK] Pin PB8 letto come ALTO (1). Hardware OK.\r\n");
    } else {
        scrivi_stringa("[HW CHECK] Pin PB8 letto come BASSO (0). \r\n");
        scrivi_stringa("ATTENZIONE: Se hai fatto il corto, DEVI leggere ALTO qui.\r\n");
    }

    // --- SEQUENZA DI INIT ---
    
    // Reset e Uscita Sleep Atomica
    CAN1_MCR |= 0x00008000; 
    while(CAN1_MCR & 0x00008000); 
    CAN1_MCR = 0x00000001; 

    // Attesa INAK=1
    timeout = 0;
    while((CAN1_MSR & 0x00000001) == 0) {
        if(timeout++ > 1000000) {
            scrivi_stringa("ERRORE CRITICO: INRQ bloccato.\r\n");
            break;
        }
    }

    if(cpu_freq > 40000000) CAN1_BTR = 0x003E0003; 
    else CAN1_BTR = 0x002B0001; 

    // Filtri
    CAN_FMR |= 1;    
    CAN_FA1R &= ~1;  
    CAN_FS1R |= 1;   
    CAN_FM1R &= ~1;  
    CAN_F0R1 = 0;    
    CAN_F0R2 = 0;    
    CAN_FA1R |= 1;   
    CAN_FMR &= ~1;   

    // AVVIO
    CAN1_MCR &= ~0x00000001; 
    
    // Attesa Sync (INAK -> 0)
    timeout = 0;
    while(CAN1_MSR & 0x00000001) 
    {
        timeout++;
        if(timeout > 8000000) 
        {
            scrivi_stringa("ERRORE: Init Failed (INAK=1).\r\n");
            scrivi_stringa("Il CAN non sente il bus libero su PB8.\r\n");
            // Se qui fallisce anche col corto, controlla il [HW CHECK] sopra.
            break; 
        }
    }

    if(timeout <= 8000000) scrivi_stringa("CAN Avviato correttamente.\r\n");

    while(1) 
    {
        // RX
        if((CAN1_RF0R & 0x00000003) != 0) 
        {
            long raw_id = CAN1_RI0R;
            if(raw_id & 0x00000004) {
                rx_id = (raw_id >> 3) & 0x1FFFFFFF;
                scrivi_stringa("RX [EXT] ");
            } else {
                rx_id = (raw_id >> 21) & 0x7FF;
                scrivi_stringa("RX [STD] ");
            }
            rx_len = CAN1_RDT0R & 0x0F;
            rx_data_L = CAN1_RDL0R;
            rx_data_H = CAN1_RDH0R;
            CAN1_RF0R |= 0x00000020; 

            sprintf(buffer_invio, "ID:%X L:%d D:%08X %08X\r\n", 
                    (unsigned int)rx_id, (int)rx_len, (unsigned int)rx_data_L, (unsigned int)rx_data_H);
            scrivi_stringa(buffer_invio);
        }

        // TX
        if(USART1_SR & 0x00000020) 
        {
            carattere_rx = (char)USART1_DR;
            if(carattere_rx == 't' || carattere_rx == 'T') 
            {
                scrivi_stringa("\r\n--- INVIO ---\r\n");
                scrivi_stringa("ID Hex: ");
                user_id = leggi_numero_hex();
                scrivi_stringa("DLC: ");
                user_dlc = leggi_numero_hex();
                if(user_dlc > 8) user_dlc = 8;
                scrivi_stringa("Data L: ");
                user_dataL = leggi_numero_hex();
                scrivi_stringa("Data H: ");
                user_dataH = leggi_numero_hex();

                if(CAN1_TSR & 0x04000000) 
                {
                    CAN1_TI0R = (user_id << 21); 
                    CAN1_TDT0R = user_dlc; 
                    CAN1_TDL0R = user_dataL; 
                    CAN1_TDH0R = user_dataH; 
                    CAN1_TI0R |= 0x00000001; 
                    scrivi_stringa("Inviato!\r\n");
                } 
                else scrivi_stringa("Mailbox Piena\r\n");
            }
        }
    }
}