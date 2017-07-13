/*
*	Classe Soluzione, definisce una soluzione ad una determinata Istanza
*	Un oggetto Soluzione può essere creato tramite un solo puntatore a oggetto Istanza (in questo caso la soluzione sarà data dai punti nell'ordine iniziale) oppure passando un array di punti ordinati
*	
*/


#ifndef SOLUZIONE_H
#define SOLUZIONE_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <math.h>
#include <random>
#include "punto.h"
#include "istanza.h"

using namespace std;

class Soluzione{
public:
	static bool verbose;							// indica se stampare sulla console messaggi di log sull'avanzamento del programma

	Soluzione(Istanza*);							// Soluzione creata di default per un'istanza ---> usa l'ordine dei punti che trova dentro l'istanza
	Soluzione(Istanza*, vector<short>*);				// Soluzione creata con un array di indici 

	int getN();									// restituisce il numero di nodi
	double getFO();								// restituisce il valore della Funzione Obiettivo della soluzione corrente
	void stampa();									// stampa l'istanza (lista di punti)
	
	string toFileJSON(string);						// esporta in un file di testo la soluzione in formato JSON 
	

private:
	int N;
	double FO;									// contiene il valore della Funzione Obiettivo
	Istanza* ist;									// puntatore all'oggetto Istanza di cui è soluzione
	vector<short> nodiOrdinati;						// contiene gli indici dei nodi della soluzione
	double calcolaFO();								// calcola la funzione obiettivo della soluzione, cioè la distanza tra tutti i nodi seguendo il loro ordine (compreso la distanza dall'ultimo dal primo --> ritorno al punto di partenza )

};


#endif // SOLUZIONE_H
