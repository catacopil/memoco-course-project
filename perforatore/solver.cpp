/*
*	Implementazione della classe Solver
*
*/

#include "solver.h"
#include <iostream>

bool Solver::verbose = false;

Solver::Solver(Istanza* i){
//  -------	COSTRUTTORE SEMPLICE, CREA UNA SOLUZIONE DI DEFAULT PER L'ISTANZA
	ist = i;
	sol = new Soluzione(ist);
	if (verbose) cout << " Solver creato per l'istanza richiesta con soluzione di default \n";
}


Solver::Solver(Istanza* i, Soluzione* s){
// ---------	COSTRUTTORE ISTANZA E SOLUZIONE ESISTENTE
	ist = i;
	sol = s;
	if (verbose) cout << " Solver creato per l'istanza richiesta con la soluzione passata \n";
}

Solver::~Solver(){
//  --------	DISTRUTTORE
	delete sol;
	if (verbose) cout << " Distrutto Solver \n";
}

Soluzione* Solver::getSoluzione(){
	return sol;
}
