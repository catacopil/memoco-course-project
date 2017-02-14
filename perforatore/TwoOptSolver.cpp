/*
*	Implementazione della classe NearSolver
*
*/

#include "TwoOptSolver.h"
#include <iostream>



bool TwoOptSolver::verbose = true;

TwoOptSolver::TwoOptSolver(Istanza* i):Solver(i){
//  -------	COSTRUTTORE SEMPLICE
	tempoRisoluzione = -1.0;
	valoreFO = -1.0;
	distanze = ist->getDistanze();
	
	if (verbose) cout << " Solver TwoOpt creato per l'istanza richiesta \n";
}


void TwoOptSolver::risolvi(int start){
//  ------- 	AVVIA IL SOLVER PER TROVARE LA SOLUZIONE

	try{
		vector<int> giainseriti;
		vector<vector<double>>* distanze = ist->getDistanze();
		vector<Punto>* nodiIstanza = ist->getNodi();
		vector<Punto> ordinati;
		int ultimoinserito = start;
		int ilminore = 0;
		int numeroNodi = ist->getN();
		cout << "\n Inizia l'esecuzione del Solver TwoOpt...." << endl;
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
		// NUOVO METODO PER LA GESTIONE DI TWO-OPT
		// creo la mia lista di punti ordinati
		nuovoPercorso = ordinati;
		// ciclo esterno per il numero di iterazioni che voglio
		/*int count = 0;
		while (count < 1000){
			// cerco 2 punti per il tentativo di 2opt 
			// TODO: da sistemare il range di movimento
			for (int i=0; i<nuovoPercorso.size()-1; i++)
				for (int j=i+1; j<nuovoPercorso.size()-1; j++){
					// provo a vedere se la 2opt porterebbe benefici
					if (two_opt(i,j)){
						
						}
					
					// se funziona allora faccio lo switch e modifico l'ordine dei punti di mezzo, incremento count anche 
					
					
					// sistemo gli indici i e j ? ha senso
					
					
					}
			}
		*/
		
		costruisciSegmenti(ordinati);
		double FOiniziale = calcolaTotSegmenti();
		cout << " FOiniziale = "<<FOiniziale<<endl;
		double media = mediaSegmenti();
		cout << " Media = "<<media;
		media = media - (media/3);		// abbasso la media richiesta
		media = 0.1;
		cout << " ---> Media = "<<media<<endl;
		
		// costruisco vector contenente i segmenti lunghi più della media
		vector<Segmento> lunghi;
		for (int j=0; j<segmentiSoluzione->size(); j++)
			if ((*segmentiSoluzione)[j].getLunghezza()>media)
				lunghi.push_back((*segmentiSoluzione)[j]);
		cout << " Creato vector con segmenti più lunghi: "<<lunghi.size()<<" segmenti rispetto al totale di "<< segmentiSoluzione->size() <<endl;
		// considero le possibili 2opt per 10 coppie di segmenti lunghi 
		int count = 0;
		int s1 = 0;
		int s2 = 0;			// prendo i segmenti normali
		bool fermati = false;
		for (int i=0; !fermati && (count<1000); i++){
			
			
			if (s1!=s2 && two_opt(s1, s2)){
				count++;
				cout << " count: "<< count <<endl;
				stampaSegmenti();
				}
			if (s2<segmentiSoluzione->size())
				s2++;
			else{
				if (s1<lunghi.size())
					s1++;
				else{
					fermati = true;			// fermo la ricerca di 2opt
					cout << " Ho provato tutte le possibili 2opt! "<<endl;
					}
				s2 = s1+1;
				}
			
		}
		
		valoreFO = calcolaTotSegmenti();
		
		
		
		
		
		
		
		// creo oggetto Soluzione e aggiorno la funzione obiettivo
		//sol = new Soluzione(ist, &ordinati); TODO: da sistemare qua la Soluzione
		//valoreFO = sol->getFO();
		
		
		tempo = clock() - tempo;
		tempoRisoluzione = ((float)tempo/CLOCKS_PER_SEC);
		if (tempoRisoluzione >= 60.0){
			int minuti = tempoRisoluzione/60;
			double secondi = fmod(tempoRisoluzione, 60);
			cout << "\n Problema risolto in "<< tempo <<" clocks ("<< minuti <<" minuti e "<< secondi <<" secondi)"<< endl;
			}
		else
			cout << "\n Problema risolto in "<< tempo <<" clocks ("<< tempoRisoluzione <<" secondi)"<< endl;
		
		cout << " Valore funzione obiettivo per TwoOptSolver: " << valoreFO << " [start: "<< start <<" | count:"<< count <<"]"<<endl<<endl;			// stampa il valore della funzione obiettivo per la soluzione ottima
		
	}
	catch (std::exception& e) {
		cout << ">>> Eccezione durante l'esecuzione del Solver TwoOpt: " << e.what() << endl;
	}
	//cout << " Problema risolto con il Solver Nearest Neighbor! \n";
}

void TwoOptSolver::costruisciSegmenti(vector<Punto> &ordinati){
	segmentiSoluzione = new vector<Segmento>;
	for (int i=0; i<ordinati.size(); i++){				// scorro tutti i Punti della soluzione inserendo per ciascuno di essi un segmento
		Punto A = ordinati[i];
		Punto B = A;
		if (i!=ordinati.size()-1)			// vedo se c'è il prossimo, altrimenti prendo il Punto iniziale
			B = ordinati[i+1];
		else 
			B = ordinati[0];
		segmentiSoluzione->push_back(Segmento(A,B));
		}
}

double TwoOptSolver::mediaSegmenti(){
//  --------	RITORNA LA LUNGHEZZA MEDIA DEI SEGMENTI
	double m = 0;
	for (int i=0; i<segmentiSoluzione->size(); i++)
		m += (*segmentiSoluzione)[i].getLunghezza();
	return m/segmentiSoluzione->size();
}


bool TwoOptSolver::two_opt(int p1, int p2){
// ------- 	VERIFICA SE L'OPERAZIONE È FATTIBILE E PORTA UN MIGLIORAMENTO  (nuova versione)
	// p1 e p2 sono rispettivamente gli indici di nuovoPercorso e indicano 2 possibili punti A e C, tali che AB e CD possono essere sostituiti con AC e BD
	bool successo = false;
	/*
	Punto A = nuovoPercorso[p1];
	Punto B = nuovoPercorso[p1+1];
	Punto C = nuovoPercorso[p2];
	Punto D = nuovoPercorso[p2+1];
	*/

//  --------	TENTA L'OPERAZIONE 2OPT SUI DUE SEGMENTI  (se l'operazione ha successo ritorna true altrimenti false)
	//if (verbose) 	cout << " Tentativo 2opt per segmenti "<<s1<< " e "<<s2<<endl;
	
	Segmento seg1 = (*segmentiSoluzione)[s1];
	Segmento seg2 = (*segmentiSoluzione)[s2];
	Punto A = seg1.getDa();
	Punto B = seg1.getA();
	Punto C = seg2.getDa();
	Punto D = seg2.getA();
	
	// controllo che non vengano generati segmenti con agli estremi gli stessi punti (sarebbe un cappio)
	if (A==C || B==D)
		return false;
	
	double nuovaVariante = A.distanza(&C) + B.distanza(&D);
	if (nuovaVariante < (seg1.getLunghezza()+seg2.getLunghezza())){
		successo = true;
		// inserisco i due nuovi segmenti al posto degli altri
		(*segmentiSoluzione)[s1] = Segmento(A,C);
		(*segmentiSoluzione)[s2] = Segmento(B,D);
		
		if (verbose) cout << " Eseguita operazione 2-opt per i segmenti di indice "<< s1 <<" e "<< s2 <<endl;
		
		
		// cambio di "direzione" i segmenti compresi tra gli indici s1 e s2 (questo garantisce che i nuovi segmenti siano composti sempre da AC e BD)
		// assumo che s1<s2 sempre!
		for (int i=s1+1; i<s2; i++){
			(*segmentiSoluzione)[i].gira();
			cout << " Giro seg "<<i<<" \t";
			}
		}
	return successo;
}


double TwoOptSolver::calcolaTotSegmenti(){
	double tot = 0.0;
	for (int i=0; i<segmentiSoluzione->size(); i++)
		tot += (*segmentiSoluzione)[i].getLunghezza();
	return tot;
}

void TwoOptSolver::stampaSegmenti(){
	for (int i=0; i<segmentiSoluzione->size(); i++)
		cout << "["<< i <<"]"<< (*segmentiSoluzione)[i].stampa()<<endl;
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

