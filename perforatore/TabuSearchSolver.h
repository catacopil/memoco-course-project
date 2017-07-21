/*
*	Classe TwoOptSolver: definisce un oggetto che risolve un problema TSP con una soluzione iniziale Nearest Neighbor migliorata ulteriormente con l'algoritmo di ricerca locale 2-opt 
*	Contiene un puntatore ad un oggetto Istanza e ad un oggetto soluzione creato dopo aver risolto il problema
*	
*/


#ifndef TABU_SEARCH_SOLVER_H
#define TABU_SEARCH_SOLVER_H

#include <iostream>
#include <string>
#include <math.h>
#include <chrono>
#include <float.h>
#include <algorithm>
#include "solver.h"
#include "punto.h"
#include "TwoOptMove.h"

using namespace std;


class TabuSearchSolver: public Solver{
public:
	static bool verbose;								// indica se stampare sulla console messaggi di log sull'avanzamento del programma

	TabuSearchSolver(Istanza*, int, int);							// Creazione del solver, collegandolo con l'oggetto Istanza che deve risolvere, richiede la lunghezza della tabuList e il numero massimo di iterazioni
	
	void risolvi(double);								// avvia il solver con un determinato limite di tempo di esecuzione (rispetta anche maxIter)
	Soluzione* getSoluzione();							// ritorna la Soluzione
	double getTempoRisoluzione();							// ritorna il tempo (in secondi) impiegato per la risoluzione, se non ancora risolto ritorna -1
	double getFO();									// ritorna il valore della Funzione Obiettivo, se non ancora risolto ritorna -1
	
 	~TabuSearchSolver();

private:
	double tempoRisoluzione;
	double valoreFO;
	const int maxIter;									// il massimo numero di iterazioni consentite
	vector<int> tabuList;								// implementazione della TabuList, contiene un valore (quando il punto è stato utilizzato per una 2-opt) per ogni punto dell'istanza
	int tabuListLenght;									// lunghezza della TabuList (quante iterazioni considerare per dire se una mossa è tabu o meno)

};


#endif // TABU_SEARCH_SOLVER_H
