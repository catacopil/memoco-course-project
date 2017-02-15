/*
*	Implementazione della classe NearSolver
*
*/

#include "Miosolver.h"
#include <iostream>



bool MioSolver::verbose = true;

MioSolver::MioSolver(Istanza* i, int kappa):Solver(i),maxK(kappa){
//  -------	COSTRUTTORE SEMPLICE
	tempoRisoluzione = -1.0;
	valoreFO = -1.0;
	distanze = ist->getDistanze();
	
	if (verbose) cout << " MioSolver creato per l'istanza richiesta \n";
}

MioSolver::~MioSolver(){
//  -------	DISTRUTTORE
	delete distanze;
}


void MioSolver::risolvi(int start){	
//  ------- 	AVVIA IL SOLVER PER TROVARE LA SOLUZIONE

	try{
		vector<int> giainseriti;												// conterrà gli indici dei punti inseriti nel ciclo hamiltoniano
		vector<vector<double>>* distanze = ist->getDistanze();						// matrice delle distanze tra i punti
		vector<Punto>* nodiIstanza = ist->getNodi();								// vector che contiene i punti dell'istanza
		vector<Punto> ordinati;												// vector che contiene i punti ordinati 
		int ultimoinserito = start;
		int ilminore = 0;
		int numeroNodi = ist->getN();
		cout << "\n Inizia l'esecuzione del MioSolver...." << endl;
		chrono::high_resolution_clock::time_point inizio = chrono::high_resolution_clock::now();
		
		ordinati.push_back((*nodiIstanza)[ultimoinserito]); 				// inserisco il primo
		giainseriti.push_back(start);
		int limiteEuristicaNearest = 4;								// il numero di nodi da inserire con l'euristica Nearest Neighbor
		int K = maxK;
		
		
		for (int i=1; i<numeroNodi; i++){
			// parto con i=1 perché il primo l'ho già inserito e aggiungo i primi K seguendo l'euristica Nearest Neighbor
			double minDist = DBL_MAX;
			for (int j=0; j<numeroNodi; j++){				// scelgo il nodo più vicino 
				if (ultimoinserito==j) continue;			// va avanti se il nodo è lo stesso
				bool piupiccolo = (minDist > (*distanze)[ultimoinserito][j]);
				bool esistegia = (find(giainseriti.begin(), giainseriti.end(), j) != giainseriti.end());		// controllo se già presente
				if (piupiccolo && !esistegia){			// se ne ho trovato uno più piccolo che non è ancora nella soluzione allora aggiorno come prossimo punto scelto
					ilminore = j;
					minDist = (*distanze)[ultimoinserito][j];
					}
				}
			if (minDist == DBL_MAX)						// non ho trovato un minimo
				throw runtime_error(" Non ho trovato una distanza minima, minore a DBL_MAX! "); 
			//cout <<"->"<< i <<") Trovato il punto " << (*nodiIstanza)[ilminore].stampa()<<endl;
			
			if (i<maxK)
				K = i-1;				// imposto K in base al numero di nodi presenti in ordinati
			else 
				K = maxK;
			
			if (ordinati.size()<=limiteEuristicaNearest){ 	
			// INSERISCO I PRIMI K SENZA EFFETTUARE SCAMBI
				// lo aggiungo a ordinati, aggiorno gli indici di giainseriti e Kprecedenti
				ordinati.push_back((*nodiIstanza)[ilminore]);
				giainseriti.push_back(ilminore);
				ultimoinserito = ilminore;
				}
			else{
			// ALGORITMO ITERATIVO CHE RIORDINA GLI ULTIMI K PER OGNI INSERIMENTO
				// trovo i K punti inseriti più vicini a quello da inserire e li metto in Kvicini (in ordine decrescente)
				Punto daAggiungere = (*nodiIstanza)[ilminore];
				// cout << " Il prossimo da aggiungere è "<< daAggiungere.stampa() <<endl;
				int Kvicini[K]{-1};
				for (int j=0; j<K; j++)			// inserisco i primi K in Kvicini
					Kvicini[j] = j;
				// ordine crescente per distanza dal punto daAggiungere
				for (int f=0; f<K-1; f++)
					for (int j=f; j<K-1; j++)
						if (ordinati[Kvicini[j]].distanza(&daAggiungere) > ordinati[Kvicini[j+1]].distanza(&daAggiungere)){
							// faccio lo scambio degli indici dentro Kvicini se non sono in ordine decrescente
							int temp = Kvicini[j];
							Kvicini[j] = Kvicini[j+1];
							Kvicini[j+1] = temp;
						}
				
				// cerco di inserire altri punti mantenendo l'ordine crescente
				for (int j=K; j<ordinati.size(); j++){
					bool inserito = false;
					int temp, next;
					for (int m=0; m<K; m++){
						if (!inserito && (ordinati[j].distanza(&daAggiungere) < ordinati[Kvicini[m]].distanza(&daAggiungere)) ){
							// inserisco il nuovo indice nella posizione
							next = Kvicini[m];
							Kvicini[m] = j;
							inserito = true;
							continue;
							}
						if (inserito){
							temp = Kvicini[m];
							Kvicini[m] = next;
							next = temp;
							}
						}
					}
				//  STAMPA dell'array Kvicini
				/*
				cout << " Kvicini: ";
				for (int j=0; j<K; j++){
					double d = ordinati[Kvicini[j]].distanza(&daAggiungere);
					cout << Kvicini[j] << " (dist: "<< d << ") | ";
					if (d==0) {
						cout << " Trovata distanza 0 tra punto di indice "<< Kvicini[j] <<" e "<< ilminore <<endl;
						cout << " Punto " <<ordinati[Kvicini[j]].stampa() << " e "<< daAggiungere.stampa()<<endl;
						throw runtime_error(" Trovata distanza 0 tra punti!");
						}
					}
				cout << endl; */
				
				// CALCOLO LE DISTANZE DEI POSSIBILI PERCORSI PER TUTTI I VALORI DI Kvicini
				vector<Soluzione*> percorsi;
				for (int j=0; j<K; j++){
					vector<Punto> percorso = ordinati;					// una copia dell'attuale percorso
					vector<Punto>::iterator it = percorso.begin();
					for (int c=0; c<Kvicini[j]+1; c++)
						it++;
					percorso.insert(it, daAggiungere);
					Soluzione* s = new Soluzione(ist, &percorso);
					percorsi.push_back(s);
					}
				
				// SCELTA DEL PUNTO MIGLIORE DOPO IL QUALE INSERIRE IL PROSSIMO PUNTO (daAggiungere)
				int migliorPunto = 0;				// indica la posizione in Kvicini dove trovare il miglior punto
				double migliorValore = percorsi[0]->getFO();
				// cout << " Valore percorso temporaneo con il punto "<<Kvicini[0]<<" ==> "<< percorsi[0]->getFO() <<endl;
				for (int j=1; j<K; j++){
					// cout << " Valore percorso temporaneo con il punto "<<Kvicini[j]<<" ==> "<< percorsi[j]->getFO() <<endl;
					if (percorsi[j]->getFO()<migliorValore){
						migliorPunto = j;
						migliorValore = percorsi[j]->getFO();
						}
					}
				if (migliorValore == 0)
					throw runtime_error(" Errore nel calcolo del sotto-percorso migliore! "); 
					
				// CANCELLO LE SOLUZIONI AGGIUNTIVE CREATE
				for (int j=0; j<percorsi.size(); j++)
					delete percorsi[j];
				
				// INSERISCO IL NUOVO PUNTO NEL POSTO GIUSTO IN ordinati
				vector<Punto>::iterator it = ordinati.begin();
				for (int c=0; c < Kvicini[migliorPunto]+1; c++)
					it++;
				ordinati.insert(it, daAggiungere);
				giainseriti.push_back(ilminore);
				ultimoinserito = ilminore;
				// cout << " Inserito il punto di indice "<< i <<" subito dopo il punto di indice "<< Kvicini[migliorPunto] <<endl;
				}
			
			}
		
		sol = new Soluzione(ist, &ordinati);					// creo oggetto Soluzione e aggiorno la funzione obiettivo
		valoreFO = sol->getFO();
		
		// calcolo del tempo impiegato
		chrono::high_resolution_clock::time_point fine = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::milliseconds>(fine - inizio);
		double millisec = duration.count();
		tempoRisoluzione = millisec/1000;
		if (tempoRisoluzione >= 60.0){
			int minuti = tempoRisoluzione/60;
			double secondi = fmod(tempoRisoluzione, 60);
			cout << " Problema risolto in "<< minuti <<" minuti e "<< secondi <<" secondi"<< endl;
			}
		else
			cout << " Problema risolto in "<< tempoRisoluzione <<" secondi"<< endl;
		
		cout << " Valore funzione obiettivo per MioSolver: " << valoreFO << endl;			// stampa il valore della funzione obiettivo per la soluzione ottima
		
	}
	catch (std::exception& e) {
		cout << ">>> Eccezione durante l'esecuzione del Mio Solver: " << e.what() << endl<<endl;
	}
	//if (verbose) cout << " Problema risolto con il MioSolver! \n";
}

double MioSolver::getFO(){
	return valoreFO;
}

double MioSolver::getTempoRisoluzione(){
	return tempoRisoluzione;
}

Soluzione* MioSolver::getSoluzione(){
	if (sol != NULL)
		return sol;
	sol = new Soluzione(ist);						// altrimenti crea soluzione di default
	return sol;
}


