/*
*	Classe CPLEX Solver: definisce un oggetto che risolve un problema TSP utilizzando la libreria CPLEX. 
*	Contiene un puntatore ad un oggetto Istanza e ad un oggetto soluzione creato dopo aver risolto il problema
*	
*/


#ifndef CPLEX_SOLVER_H
#define CPLEX_SOLVER_H

#include <iostream>
#include <string>
#include <math.h>
#include <time.h>
#include "cpxmacro.h"
#include "solver.h"

using namespace std;


class CPLEX_Solver: public Solver{
public:
	static bool verbose;								// indica se stampare sulla console messaggi di log sull'avanzamento del programma

	CPLEX_Solver(Istanza*, CEnv, Prob);								// Solver creato di default per un'istanza ---> crea una Soluzione di default
	
	void risolvi();
	Soluzione* getSoluzione();
	
	~CPLEX_Solver();

private:
	const CEnv ENV;
	Prob LP;
	double tempoRisoluzione;
	void setupLP();
};


#endif // CPLEX_SOLVER_H
