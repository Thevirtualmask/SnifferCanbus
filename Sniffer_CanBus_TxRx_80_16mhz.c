#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "STM32F413XX.h" 

static long current_cpu_freq = 16; 
static char menu_buffer[100];
static char buffer_invio[100];

//prototipi
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
        while(!(USART1_SR & 0x00000020)); //finche' RXNE non diventa 1 (tasto seriale)
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
    RCC_APB1ENR = (long)0x10000000; //pwr
    RCC_APB2ENR = (long)0x00000010; //usart1
    PWR_CR = (long)0x0000C000; //scale 1
    GPIOA_MODER = (long)0xA8280400;  
    GPIOA_PUPDR = (long)0x00140000; 
    GPIOA_AFRH  = (long)0x00000770;  
    USART1_BRR = (long)0x00000683;  
    USART1_CR1 = (long)0x0000200C; 
}

void scrivi_char(char c) {
    while(!(USART1_SR & 0x00000080)); //aspetta TX vuoto (se TXE 1, il micro ha finito)
    USART1_DR = c;
}

void scrivi_stringa(char* str) {
    while(*str) scrivi_char(*str++);
}

//scrive nei reistri
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
    for(int i=0; i<idx; i++) //da buffer in numero
		{
        char ch = buf[i];
        int digit = 0;
        if(ch >= '0' && ch <= '9') digit = ch - '0';
        else if(ch >= 'A' && ch <= 'F') digit = ch - 'A' + 10;
        else if(ch >= 'a' && ch <= 'f') digit = ch - 'a' + 10;
        valore = (valore << 4) | digit; //da testo in numero 
    }
    scrivi_stringa("\r\n"); 
    return valore;
}

void set_clock_80MHz(void) {
    USART1_CR1 = (long)0x00000000; //UART off
    FLASH_ACR = (long)0x00000704; //introduco 2 wait states 
    RCC_PLLCFGR = (long)0x02002810; 
    RCC_CR = (long)0x01000081; //PLL on 
	
    while(!(RCC_CR & 0x02000000)); //aspetto che sia stabile (PLLRDY)
    RCC_CFGR = (long)0x00001002; //cambio clk (PLL)
	
    while ((RCC_CFGR & 0x0000000C) != 0x00000008); //check tramite Sys Clk Switch Stat 
    for(int i = 0; i < 200000; i++); 
    USART1_BRR = (long)0x0000208D; //aggiorno brr
    USART1_CR1 = (long)0x0000200C;
}

//operazione inversa
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
		while(!(USART1_SR & 0x00000020)); //finche' RXE e' uguale a 0 (prosegue solo con input)
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
    long user_id, user_dlc, user_dataL, user_dataH;
    long timeout;

    RCC_AHB1ENR |= 0x00000003; //GPIOA,GPIOB on
    RCC_APB1ENR |= 0x02000000; //CAN1 on 
    
    for(int i=0; i<10000; i++); //introduco un ritardo per assicurarsi che il CAN sia pronto 

    //PB8,PB9 
    GPIOB_MODER &= ~0x000F0000; 
    GPIOB_MODER |= 0x000A0000; //AF 
    GPIOB_AFRH &= ~0x000000FF;
    GPIOB_AFRH |= 0x00000088;  //AF8 (CAN1)
		GPIOB_PUPDR &= ~0x000F0000; //disabilito pullup perche' uso transceiver

    scrivi_stringa("\r\n--- CAN CONFIG AF8 (PB8/PB9) ---\r\n");

    //reset CAN
    CAN1_MCR |= 0x00008000;
    while(CAN1_MCR & 0x00008000);

    CAN1_MCR = 0x00000001; //init (INRQ on)
    
    timeout=0;
    while((CAN1_MSR & 1) == 0) //entra quando INAK=1
		{
        if(timeout++ > 100000) { 
            scrivi_stringa("Fail (INRQ stuck)\r\n"); 
            return; 
        }
    }

   //timing con opzione loopback (500 kbit/s), Baud Rate CAN=(fAPB1)/[prescaler*(bit time)]
    scrivi_stringa("\r\nAttivare Loopback Mode per test interno? (s/n): ");
    while(!(USART1_SR & 0x00000020));
    char lb_choice = (char)USART1_DR;

    if(cpu_freq > 40000000) //a 80MHz, APB1 sta a 40 MHz (non arriva a 80) 
    {
        CAN1_BTR = 0x003E0003; //contiene prescaler (/4), Time Segment 1 e 2, bit di sincronizzazione =20 quanti
        if(lb_choice == 's' || lb_choice == 'S') CAN1_BTR |= 0x40000000; //bit 30 loopback mode (on)
        scrivi_stringa("\r\nMode: 40MHz APB1 (80MHz CPU)");
    } 
    else 
    {
        //16 MHz, APB1=16 MHz (operazioni uguali)
        CAN1_BTR = 0x002B0001;
        if(lb_choice == 's' || lb_choice == 'S') CAN1_BTR |= 0x40000000; 
        scrivi_stringa("\r\nMode: 16MHz APB1 (16MHz CPU)");
    }

		//seriale
    if(lb_choice == 's' || lb_choice == 'S') scrivi_stringa(" [LOOPBACK ATTIVO]\r\n");
    else scrivi_stringa(" [NORMAL MODE]\r\n");
		
    //filtri
    CAN_FMR |= 1; //init
    CAN_FA1R &= ~1;  
    CAN_FS1R |= 1;   
    CAN_FM1R &= ~1;  
    CAN_F0R1 = 0;    
    CAN_F0R2 = 0;  //maschera off: sniffo ogni bit
    CAN_FA1R |= 1;   
    CAN_FMR &= ~1;   

    CAN1_MCR &= ~0x00000001; //INRQ=0 (uscita init)

    timeout = 0; //rst contatore
    while(CAN1_MSR & 0x00000001) //fino a INAK=0
			{
        timeout++;
        if(timeout > 8000000) { 
            scrivi_stringa("ERRORE: Init Failed (INAK stuck).\r\n");
            return; 
        }
			}
    
    scrivi_stringa("CAN AVVIATO! 't' per invio.\r\n");

    while(1) 
    {
        //RX
        if((CAN1_RF0R & 0x00000003) != 0) //se ci sono messaggi (RF0R != 0)
        {
            long raw_id = CAN1_RI0R;
            if(raw_id & 0x00000004) //se IDE=1, ID lungo 29 bit (IDE=0, ID lungo 11) 
							{
                rx_id = (raw_id >> 3) & 0x1FFFFFFF;
                scrivi_stringa("RX [EXT] ");
							} else 
								{
									rx_id = (raw_id >> 21) & 0x7FF;
									scrivi_stringa("RX [STD] ");
								}
            rx_len = CAN1_RDT0R & 0x0F; //lettura DLC
            rx_data_L = CAN1_RDL0R; //primi 4 byte
            rx_data_H = CAN1_RDH0R; //ultimi 4 byte
            CAN1_RF0R |= 0x00000020; //release FIFO (libera memoria per il futuro)
						
						//numeri in testo leggibile
            sprintf(buffer_invio, "ID:%X L:%d D:%08X %08X\r\n", 
                    (unsigned int)rx_id, (int)rx_len, (unsigned int)rx_data_L, (unsigned int)rx_data_H);
            scrivi_stringa(buffer_invio);
        }

        //TX
        if(USART1_SR & 0x00000020) //se RXNE=1 (interazione)
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
