/*
*	Classe Punto: rappresenta un nodo dell'Istanza, le coordinate sono due numeri interi che variano tra 0 e 1000
*	Contiene vari metodi di utilità quali: il metodo distanza() che calcola la distanza tra 2 punti, il metodo stampa() e operatore di uguaglianza
*	
*/

#ifndef PUNTO_H
#define PUNTO_H

#include <iostream>
#include <sstream> 
#include <cstdlib>
#include <string.h>
#include <math.h>
#include <random>

using namespace std;

class Punto{
public:
	int x, y;
	Punto(int, int);						// creazione punto standard
	double distanza(Punto*);		// calcola la distanza assoluta tra due punti
	string stampa();
	bool operator== (const Punto&) const;
};

#endif // PUNTO_H
