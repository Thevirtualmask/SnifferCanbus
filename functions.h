#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdint.h> // Per float e uint32_t

//prototipi 
void init(void);
void scrivi_char(char c);
void scrivi_stringa(char *str);
// Aggiungi in functions.h
void set_clock_16MHz(void);
void funzione_can_transmit_sim(void);
void funzione_can_receive_sim(void);
// Variabili condivise (se servono in più file)
extern char temp_string[];

#endif