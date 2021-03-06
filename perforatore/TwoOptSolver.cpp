/*
*	Implementazione della classe TwoOptSolver
*
*/

#include "TwoOptSolver.h"
#include <iostream>



bool TwoOptSolver::verbose = false;

TwoOptSolver::TwoOptSolver(Istanza* i, int M=100):Solver(i),maxOpt(M){			// il numero massimo di miglioramenti richiesti di default è 1000
//  -------	COSTRUTTORE SEMPLICE
	tempoRisoluzione = -1.0;
	valoreFO = -1.0;
	
	if (verbose) cout << " Solver TwoOpt creato per l'istanza di richiesta \n";
}

TwoOptSolver::~TwoOptSolver(){
//  -------	DISTRUTTORE 
}


void TwoOptSolver::risolvi(double timeLimit){
//  ------- 	AVVIA IL SOLVER PER TROVARE LA SOLUZIONE

	try{
		cout << "\n Inizia l'esecuzione del Solver TwoOpt...." << endl;
		chrono::high_resolution_clock::time_point inizio = std::chrono::high_resolution_clock::now();
		
	// ----------  CREAZIONE SOLUZIONE INIZIALE 		-------------
		int N = ist->getN();
		double minFO = DBL_MAX;							// il valore della migliore soluzione trovata, inizializzato a DBL_MAX
		int primoinserito = 0;
		int ilminore = 0;
			
		vector<Punto> &nodiIstanza = *(ist->getNodi());		// vector contenente i punti non ancora scelti per la soluzione
		vector<short> indiciNodi;						// vector contenente gli indici (in istanza) dei punti dentro nodiIstanza
		vector<short> indiciSoluzione;					// vector contenente gli indici (in istanza) della soluzione
		
		for (short k=0; k<N; k++)						// inizializzo gli indici del vettore indiciNodi
			indiciNodi.push_back(k);
		// inserisco il primo punto della soluzione
		Punto ultimoPuntoInserito = nodiIstanza[primoinserito];
		nodiIstanza.erase(nodiIstanza.begin()+primoinserito);
		indiciSoluzione.push_back(indiciNodi[primoinserito]);
		indiciNodi.erase(indiciNodi.begin()+primoinserito);
		
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
		if (sol==NULL) delete sol;
		sol = new Soluzione(ist, &indiciSoluzione);
		minFO = sol->getFO();

	// ----------	ITERAZIONE CON RICERCA NEL VICINATO

		bool stop = false;							// indica se devo fermarmi durante le iterazioni di ricerca
		bool trovato = false;						// indica se ho trovato qualche mossa 2-opt
		int count = 0;
		int maxMoves = 5;							// indica il numero di mosse 2-opt da esplorare prima di scegliere la migliore
		if (N > 300) maxMoves = 7;
		if (N > 2000) maxMoves = 10;
		
		while (!stop){
			// cerco delle mosse 2opt da attuare  (max 4-5?)
			trovato = false;
			TwoOptMove bestMove(ist, sol, -1, -1);
			double bestImprovement = 0;
			int moves = 0;
			
			for (int i=0; i<indiciSoluzione.size()-1 && moves < maxMoves && !stop; i++){
				for (int j=i+1; j<indiciSoluzione.size()-1 && moves < maxMoves; j++){
					if (i==j || j==i+1 || i==j+1) continue;
					TwoOptMove newMove(ist, sol, i, j);
					if (newMove.valida && newMove.miglioramento>0) moves++;
					if (newMove.valida && newMove.miglioramento > bestImprovement ){
						trovato = true;
						bestMove = newMove;
						bestImprovement = newMove.miglioramento;
						if (verbose) cout << " Nuovo BestImprovement trovato = "<< bestImprovement << endl;
						}
					}
				chrono::high_resolution_clock::time_point t_now = chrono::high_resolution_clock::now();
				auto duration = chrono::duration_cast<chrono::microseconds>(t_now - inizio);
				double tempoImpiegato = duration.count();			// i microsecondi impiegati finora
				tempoImpiegato = tempoImpiegato/1000000;
				if (tempoImpiegato > timeLimit){
					stop = true;
					trovato = false;
					}
				}
			
			// scelgo tra le mosse trovate la migliore (bestMove) e la applico aggiornando la soluzione
			// devo aggiornare indiciSoluzione
			if (trovato){
				// ok, ho una nuova mossa da attuare
				
				int indiceA = bestMove.getPrimoSegmento();
				int indiceB = indiceA+1;
				int indiceC = bestMove.getSecondoSegmento();
				if (indiceA >= indiceC)
					throw runtime_error(" Errore durante lo switch di due Punti in Two-Opt ----> indiceA >= indiceC! "); 
				short temp = indiciSoluzione[indiceB];						// scambio i punti interessati (sarebbero B e C)
				indiciSoluzione[indiceB] = indiciSoluzione[indiceC];
				indiciSoluzione[indiceC] = temp;
	
				// inverto l'ordine per i valori compresi tra puntoB e puntoC
			 	int k2 = indiceC-1;
			 	for (int k = indiceB+1; k<k2; k++){
			 		temp = indiciSoluzione[k];
			 		indiciSoluzione[k] = indiciSoluzione[k2];
			 		indiciSoluzione[k2] = temp;
			 		k2--;
			 		}
			 	// aggiorno la soluzione e il contatore
			 	delete sol;
			 	sol = new Soluzione(ist, &indiciSoluzione);
			 	count++;
			 	if (verbose) cout << " Soluzione aggiornata: "<<sol->getFO()<< " | count = "<< count <<endl<<endl;
			}
			
			// controllo se mi devo fermare o no
			chrono::high_resolution_clock::time_point t_now = chrono::high_resolution_clock::now();
			auto duration = chrono::duration_cast<chrono::microseconds>(t_now - inizio);
			double tempoImpiegato = duration.count();			// i microsecondi impiegati finora
			tempoImpiegato = tempoImpiegato/1000000;
			if (count >= maxOpt || !trovato || tempoImpiegato > timeLimit){
				stop = true;
				if (count >= maxOpt)
					cout << " Raggiunto il limite massimo di operazioni. count = " << count <<endl; 
				if (tempoImpiegato > timeLimit)
					cout << " Tempo scaduto. Fermo il Solver \n";
			}
			
		}
			
		if (verbose)	cout << " Eseguite "<< count << " two-opt";
		if (!trovato && verbose)		cout << " Non ho più trovato miglioramenti !";
	
		valoreFO = sol->getFO();
		if (verbose) cout << " Soluzione = "<< valoreFO <<endl;
		
		
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
		
		cout << " Valore funzione obiettivo per TwoOptSolver: " << valoreFO << " | count 2-opt:"<< count <<"]"<<endl<<endl;			// stampa il valore della funzione obiettivo per la soluzione ottima
		
	}
	catch (std::exception& e) {
		cout << ">>> Eccezione durante l'esecuzione del Solver TwoOpt: " << e.what() << endl;
	}
	//cout << " Problema risolto con il Solver Nearest Neighbor! \n";
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
