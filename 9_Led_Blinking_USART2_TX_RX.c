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
void Gestione_Sniffer_CAN(long cpu_freq);

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
                // Passo la frequenza in Hz
                Gestione_Sniffer_CAN(current_cpu_freq * 1000000); 
                break;
            default:  
                scrivi_stringa("\n\r[ERR] Comando errato.\n\r"); 
                break;
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

// Funzione per leggere un numero esadecimale da tastiera (es: scrivi "1A" -> invio)
long leggi_numero_hex(void) {
    char buf[9]; // Max 8 cifre hex + terminatore
    int idx = 0;
    char c;
    long valore = 0;
    
    while(1) {
        while(!(USART1_SR & 0x00000020)); // Attesa RX
        c = (char)USART1_DR;
        
        // Se premo Invio (\r) finisco
        if(c == '\r') break;
        
        // Echo del carattere (per vedere cosa scrivo)
        scrivi_char(c);
        
        // Salvo nel buffer se c'è spazio
        if(idx < 8) {
            buf[idx++] = c;
        }
    }
    buf[idx] = 0; // Termino stringa
    
    // Conversione manuale ASCII -> HEX
    // sscanf(buf, "%x", &valore); // Metodo standard, ma facciamolo a mano per sicurezza
    valore = 0;
    for(int i=0; i<idx; i++) {
        char ch = buf[i];
        int digit = 0;
        if(ch >= '0' && ch <= '9') digit = ch - '0';
        else if(ch >= 'A' && ch <= 'F') digit = ch - 'A' + 10;
        else if(ch >= 'a' && ch <= 'f') digit = ch - 'a' + 10;
        
        valore = (valore << 4) | digit; // Shift e aggiungi
    }
    
    scrivi_stringa("\r\n"); // A capo dopo invio
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
    
    // Variabili per l'input manuale
    long user_id, user_dlc, user_dataL, user_dataH;

    // 1. CLOCK
    RCC_AHB1ENR |= 0x00000001; // GPIOA
    RCC_APB1ENR |= 0x02000000; // CAN1 (Bit 25)

    // 2. GPIO (PA11=RX, PA12=TX)
    GPIOA_MODER &= 0xFC3FFFFF; 
    GPIOA_MODER |= 0x02800000; // Alternate Function
    
    // --- FONDAMENTALE PER LOOPBACK E INIT ---
    // Il Pull-Up evita che RX legga 0 e blocchi l'inizializzazione
    GPIOA_PUPDR &= 0xFC3FFFFF; 
    GPIOA_PUPDR |= 0x01400000; 

    GPIOA_AFRH &= 0xFFF00FFF;
    GPIOA_AFRH |= 0x00099000;  // AF9 (CAN1)

    scrivi_stringa("\r\n--- SNIFFER CAN (Loopback+PullUp) ---\r\n");

    // 4. CONFIGURAZIONE CAN (SEQUENZA FIX INIT)
    
    // A. RESET SOFTWARE (MCR Bit 15)
    // Questo è il trucco per sbloccare l'INRQ se è impallato
    CAN1_MCR |= 0x00008000; 
    while(CAN1_MCR & 0x00008000); // Aspetto che il reset finisca

    // B. USCITA DA SLEEP MODE (MCR Bit 1)
    CAN1_MCR &= ~0x00000002; 
    while(CAN1_MSR & 0x00000002); // Aspetto SLAK=0

    // C. RICHIESTA INIT MODE (MCR Bit 0)
    CAN1_MCR |= 0x00000001; 
    while(!(CAN1_MSR & 0x00000001)); // Aspetto INAK=1 (Ora dovrebbe andare!)

    // D. CONFIGURAZIONE BIT TIMING
    if(cpu_freq > 40000000) 
        CAN1_BTR = 0x003E0003; // 80 MHz -> 500k
    else 
        CAN1_BTR = 0x002B0001; // 16 MHz -> 500k

    // E. ATTIVAZIONE LOOPBACK
    // Se lo commenti, assicurati di avere i fili collegati!
    CAN1_BTR |= 0x40000000; 

    // F. FILTRI (Accetta tutto)
    CAN_FMR |= 1;    
    CAN_FA1R &= ~1;  
    CAN_FS1R |= 1;   
    CAN_FM1R &= ~1;  
    CAN_F0R1 = 0;    
    CAN_F0R2 = 0;    
    CAN_FA1R |= 1;   
    CAN_FMR &= ~1;   

    // G. AVVIO (Uscita Init Mode)
    CAN1_MCR &= ~0x00000001; // INRQ=0
    
    // H. ATTESA SINCRONIZZAZIONE
    timeout = 0;
    while(CAN1_MSR & 0x00000001) 
    {
        timeout++;
        if(timeout > 8000000) // Timeout aumentato
        {
            scrivi_stringa("ERRORE CRITICO: CAN Init Fallito.\r\n");
            scrivi_stringa("Probabile causa: Clock errato o Pin RX a massa.\r\n");
            break; 
        }
    }

    if(timeout <= 8000000) scrivi_stringa("CAN Avviato. Premi 't' per menu invio.\r\n");

    while(1) 
    {
        // RICEZIONE (Sniffer)
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

        // TRASMISSIONE MANUALE (Menu)
        if(USART1_SR & 0x00000020) 
        {
            carattere_rx = (char)USART1_DR;
            if(carattere_rx == 't' || carattere_rx == 'T') 
            {
                // -- INIZIO MENU COMPILAZIONE PACCHETTO --
                scrivi_stringa("\r\n--- COMPILAZIONE PACCHETTO ---\r\n");
                
                scrivi_stringa("Inserisci ID Hex (es: 123): ");
                user_id = leggi_numero_hex();
                
                scrivi_stringa("Inserisci DLC (0-8): ");
                user_dlc = leggi_numero_hex();
                if(user_dlc > 8) user_dlc = 8;

                scrivi_stringa("Dati Low Hex (es: AABB): ");
                user_dataL = leggi_numero_hex();

                scrivi_stringa("Dati High Hex (es: CCDD): ");
                user_dataH = leggi_numero_hex();

                // Invio effettivo
                if(CAN1_TSR & 0x04000000) 
                {
                    // Nota: Assumo Standard ID per semplicità. 
                    // Se vuoi Extended, dovresti chiedere anche quello.
                    CAN1_TI0R = (user_id << 21); // Sposto a sinistra per STD ID
                    
                    CAN1_TDT0R = user_dlc; 
                    CAN1_TDL0R = user_dataL; 
                    CAN1_TDH0R = user_dataH; 
                    
                    CAN1_TI0R |= 0x00000001; // TX Request
                    scrivi_stringa("Pacchetto Inviato!\r\n");
                } 
                else scrivi_stringa("Err: Mailbox Piena\r\n");
            }
        }
    }
}