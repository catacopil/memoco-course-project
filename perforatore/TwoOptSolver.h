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
#include <chrono>
#include <float.h>
#include <algorithm>
#include "solver.h"
#include "punto.h"

using namespace std;


class TwoOptSolver: public Solver{
public:
	static bool verbose;								// indica se stampare sulla console messaggi di log sull'avanzamento del programma

	TwoOptSolver(Istanza*, int);							// Creazione del solver, collegandolo con l'oggetto Istanza che deve risolvere, richiede il numero massimo di miglioramenti desiderato (di default è 1000)
	
	void risolvi(int);									// risolve il problema utilizzando l'euristica Nearest Neighbor e poi ottimizza utilizzando un determinato numero di azioni 2-opt
	Soluzione* getSoluzione();							// ritorna la Soluzione
	double getTempoRisoluzione();							// ritorna il tempo (in secondi) impiegato per la risoluzione, se non ancora risolto ritorna -1
	double getFO();									// ritorna il valore della Funzione Obiettivo, se non ancora risolto ritorna -1
	
 	~TwoOptSolver();

private:
	double tempoRisoluzione;
	double valoreFO;
	const int maxOpt;									// il massimo per di miglioramenti desiderati
	vector<Punto> ordinati;								// vector di punti utilizzato per ordinare i nodi dell'istanza
	vector<vector<double>>* distanze;						// matrice delle distanze
	
	bool two_opt(int, int);								// Tentativo di 2-opt per 2 segmenti trovati
	void switch_two_opt(int, int);						// Esegue l'operazione 2-opt per i 2 segmenti 

};


#endif // TWO_OPT_SOLVER_H
