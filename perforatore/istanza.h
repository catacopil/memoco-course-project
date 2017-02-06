/*
*	Classe Istanza, definisce un'istanza di un problema
*	Un oggetto istanza può essere creato a partire da una matrice delle distanze oppure da una lista di punti
*/


#ifndef ISTANZA_H
#define ISTANZA_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <math.h>
#include <random>
#include "punto.h"

using namespace std;

class Istanza{

public:
	static bool verbose;							// indica se stampare sulla console messaggi di log sull'avanzamento del programma
	
	Istanza(int);									// generazione casuale istanza
	Istanza(string);								// lettura istanza da file

	int getN();									// restituisce il numero di nodi
	vector<Punto>* getNodi();						// restituisce un puntatore ad una copia di arr_nodi
	void stampaNodi();								// stampa l'istanza (lista di punti)
	
	string toFileJSON(string);						// esporta in un file di testo l'istanza in formato JSON
	string toFileMatriceDistanze(string);				// esporta in un file di testo la matrice delle distanze
	


private:
	int N;
	vector<Punto> arr_nodi;							// contiene i nodi dell'istanza (lista di punti)
	vector<vector<double>> M;						// la matrice delle distanze tra i nodi dell'istanza
	void calcolaMatriceDistanze();					// calcola la matrice delle distanze

};



#endif // ISTANZA_H
