/*
*	Implementazione della classe NearSolver
*
*/

#include "Nearsolver.h"
#include <iostream>



bool NearSolver::verbose = false;

NearSolver::NearSolver(Istanza* i):Solver(i){
//  -------	COSTRUTTORE SEMPLICE
	tempoRisoluzione = -1.0;
	valoreFO = -1.0;
	distanze = ist->getDistanze();
	
	if (verbose) cout << " Solver Nearest Neighbor creato per l'istanza richiesta \n";
}


void NearSolver::risolvi(int start){	
//  ------- 	AVVIA IL SOLVER PER TROVARE LA SOLUZIONE

	try{
		vector<int> giainseriti;
		vector<vector<double>>* distanze = ist->getDistanze();
		vector<Punto>* nodiIstanza = ist->getNodi();
		vector<Punto> ordinati;
		int ultimoinserito = start;
		int ilminore = 0;
		int numeroNodi = ist->getN();
		cout << "\n Inizia l'esecuzione del NearSolver...." << endl;
		clock_t tempo = clock();
		
		ordinati.push_back((*nodiIstanza)[ultimoinserito]); 				// inserisco il primo
		giainseriti.push_back(start);
		for (int i=1; i<numeroNodi; i++){
			// parto con i=1 perché il primo l'ho già inserito e faccio girare il ciclo N-1 volte
			
			// scelgo il prossimo nodo
			double minDist = DBL_MAX;
			for (int j=0; j<numeroNodi; j++){
				if (ultimoinserito==j) continue;			// va avanti se il nodo è lo stesso
				bool piupiccolo = (minDist > (*distanze)[ultimoinserito][j]);
				bool esistegia = (find(giainseriti.begin(), giainseriti.end(), j) != giainseriti.end());
				if (piupiccolo && !esistegia){			// allora aggiorno come prossimo punto scelto
					ilminore = j;
					minDist = (*distanze)[ultimoinserito][j];
					}
			}
			if (minDist == DBL_MAX)						// non ho trovato un minimo
				throw runtime_error(" Non ho trovato una distanza minima, minore a DBL_MAX! "); 
			
			// lo aggiungo a ordinati, aggiorno gli indici e aggiorno il totale
			ordinati.push_back((*nodiIstanza)[ilminore]);
			giainseriti.push_back(ilminore);
			ultimoinserito = ilminore;
			
		}
		
		// creo oggetto Soluzione e aggiorno la funzione obiettivo
		sol = new Soluzione(ist, &ordinati);
		valoreFO = sol->getFO();
		
		
		tempo = clock() - tempo;
		tempoRisoluzione = ((float)tempo/CLOCKS_PER_SEC);
		if (tempoRisoluzione >= 60.0){
			int minuti = tempoRisoluzione/60;
			double secondi = fmod(tempoRisoluzione, 60);
			cout << " Problema risolto in "<< tempo <<" clocks ("<< minuti <<" minuti e "<< secondi <<" secondi)"<< endl;
			}
		else
			cout << " Problema risolto in "<< tempo <<" clocks ("<< tempoRisoluzione <<" secondi)"<< endl;
		
		cout << " Valore funzione obiettivo: " << valoreFO << " [start: "<< start <<"]"<<endl;			// stampa il valore della funzione obiettivo per la soluzione ottima
		
	}
	catch (std::exception& e) {
		cout << ">>> Eccezione durante l'esecuzione del Solver Nearest Neighbor: " << e.what() << endl<<endl;
	}
	//cout << " Problema risolto con il Solver Nearest Neighbor! \n";
}

double NearSolver::getFO(){
	return valoreFO;
}

double NearSolver::getTempoRisoluzione(){
	return tempoRisoluzione;
}

Soluzione* NearSolver::getSoluzione(){
	if (sol != NULL)
		return sol;
	sol = new Soluzione(ist);						// altrimenti crea soluzione di default
	return sol;
}


