/*
*	Implementazione della classe TwoOptSolver
*
*/

#include "TabuSearchSolver.h"
#include <iostream>



bool TabuSearchSolver::verbose = false;

TabuSearchSolver::TabuSearchSolver(Istanza* i, int L=7, int M=100):Solver(i), maxIter(M), tabuListLenght(L){	
//  -------	COSTRUTTORE SEMPLICE
	tempoRisoluzione = -1.0;
	valoreFO = -1.0;
	
	if (verbose) cout << " Solver Tabu Search creato per l'istanza di richiesta \n";
}

TabuSearchSolver::~TabuSearchSolver(){
//  -------	DISTRUTTORE 
}


void TabuSearchSolver::risolvi(double timeLimit){
//  ------- 	AVVIA IL SOLVER

	try{
		cout << "\n Inizia l'esecuzione del Solver Tabu Search...." << endl;
		chrono::high_resolution_clock::time_point inizio = std::chrono::high_resolution_clock::now();
		
		
	// ----------  CREAZIONE SOLUZIONE INIZIALE 		-------------
		int N = ist->getN();
		double minFO = DBL_MAX;							// il valore della migliore soluzione trovata, inizializzato a DBL_MAX
		int primoinserito = 0;
		int ilminore = 0;
		
		for (int i=0; i<N; i++)							// inizializzazione tabuList
			tabuList.push_back(-tabuListLenght-1);
			
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
		
		while (!stop){
			// cerco delle mosse 2opt da attuare  (max 4-5?)
			trovato = false;
			TwoOptMove bestMove(ist, sol, -1, -1);
			double bestImprovement = -DBL_MAX;
			
			for (int i=0; i<indiciSoluzione.size()-1 && !stop; i++){
				for (int j=i+1; j<indiciSoluzione.size()-1; j++){
					if (i==j || j==i+1 || i==j+1) continue;
					/*
					if ((count - tabuList[sol->getIndicePunto(i+1)]<= tabuListLenght) 
						&& (count - tabuList[sol->getIndicePunto(j)]<= tabuListLenght)){ 		// controllo se è una mossa tabu
						cout << sol->getIndicePunto(i+1) << " e "<< sol->getIndicePunto(j) << " sono ancora Tabu! \n";
						continue;	
						} */
					TwoOptMove newMove(ist, sol, i, j);
					if ( newMove.valida && (count - tabuList[sol->getIndicePunto(i+1)]<= tabuListLenght) 
						&& (count - tabuList[sol->getIndicePunto(j)]<= tabuListLenght)){ 		// controllo se è una mossa tabu
						//cout << sol->getIndicePunto(i+1) << " e "<< sol->getIndicePunto(j) << " sono ancora Tabu! \n";
						continue;	
						}
						
					if (newMove.valida && newMove.miglioramento > bestImprovement ){
						trovato = true;
						bestMove = newMove;
						bestImprovement = newMove.miglioramento;
						if (verbose) cout << " Nuovo BestImprovement trovato = "<< bestImprovement << endl;
						}
					}
				// ---------  	CRITERI DI ARRESTO
				chrono::high_resolution_clock::time_point t_now = chrono::high_resolution_clock::now();
				auto duration = chrono::duration_cast<chrono::microseconds>(t_now - inizio);
				double tempoImpiegato = duration.count();			// i microsecondi impiegati finora
				tempoImpiegato = tempoImpiegato/1000000;
				if (tempoImpiegato > timeLimit){
					stop = true;
					trovato = false;
					cout << " Tempo scaduto. Fermo il Solver \n";
					}
				}
			
			// scelgo tra le mosse trovate la migliore (guadagno più alto) e la attuo aggiornando la soluzione
			// devo aggiornare indiciSoluzione
			if (trovato){			// ok, ho una nuova mossa da attuare
				int indiceA = bestMove.getPrimoSegmento();
				int indiceB = indiceA+1;
				int indiceC = bestMove.getSecondoSegmento();
				if (indiceA >= indiceC)
					throw runtime_error(" Errore durante lo switch di due Punti in TabuSearch ----> indiceA >= indiceC! "); 
				
				// aggiornamento della tabuList
				count++;
				tabuList[sol->getIndicePunto(indiceB)] = count;
				tabuList[sol->getIndicePunto(indiceC)] = count;
				if (verbose) cout << count << " add alle mosse tabu: "<< sol->getIndicePunto(indiceB) << " e " << sol->getIndicePunto(indiceC) <<endl;
				
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
			 	// aggiorno la soluzione
			 	delete sol;
			 	sol = new Soluzione(ist, &indiciSoluzione);
			 	
			 	if (verbose) cout << " Soluzione aggiornata: "<<sol->getFO()<< " | count = "<< count <<endl<<endl;
			}
			
			// ---------  	CRITERI DI ARRESTO
			
			if (count >= maxIter || !trovato){
				stop = true;
				if (count >= maxIter)
					cout << " Raggiunto il limite massimo di operazioni. count = " << count <<endl; 
			}
		}

			
		if (verbose)	cout << " Eseguite "<< count << " two-opt";
		if (!trovato && verbose)		cout << " Non ho più trovato miglioramenti !\n";
	
		// aggiorno la funzione obiettivo
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
		
		cout << " Valore funzione obiettivo per TabuSearchSolver: " << valoreFO << " | count 2-opt:"<< count <<"]"<<endl<<endl;			// stampa il valore della funzione obiettivo per la soluzione ottima
		
	}
	catch (std::exception& e) {
		cout << ">>> Eccezione durante l'esecuzione del Solver TabuSearch: " << e.what() << endl;
	}
	//cout << " Problema risolto con il Solver Nearest Neighbor! \n";
}


double TabuSearchSolver::getFO(){
	return valoreFO;
}


double TabuSearchSolver::getTempoRisoluzione(){
	return tempoRisoluzione;
}


Soluzione* TabuSearchSolver::getSoluzione(){
	if (sol != NULL)
		return sol;
	sol = new Soluzione(ist);						// altrimenti crea soluzione di default
	return sol;
}

