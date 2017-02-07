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
	Soluzione(Istanza*, vector<Punto>*);				// Soluzione creata con un array di punti 

	int getN();									// restituisce il numero di nodi
	void stampa();									// stampa l'istanza (lista di punti)
	
	string toFileJSON(string);						// esporta in un file di testo la soluzione in formato JSON 
	
//	bool checkAmmissibile();							// controlla se la soluzione è ammissibile (cioè se contiene un ciclo Hamiltoniano, senza sottocicli)

private:
	int N;
	Istanza* ist;
	vector<Punto> ordinati;							// contiene i nodi dell'istanza (lista di punti) 

};


#endif // SOLUZIONE_H
