/*
*	Classe TwoOptSolver: definisce un oggetto che risolve un problema TSP con una soluzione iniziale Nearest Neighbor migliorata ulteriormente con l'algoritmo di ricerca locale 2-opt 
*	Contiene un puntatore ad un oggetto Istanza e ad un oggetto soluzione creato dopo aver risolto il problema
*	
*/


#ifndef TWO_OPT_SOLVER_H
#define TWO_OPT_SOLVER_H

#include <iostream>
#include <string>
#include <math.h>
#include <time.h>
#include <float.h>
#include <algorithm>
#include "solver.h"
#include "punto.h"
#include "segmento.h"

using namespace std;


class TwoOptSolver: public Solver{
public:
	static bool verbose;								// indica se stampare sulla console messaggi di log sull'avanzamento del programma

	TwoOptSolver(Istanza*);								// Creazione del solver, collegandolo con l'oggetto Istanza che deve risolvere, viene passato anche il parametro K
	
	void risolvi(int);									// risolve il problema utilizzando l'euristica Nearest Neighbor e poi ottimizza utilizzando un determinato numero di azioni 2-opt
	Soluzione* getSoluzione();							// ritorna la Soluzione
	double getTempoRisoluzione();							// ritorna il tempo (in secondi) impiegato per la risoluzione, se non ancora risolto ritorna -1
	double getFO();									// ritorna il valore della Funzione Obiettivo, se non ancora risolto ritorna -1
	
// 	~MioSolver();		TODO: SERVE ??

private:
	double tempoRisoluzione;
	double valoreFO;
	vector<Punto> nuovoPercorso;
	vector<vector<double>>* distanze;						// matrice delle distanze
	vector<Segmento>* segmentiSoluzione;					// contiene i segmenti dell'attuale soluzione
	void costruisciSegmenti(vector<Punto>&);							// popola il vector segmentiSoluzione
	double mediaSegmenti();								// calcola la lunghezza media dei segmenti
	bool two_opt(int, int);								// Tentativo di 2-opt per 2 segmenti trovati
	double calcolaTotSegmenti();							// Calcola la FO sommando le lunghezze dei segmenti attuali
	void stampaSegmenti();								// stampa a video gli oggetti di segmentiSoluzione
};


#endif // TWO_OPT_SOLVER_H
