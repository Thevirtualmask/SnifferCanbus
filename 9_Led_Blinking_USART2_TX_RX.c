#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "STM32F413xx.h" // Header corretto
#include "functions.h"

// Variabili di stato
uint32_t current_cpu_freq = 16;
char menu_buffer[100];

// Prototipi locali
void stampa_intestazione(void);
void gestisci_menu_frequenza(void);

// Importa funzione specifica F413
extern void set_clock_100MHz(void);

int main(void)
{
	char scelta;
	init(); // Partenza a 16 MHz
	scrivi_stringa("\n\r\n\r** STM32F413 BOOT SUCCESSFUL **\n\r");
	while(1)
		{
			stampa_intestazione();

			while(!(USART1_SR & 0x00000020)); // Attesa RX
				scelta = (char)USART1_DR;
				switch(scelta)
					{
						case '1':
						scrivi_stringa("\n\r[CMD] Configurazione Clock...\n\r");
						gestisci_menu_frequenza();
						break;
						case '2':
						funzione_can_transmit_sim();
						break;
						case '3':
						funzione_can_receive_sim();
						break;
						default:
						scrivi_stringa("\n\r[ERR] Comando non valido.\n\r");
						break;
					}
				}
}

void stampa_intestazione(void)
{
	scrivi_stringa("\n\r--- F413 DASHBOARD ---\n\r");
	sprintf(menu_buffer, "Clock attuale: %d MHz\n\r", (int)current_cpu_freq);
	scrivi_stringa(menu_buffer);
	scrivi_stringa("1. Modifica Frequenza (16/100 MHz)\n\r");
	scrivi_stringa("2. Test CAN TX\n\r");
	scrivi_stringa("3. Test CAN RX\n\r");
	scrivi_stringa("Inserisci comando > ");
}

void gestisci_menu_frequenza(void)
{
 char sub_scelta;
 scrivi_stringa("\n\r--- SELEZIONE PERFORMANCE ---\n\r");
 scrivi_stringa("a. Modalita' Low Power (16 MHz)\n\r");
 scrivi_stringa("b. Modalita' Turbo F413 (100 MHz)\n\r");
 scrivi_stringa("> ");
	
 while(!(USART1_SR & 0x00000020));
		sub_scelta = (char)USART1_DR;
		if(sub_scelta == 'a') {
			if(current_cpu_freq == 16) {
				scrivi_stringa("\n\r[INFO] Gia' in Low Power.\n\r");
 } else {
				scrivi_stringa("\n\r[SYS] Riduzione clock a 16 MHz...\n\r");
				set_clock_16MHz();
				current_cpu_freq = 16;
				scrivi_stringa("[OK] Clock ridotto.\n\r");
				}
} else if(sub_scelta == 'b') {
				if(current_cpu_freq == 100) {
				scrivi_stringa("\n\r[INFO] Gia' alla massima velocita'.\n\r");
 } else {
				scrivi_stringa("\n\r[SYS] Spinta PLL a 100 MHz (3WS Flash)...\n\r");
				set_clock_100MHz(); // Funzione specifica aggiornata
				current_cpu_freq = 100;
				scrivi_stringa("[OK] Sistema operativo a 100 MHz.\n\r");
				}
 } else {
				scrivi_stringa("\n\r[ERR] Scelta errata.\n\r");
				}
}

