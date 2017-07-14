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
	
	if (verbose) cout << " Solver Nearest Neighbor creato per l'istanza richiesta \n";
}

NearSolver::~NearSolver(){
//  ------	DISTRUTTORE 	
}


void NearSolver::risolvi(int nodoStart){	
//  ------- 	AVVIA IL SOLVER PER TROVARE LA SOLUZIONE 	

	try{
		int nodoinit, nodofine;
		nodoinit = nodoStart;
		nodofine = nodoStart+1;
		double minFO = DBL_MAX;
		int migliorStart;
		int N = ist->getN();
		
		if (nodoStart == -1){			// controllo se devo partire da ogni singolo nodo per risolvere il problema
			nodoinit = 0;
			nodofine = N;
		}
		
		cout << "\n Inizia l'esecuzione del NearSolver...." << endl;
		chrono::high_resolution_clock::time_point inizio = chrono::high_resolution_clock::now();
		
		for (int start=nodoinit; start<nodofine; start++){
			int ultimoinserito = start;
			int ilminore = 0;
			
			vector<Punto> &nodiIstanza = *(ist->getNodi());		// vector contenente i punti non ancora scelti per la soluzione
			vector<short> indiciNodi;						// vector contenente gli indici (in istanza) dei punti dentro nodiIstanza
			vector<short> indiciSoluzione;				// vector contenente gli indici (in istanza) della soluzione
			
			for (short k=0; k<N; k++)
				indiciNodi.push_back(k);

			// inserisco il primo punto della soluzione
			Punto ultimoPuntoInserito = nodiIstanza[ultimoinserito];
			nodiIstanza.erase(nodiIstanza.begin()+ultimoinserito);
			indiciSoluzione.push_back(indiciNodi[ultimoinserito]);
			indiciNodi.erase(indiciNodi.begin()+ultimoinserito);
			
			for (int i=1; i<N; i++){
				// parto con i=1 perché il primo l'ho già inserito e faccio girare il ciclo N-1 volte 
				// scelgo il prossimo nodo
				double minDist = DBL_MAX;
				
				for (int j=0; j<nodiIstanza.size(); j++){
					double distanzaCorrente = ultimoPuntoInserito.distanza(&nodiIstanza[j]);
					if (minDist > distanzaCorrente){
						minDist = distanzaCorrente;
						ilminore = j;
					}
				}
				//cout << "For: i=" << i <<" ho scelto di inserire il "<< ilminore <<endl;
				if (minDist == DBL_MAX)				// non ho trovato un minimo, c'è qualcosa di sbagliato!
					throw runtime_error(" Non ho trovato una distanza minima, minore a DBL_MAX! "); 
				// tolgo il miglior nodo da nodiIstanza e aggiungo il suo indice alla soluzione
				ultimoPuntoInserito = nodiIstanza[ilminore];
				nodiIstanza.erase(nodiIstanza.begin()+ilminore);
				indiciSoluzione.push_back(indiciNodi[ilminore]);
				indiciNodi.erase(indiciNodi.begin()+ilminore);
			}
		
			// aggiorno la funzione obiettivo e la soluzione se migliore
			Soluzione* ultimaSol = new Soluzione(ist, &indiciSoluzione);
			double ultimaFO = ultimaSol->getFO();
			if (ultimaFO < minFO){
				minFO = ultimaFO;
				if (sol) delete sol;
				sol = ultimaSol;
				migliorStart = start;
				}
			else 
				delete ultimaSol;
			}
		valoreFO = minFO;
		// calcola e stampa il tempo impiegato 
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


