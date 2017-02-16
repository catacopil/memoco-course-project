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

NearSolver::~NearSolver(){
//  ------	DISTRUTTORE
	delete distanze;
}


void NearSolver::risolvi(int nodoStart){	
//  ------- 	AVVIA IL SOLVER PER TROVARE LA SOLUZIONE

	try{
		vector<int> giainseriti;
		vector<vector<double>>* distanze = ist->getDistanze();
		vector<Punto>* nodiIstanza = ist->getNodi();
		vector<Punto> ordinati;
		
		int nodoinit, nodofine;
		nodoinit = nodoStart;
		nodofine = nodoStart+1;
		double minFO = DBL_MAX;
		int migliorStart;
		
		if (nodoStart == -1){			// controllo se devo partire da ogni singolo nodo per risolvere il problema
			nodoinit = 0;
			nodofine = nodiIstanza->size();
		}
		
		cout << "\n Inizia l'esecuzione del NearSolver...." << endl;
		chrono::high_resolution_clock::time_point inizio = chrono::high_resolution_clock::now();
		
		for (int start=nodoinit; start<nodofine; start++){
			int ultimoinserito = start;
			int ilminore = 0;
			int numeroNodi = ist->getN();
		
			ordinati.clear();		
			ordinati.push_back((*nodiIstanza)[ultimoinserito]); 				// inserisco il primo
			giainseriti.clear();
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
			if (sol) delete sol;
			sol = new Soluzione(ist, &ordinati);
			valoreFO = sol->getFO();
			if (valoreFO<minFO){
				minFO = valoreFO;
				migliorStart = start;
				}
			}
		valoreFO = minFO;
		chrono::high_resolution_clock::time_point fine = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::microseconds>(fine - inizio);
		double microsec = duration.count();
		tempoRisoluzione = microsec/1000000;
		if (tempoRisoluzione >= 60.0){
			int minuti = tempoRisoluzione/60;
			double secondi = fmod(tempoRisoluzione, 60);
			cout << " Problema risolto in "<< minuti <<" minuti e "<< secondi <<" secondi"<< endl;
			}
		else
			cout << " Problema risolto in "<< tempoRisoluzione <<" secondi"<< endl;
		
		cout << " Valore funzione obiettivo: " << valoreFO << " [start: "<< migliorStart <<"]"<<endl;		// stampa il valore della funzione obiettivo per la soluzione ottima
		
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


