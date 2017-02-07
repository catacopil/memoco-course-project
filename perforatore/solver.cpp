/*
*	Implementazione della classe Solver
*
*/

#include "solver.h"
#include <iostream>

bool Solver::verbose = true;

Solver::Solver(Istanza* i){
//  -------	COSTRUTTORE SEMPLICE, CREA UNA SOLUZIONE DI DEFAULT PER L'ISTANZA
	ist = i;
	sol = new Soluzione(ist);
	if (verbose) cout << " Solver creato per l'istanza richiesta con soluzione di default \n";
}


Soluzione::Soluzione(Istanza* i, Soluzione* s){
// ---------	COSTRUTTORE ISTANZA E SOLUZIONE ESISTENTE
// TODO: 	HA SENSO QUESTO COSTRUTTORE? Ha senso creare un solver con già una soluzione?
	ist = i;
	sol = s;
	if (verbose) cout << " Solver creato per l'istanza richiesta con la soluzione passata \n";
}


Soluzione* Solver::getSoluzione(){
	return sol;
}
