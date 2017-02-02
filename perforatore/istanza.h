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

	Istanza(int);					// generazione casuale istanza
	Istanza(string);				// lettura istanza da file

	int getN();
	void stampaNodi();					// stampa l'istanza (lista di punti)
	
	string toFileJSON(string);
	string toFileMatriceDistanze(string);
	


private:
	int N;
	vector<Punto> arr_nodi;
	vector<vector<double>> M;
	void calcolaMatriceDistanze();

};



#endif // ISTANZA_H
