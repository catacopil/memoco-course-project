/*
*	Classe Nearest Solver: definisce un oggetto che risolve un problema TSP con l'euristica Nearest Neighbor (sceglie il più punto più vicino). 
*	Contiene un puntatore ad un oggetto Istanza e ad un oggetto soluzione creato dopo aver risolto il problema
*	
*/


#ifndef NEAREST_SOLVER_H
#define NEAREST_SOLVER_H

#include <iostream>
#include <string>
#include <math.h>
#include <time.h>
#include <float.h>
#include <algorithm>
#include "solver.h"
#include "punto.h"

using namespace std;


class NearSolver: public Solver{
public:
	static bool verbose;								// indica se stampare sulla console messaggi di log sull'avanzamento del programma

	NearSolver(Istanza*);								// Creazione del solver, collegandolo con l'oggetto Istanza che deve risolvere
	
	void risolvi(int);
	Soluzione* getSoluzione();							// ritorna la Soluzione
	double getTempoRisoluzione();							// ritorna il tempo (in secondi) impiegato per la risoluzione, se non ancora risolto ritorna -1
	double getFO();									// ritorna il valore della Funzione Obiettivo, se non ancora risolto ritorna -1
	
// 	~NearSolver();		TODO: SERVE ?

private:
	double tempoRisoluzione;
	double valoreFO;
	vector<vector<double>>* distanze;
};


#endif // NEAREST_SOLVER_H