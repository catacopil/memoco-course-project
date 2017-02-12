/*
*	Classe MioSolver: definisce un oggetto che risolve un problema TSP con l'algoritmo da me (Catalin Copil) ideato 
*	Contiene un puntatore ad un oggetto Istanza e ad un oggetto soluzione creato dopo aver risolto il problema
*	
*/


#ifndef MIO_SOLVER_H
#define MIO_SOLVER_H

#include <iostream>
#include <string>
#include <math.h>
#include <time.h>
#include <float.h>
#include <algorithm>
#include "solver.h"
#include "punto.h"

using namespace std;


class MioSolver: public Solver{
public:
	static bool verbose;								// indica se stampare sulla console messaggi di log sull'avanzamento del programma

	MioSolver(Istanza*, int);							// Creazione del solver, collegandolo con l'oggetto Istanza che deve risolvere, viene passato anche il parametro K
	
	void risolvi(int);
	Soluzione* getSoluzione();							// ritorna la Soluzione
	double getTempoRisoluzione();							// ritorna il tempo (in secondi) impiegato per la risoluzione, se non ancora risolto ritorna -1
	double getFO();									// ritorna il valore della Funzione Obiettivo, se non ancora risolto ritorna -1
	
// 	~MioSolver();		TODO: SERVE ??

private:
	double tempoRisoluzione;
	double valoreFO;
	const int maxK;
	vector<vector<double>>* distanze;
};


#endif // MIO_SOLVER_H
