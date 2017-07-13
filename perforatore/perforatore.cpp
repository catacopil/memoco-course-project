#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <time.h>
#include "cpxmacro.h"
#include "istanza.h"
#include "soluzione.h"
#include "solver.h"
#include "CPLEXsolver.h"
#include "Nearsolver.h"
//#include "TwoOptSolver.h"

using namespace std;

bool verbose = true;				// indica se stampare sulla console messaggi di log sull'avanzamento del programma

int status;
char errmsg[BUF_SIZE];

int main(int argc, char const *argv[]) {
	
	// 	DICHIARAZIONE VARIABILI PER CPLEX
	DECL_ENV(env);
	DECL_PROB(env, lp);
	
	typedef std::chrono::duration<double> secondi;

	int numeroNodi=0;						// numero di nodi del problema
	string fileName;						// nome del file che contiene l'istanza del problema
	ifstream lettoreIstanza;
	if (argc != 2){
		cout << " Errore: argomenti non sufficienti, argc= "<< argc << " | " ;
		for(int i=0; i<argc; i++)
			cout << argv[i]<< " ";
		cout << "\n Il corretto avvio del perforatore necessita di un file contenente un'istanza o un intero per la dimensione dell'istanza da creare\n";
		cout << " Esempi: \n \t perforatore util/istanzaG57.txt  \n \t perforatore 100 \n";
		return 0;
	}
	try{
		numeroNodi = stoi(argv[1]);
	}
	catch (std::exception& e) {
		cout << " Creazione istanza da file... "<< endl;
		try{
            fileName = argv[1];
		}
		catch (std::exception& e) {
			cout << ">>> Eccezione durante lettura parametro stringa " << e.what() << std::endl;
			return 0;
		}
	}
	Istanza* ist;
	if (numeroNodi!=0)
		ist = new Istanza(numeroNodi);
	else 
		ist = new Istanza(fileName);
	
	// 	FLAG DI ATTIVAZIONE / DISATTIVAZIONE DEI SOLVER E DELLE ALTRE FUNZIONALITÀ 
	bool attivoCPLEX = false;				// solver CPLEX attivato
	bool attivoNS = true;				// NearSolver attivato
	bool attivoTWO = false;				// TwoOptSolver attivato
	bool scriviIstanza = false;			// scrittura istanza attivata
	bool SolNS = false;					// scrittura soluzione NearSolver attivata
	bool SolTWO = false;					// scrittura soluzione TwoOptSolver attivata
	bool qualsiasiSTART = true;			// ricerca nodo iniziale ottimo attivata
	
	//	VALORI DEFAULT PER I SOLVER 
	const int MAX_2OPT = 10000;
	const int MAX_K = 30;
	int START = 0;
	const int MAX_CPLEX_PROB = 300;			// limite massimo nodi per la generazione del problema CPLEX
	const int MAX_CPLEX_EXEC = 100;			// limite massimo nodi per l'esecuzione del solver CPLEX
	
	
	if (qualsiasiSTART)
		START = -1;
	
	if (scriviIstanza){
		string nomeFileIst = "util/Ist_rand";
		nomeFileIst = nomeFileIst+to_string(ist->getN())+".txt";
		ist->toFileJSON(nomeFileIst);
		ist->toFileMatriceDistanze("Ist_MatriceDistanze.txt");
		}
		
	if (ist->getN()>MAX_CPLEX_PROB)			// disabilita CPLEX per istanze più grandi di 300 nodi
		attivoCPLEX = false;
	CPLEX_Solver* CPX;
	if (attivoCPLEX){
		CPX = new CPLEX_Solver(ist, env, lp);
		if (ist->getN()<=MAX_CPLEX_PROB)
		CPX->risolvi();
		}
	
	NearSolver* NS;
	if (attivoNS){
		NS = new NearSolver(ist);
		NS->risolvi(START);
		if (SolNS)
			NS->getSoluzione()->toFileJSON("util/solNear.txt");
		}
	
	
/*	TwoOptSolver* TWO;
	if (attivoTWO){
		TWO = new TwoOptSolver(ist, MAX_2OPT);
		TWO->risolvi(START);
		if (SolTWO)
			TWO->getSoluzione()->toFileJSON("util/solTwo_Opt.txt");
		} */
	

	
	cout << "\n\n -------  RISULTATI FINALI  -------- \n\n";
	if (attivoCPLEX) cout << " Il minimo per CPLEX è: "<< CPX->getFO() << " in "<<CPX->getTempoRisoluzione()<< " secondi " << endl;
	if (attivoNS)	cout << " Il minimo per Nearest Neighbor è: "<< NS->getFO() << " in " << NS->getTempoRisoluzione() << " secondi " << endl;
	//if (attivoTWO)	cout << " Il minimo per TwoOpt Solver è: "<< TWO->getFO()  << " in " << TWO->getTempoRisoluzione() << " secondi " << endl;
	
	
	// CANCELLA OGGETTI DALLO STACK
	if (attivoNS) delete NS;
//	if (attivoTWO) delete TWO;
	
	delete ist;
	
	if (attivoCPLEX){
		// LIBERA MEMORIA CPLEX
		delete CPX;
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
    		}
    
    return 1;
}
