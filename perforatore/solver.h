/*
*	Classe base Solver: definisce un oggetto che risolve un problema TSP. La classe Solver sarà alla base di altri Solver più complessi, tra i quali anche CPLEXSolver
*	Un oggetto Solver contiene un puntatore all'istanza che risolve, e un puntatore ad un oggetto Soluzione che sarà popolato al momento della risoluzionel
*	L'oggetto Solver si occupa della risoluzione del problema, della misurazione del tempo impiegato e della creazione della soluzione che viene restituita all'utente
*/


#ifndef SOLVER_H
#define SOLVER_H

#include <iostream>
#include "istanza.h"
#include "soluzione.h"

using namespace std;

class Solver{
public:
	static bool verbose;							// indica se stampare sulla console messaggi di log sull'avanzamento del programma

	Solver(Istanza*);								// Solver creato di default per un'istanza ---> crea una Soluzione di default
	Solver(Istanza*, Soluzione*);						// Solver creato con soluzione già esistente
	
//	void risolvi();
	Soluzione* getSoluzione();

private:
	Istanza* ist;
	Soluzione* sol;
	
};


#endif // SOLVER_H
