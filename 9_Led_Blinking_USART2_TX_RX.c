#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "STM32F413XX.h" 

// --- DEFINIZIONI REGISTRI (Volatile) ---
#ifndef CAN1_BASE
#define CAN1_BASE    0x40006400
#define CAN1_MCR     *(volatile long*)(CAN1_BASE + 0x00)
#define CAN1_MSR     *(volatile long*)(CAN1_BASE + 0x04)
#define CAN1_TSR     *(volatile long*)(CAN1_BASE + 0x08)
#define CAN1_RF0R    *(volatile long*)(CAN1_BASE + 0x0C)
#define CAN1_BTR     *(volatile long*)(CAN1_BASE + 0x1C)
#define CAN1_ESR     *(volatile long*)(CAN1_BASE + 0x18) 
#define CAN1_TI0R    *(volatile long*)(CAN1_BASE + 0x180)
#define CAN1_TDT0R   *(volatile long*)(CAN1_BASE + 0x184)
#define CAN1_TDL0R   *(volatile long*)(CAN1_BASE + 0x188)
#define CAN1_TDH0R   *(volatile long*)(CAN1_BASE + 0x18C)
#define CAN1_RI0R    *(volatile long*)(CAN1_BASE + 0x1B0)
#define CAN1_RDT0R   *(volatile long*)(CAN1_BASE + 0x1B4)
#define CAN1_RDL0R   *(volatile long*)(CAN1_BASE + 0x1B8)
#define CAN1_RDH0R   *(volatile long*)(CAN1_BASE + 0x1BC)
#define CAN_FMR      *(volatile long*)(CAN1_BASE + 0x200)
#define CAN_FM1R     *(volatile long*)(CAN1_BASE + 0x204)
#define CAN_FS1R     *(volatile long*)(CAN1_BASE + 0x20C)
#define CAN_FA1R     *(volatile long*)(CAN1_BASE + 0x21C)
#define CAN_F0R1     *(volatile long*)(CAN1_BASE + 0x240)
#define CAN_F0R2     *(volatile long*)(CAN1_BASE + 0x244)
#endif

// GPIO Indirizzi Puri
#define RCC_AHB1ENR_RAW  *(volatile long*)0x40023830
#define RCC_APB1ENR_RAW  *(volatile long*)0x40023840
#define GPIOB_MODER_RAW  *(volatile long*)0x40020400
#define GPIOB_AFRH_RAW   *(volatile long*)0x40020424
#define GPIOB_IDR_RAW    *(volatile long*)0x40020410
#define GPIOB_PUPDR_RAW  *(volatile long*)0x4002040C

static long current_cpu_freq = 16; 
static char menu_buffer[100];
static char buffer_invio[100];

// PROTOTIPI
void init(void);
void scrivi_char(char c);
void scrivi_stringa(char* str);
void set_clock_80MHz(void);
void set_clock_16MHz(void);
void stampa_intestazione(void);
void gestisci_menu_frequenza(void);
void Gestione_Sniffer_CAN(long cpu_freq);
long leggi_numero_hex(void); 
void Dump_Registri_CAN(void); 

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

// Funzione DEBUG robusta
void Dump_Registri_CAN(void) {
    scrivi_stringa("\r\n--- DUMP REGISTRI ---\r\n");
    sprintf(menu_buffer, "MCR: %08X\r\n", (unsigned int)CAN1_MCR); scrivi_stringa(menu_buffer);
    sprintf(menu_buffer, "MSR: %08X (INAK=%d)\r\n", (unsigned int)CAN1_MSR, (int)(CAN1_MSR & 1)); scrivi_stringa(menu_buffer);
    
    unsigned int esr_val = (unsigned int)CAN1_ESR;
    sprintf(menu_buffer, "ESR: %08X (LEC=%d)\r\n", esr_val, (int)((esr_val >> 4) & 0x7)); 
    scrivi_stringa(menu_buffer);
    
    scrivi_stringa("---------------------\r\n");
}

void Gestione_Sniffer_CAN(long cpu_freq) 
{
    long rx_id, rx_len, rx_data_L, rx_data_H;
    char carattere_rx;
    long user_id, user_dlc, user_dataL, user_dataH;
    long timeout;

    // 1. CLOCK (Abilito e aspetto)
    RCC_AHB1ENR_RAW |= 0x00000003; // GPIOA, GPIOB
    RCC_APB1ENR_RAW |= 0x02000000; // CAN1
    
    for(int i=0; i<10000; i++); 

    // 2. GPIO PB8/PB9 Setup
    GPIOB_MODER_RAW &= ~0x000F0000; 
    GPIOB_MODER_RAW |= 0x000A0000; // AF Mode
    
    // --- CORREZIONE: AF8 (0x88) per PB8/PB9 su F413 ---
    GPIOB_AFRH_RAW &= ~0x000000FF;
    GPIOB_AFRH_RAW |= 0x00000088;  // AF8 (CAN1)

    // PullUp disabilitati (uso Transceiver)
    GPIOB_PUPDR_RAW &= ~0x000F0000; 

    scrivi_stringa("\r\n--- CAN CONFIG AF8 (PB8/PB9) ---\r\n");

    // Reset CAN
    CAN1_MCR |= 0x00008000;
    while(CAN1_MCR & 0x00008000);

    // Richiesta Init (Forzata)
    CAN1_MCR = 0x00000001; 
    
    timeout=0;
    while((CAN1_MSR & 1) == 0) {
        if(timeout++ > 100000) { 
            scrivi_stringa("Fail (INRQ stuck)\r\n"); 
            Dump_Registri_CAN();
            return; 
        }
    }

    // --- CONFIGURAZIONE TIMING ---
    // Target: 500 kbit/s
    
   // --- CONFIGURAZIONE TIMING CON OPZIONE LOOPBACK ---
    scrivi_stringa("\r\nAttivare Loopback Mode per test interno? (s/n): ");
    while(!(USART1_SR & 0x00000020));
    char lb_choice = (char)USART1_DR;

    if(cpu_freq > 40000000) 
    {
        // CASO 80 MHz -> APB1 = 40 MHz
        CAN1_BTR = 0x003E0003; 
        if(lb_choice == 's' || lb_choice == 'S') CAN1_BTR |= 0x40000000; 
        scrivi_stringa("\r\nMode: 40MHz APB1 (80MHz CPU)");
    } 
    else 
    {
        // CASO 16 MHz -> APB1 = 16 MHz
        CAN1_BTR = 0x002B0001;
        if(lb_choice == 's' || lb_choice == 'S') CAN1_BTR |= 0x40000000; 
        scrivi_stringa("\r\nMode: 16MHz APB1 (16MHz CPU)");
    }

    if(lb_choice == 's' || lb_choice == 'S') scrivi_stringa(" [LOOPBACK ATTIVO]\r\n");
    else scrivi_stringa(" [NORMAL MODE]\r\n");
		
    // Filtri
    CAN_FMR |= 1;    
    CAN_FA1R &= ~1;  
    CAN_FS1R |= 1;   
    CAN_FM1R &= ~1;  
    CAN_F0R1 = 0;    
    CAN_F0R2 = 0;    
    CAN_FA1R |= 1;   
    CAN_FMR &= ~1;   

    // AVVIO (Uscita Init)
    CAN1_MCR &= ~0x00000001; 

    // Attesa Sync (INAK=0)
    timeout = 0;
    while(CAN1_MSR & 0x00000001) {
        timeout++;
        if(timeout > 8000000) { 
            scrivi_stringa("ERRORE: Init Failed (INAK stuck).\r\n");
            Dump_Registri_CAN(); 
            return; 
        }
    }
    
    scrivi_stringa("CAN AVVIATO! 't' per invio.\r\n");

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