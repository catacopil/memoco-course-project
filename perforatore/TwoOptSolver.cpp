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


void TwoOptSolver::risolvi(){
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
		int maxMoves = 5;
		
		while (!stop){
			// cerco delle mosse 2opt da attuare  (max 4-5?)
			trovato = false;
			TwoOptMove bestMove(ist, sol, -1, -1);
			double bestImprovement = 0;
			int moves = 0;
			
			for (int i=0; i<indiciSoluzione.size()-1 && moves < maxMoves; i++)
				for (int j=i+1; j<indiciSoluzione.size()-1 && moves < maxMoves; j++){
					if (i==j || j==i+1 || i==j+1) continue;
					TwoOptMove newMove(ist, sol, i, j);
					if (newMove.valida) moves++;
					if (newMove.valida && newMove.miglioramento > bestImprovement ){
						trovato = true;
						bestMove = newMove;
						bestImprovement = newMove.miglioramento;
						if (verbose) cout << " Nuovo BestImprovement trovato = "<< bestImprovement << endl;
						}
					}
			
			// scelgo tra le mosse trovate la migliore (guadagno più alto) e la attuo aggiornando la soluzione
			// devo aggiornare indiciSoluzione
			if (trovato){
				// ok, ho una nuova mossa da attuare
				
				int indiceA = bestMove.getPrimoSegmento();
				int indiceB = indiceA+1;
				int indiceC = bestMove.getSecondoSegmento();
				int indiceD = indiceC+1;
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
			if (count > maxOpt || !trovato){
				stop = true;
			}
			
		}






		/*

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
			} */
			
		if (verbose)	cout << " Eseguite "<< count << " two-opt";
		if (!trovato && verbose)		cout << " Non ho più trovato miglioramenti !";
	
		// creo oggetto Soluzione e aggiorno la funzione obiettivo
		//sol = new Soluzione(ist, &ordinati);
		valoreFO = sol->getFO();
		if (verbose) cout << " Soluzione = "<< valoreFO <<endl;
		
			
		//valoreFO = minFO;
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

/*
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
}  */


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

