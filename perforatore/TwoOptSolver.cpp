/*
*	Implementazione della classe TwoOptSolver
*
*/

#include "TwoOptSolver.h"
#include <iostream>



bool TwoOptSolver::verbose = false;

TwoOptSolver::TwoOptSolver(Istanza* i, int M=10000):Solver(i),maxOpt(M){			// il numero massimo di miglioramenti richiesti di default è 1000
//  -------	COSTRUTTORE SEMPLICE
	tempoRisoluzione = -1.0;
	valoreFO = -1.0;
	distanze = ist->getDistanze();
	
	if (verbose) cout << " Solver TwoOpt creato per l'istanza di richiesta \n";
}

TwoOptSolver::~TwoOptSolver(){
//  -------	DISTRUTTORE 
	delete distanze;	// elimino la matrice delle distanze
}


void TwoOptSolver::risolvi(int nodoStart){
//  ------- 	AVVIA IL SOLVER PER TROVARE LA SOLUZIONE

	try{
		vector<int> giainseriti;
		vector<vector<double>>* distanze = ist->getDistanze();
		vector<Punto>* nodiIstanza = ist->getNodi();
		
		int numeroNodi = ist->getN();
		
		int nodoinit, nodofine;
		nodoinit = nodoStart;
		nodofine = nodoStart+1;
		double minFO = DBL_MAX;
		int migliorStart;
		int migliorCount;
		
		if (nodoStart == -1){			// controllo se devo partire da ogni singolo nodo per risolvere il problema
			nodoinit = 0;
			nodofine = numeroNodi;
		}
		
		cout << "\n Inizia l'esecuzione del Solver TwoOpt...." << endl;
		chrono::high_resolution_clock::time_point inizio = std::chrono::high_resolution_clock::now();

		for (int start=nodoinit; start<nodofine; start++){
			int ultimoinserito = start;
			int ilminore = 0;
			// svuoto ordinati ( utile nel caso in cui lancio più volte il metodo risolvi )
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

			// ciclo esterno per il numero di iterazioni che voglio
			int count = 0;
			bool trovato = true;
			while (count < maxOpt && trovato){
				// cerco 2 punti per il tentativo di 2opt 
				trovato = false;
				for (int i=0; i<ordinati.size()-1; i++)
					for (int j=i+1; j<ordinati.size()-1; j++){
						if (i==j || j==i+1 || i==j+1) continue;
					
						// provo a vedere se la 2opt porterebbe benefici
						if (two_opt(i,j)){
							trovato = true;
							// se funziona allora faccio lo switch e modifico l'ordine dei punti di mezzo, incremento count anche
							switch_two_opt(i,j);
							count++;
							}
						}
				}
			if (verbose) cout << " Eseguite "<< count << " two-opt";
			if (!trovato)
				if (verbose) cout << " Non ho più trovato miglioramenti !";
		
			// creo oggetto Soluzione e aggiorno la funzione obiettivo
			sol = new Soluzione(ist, &ordinati);
			valoreFO = sol->getFO();
			if (verbose) cout << " Soluzione = "<< valoreFO <<endl;
			if (valoreFO<minFO){
				minFO = valoreFO;
				migliorStart = start;
				migliorCount = count;
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
		
		cout << " Valore funzione obiettivo per TwoOptSolver: " << valoreFO << " [start: "<< migliorStart <<" | count 2-opt:"<< migliorCount <<"]"<<endl<<endl;			// stampa il valore della funzione obiettivo per la soluzione ottima
		
	}
	catch (std::exception& e) {
		cout << ">>> Eccezione durante l'esecuzione del Solver TwoOpt: " << e.what() << endl;
	}
	//cout << " Problema risolto con il Solver Nearest Neighbor! \n";
}


bool TwoOptSolver::two_opt(int p1, int p2){
// ------- 	VERIFICA SE L'OPERAZIONE È FATTIBILE E PORTA UN MIGLIORAMENTO  (nuova versione)
	// p1 e p2 sono rispettivamente gli indici di nuovoPercorso e indicano 2 possibili punti A e C, tali che AB e CD possono essere sostituiti con AC e BD
	bool successo = false;
	
	Punto A = ordinati[p1];
	Punto B = ordinati[p1+1];
	Punto C = ordinati[p2];
	Punto D = ordinati[p2+1];
	
	if (A==C || B==D)
		return false;
	
	double attualeDist = A.distanza(&B) + C.distanza(&D);
	double nuovaVariante = A.distanza(&C) + B.distanza(&D);
	
	if (nuovaVariante < attualeDist){
		successo = true;
		//if (verbose) cout << " Trovata 2opt per i punti " << p1 << " e "<< p2 << endl;
		}
	return successo;
}

void TwoOptSolver::switch_two_opt(int i, int j){
// -------	ESEGUE LA TWO-OPT SUI NODI DEGLI INDICI PASSATI E SISTEMA L'ORDINE DEI NODI INTERMEDI
	// assumo che i<j sempre!
	if (i>=j)
		throw runtime_error(" Errore durante lo switch di due Punti in Two-Opt ----> i>=j! "); 
	int puntoA = i;
	int puntoB = i+1;
	int puntoC = j;
	int puntoD = j+1;
	Punto temp = ordinati[puntoB];						// scambio i punti interessati (sarebbero B e C)
	ordinati[puntoB] = ordinati[puntoC];
	ordinati[puntoC] = temp;
	
	// inverto l'ordine per i punti compresi tra puntoB e puntoC
 	int k2 = puntoC-1;
 	for (int k = puntoB+1; k<k2; k++){
 		temp = ordinati[k];
 		ordinati[k] = ordinati[k2];
 		ordinati[k2] = temp;
 		k2--;
 		}
}


double TwoOptSolver::getFO(){
	return valoreFO;
}


double TwoOptSolver::getTempoRisoluzione(){
	return tempoRisoluzione;
}


Soluzione* TwoOptSolver::getSoluzione(){
	if (sol != NULL)
		return sol;
	sol = new Soluzione(ist);						// altrimenti crea soluzione di default
	return sol;
}

