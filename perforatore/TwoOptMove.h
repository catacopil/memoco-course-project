/*
*	Classe TwoOptMove, definisce una mossa 2-opt per una determinata soluzione di un'istanza
*	Un oggetto TwoOptMove può rappresentare una mossa valida o meno, se è valida allora porta un miglioramento alla soluzione
*/


#ifndef TWOOPTMOVE_H
#define TWOOPTMOVE_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "punto.h"
#include "istanza.h"
#include "soluzione.h"

using namespace std;

class TwoOptMove{

public:
	static bool verbose;							// indica se stampare sulla console messaggi di log sull'avanzamento del programma
	bool valida;									// indica se la mossa è valida (porta un miglioramento alla soluzione)
	double miglioramento;							// indica il guadagno che la mossa dà
	TwoOptMove(Istanza*, Soluzione*, int, int);			// Unico costruttore valido: ha puntatori a istanza e soluzione e agli indici dei primi punti dei segmenti interessati alla mossa
	int getPrimoSegmento();
	int getSecondoSegmento();


private:
	Istanza* ist;
	Soluzione* sol;
	int primoSegmento, secondoSegmento;
	
};



#endif // TWOOPTMOVE_H
