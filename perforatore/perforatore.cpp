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
#include "TwoOptSolver.h"
#include "TabuSearchSolver.h"

using namespace std;

bool verbose = true;				// indica se stampare sulla console messaggi di log sull'avanzamento del programma

int status;
char errmsg[BUF_SIZE];

int main(int argc, char const *argv[]) {
	
	// 	DICHIARAZIONE VARIABILI PER CPLEX
	DECL_ENV(env);
	DECL_PROB(env, lp);
	
	//typedef std::chrono::duration<double> secondi;
	
	//	LETTURA PARAMETRI DA RIGA DI COMANDO
	int numeroNodi=0;						// numero di nodi del problema
	string fileName;						// nome del file che contiene l'istanza del problema
	ifstream lettoreIstanza;
	int maxIter = 0;
	int tabuLength = 0;
	double maxSecondi = 0.0;
	if (argc < 2){
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
			cout << " >>> Eccezione durante la lettura parametri. \n La sintassi corretta è: \t ./perforatore nomeFileIstanza.txt tempoInSecondi tabuLength maxIter \n -> nomeFileIstanza.txt può essere il nome del file contenente l'istanza oppure il numero dei nodi di un'istanza casuale da generare \n -> tempoInSecondi (opzionale, di default impostato a 10) deve essere un double > 0, rappresenta il limite di tempo nel quale i solver vengono eseguiti \n -> tabuLength (opzionale, di default impostato a 7) deve essere un intero >0 \n -> maxIter (opzionale, di default impostato a 100) deve essere un intero >0 " <<endl;
			return 0;
		}
	}
	try{
		if (argc >= 3)
			maxSecondi = stod(argv[2]);
		if (argc >= 4){
			tabuLength = stoi(argv[3]);
		}
		if (argc >= 5)
			maxIter = stoi(argv[4]);
	}
	catch (std::exception& e) {
		cout << " >>> Eccezione durante la lettura parametri. \n La sintassi corretta è: \t ./perforatore nomeFileIstanza.txt tempoInSecondi tabuLength maxIter \n -> nomeFileIstanza.txt può essere il nome del file contenente l'istanza oppure il numero dei nodi di un'istanza casuale da generare \n -> tempoInSecondi (opzionale, di default impostato a 10) deve essere un double > 0, rappresenta il limite di tempo nel quale i solver vengono eseguiti \n -> tabuLength (opzionale, di default impostato a 7) deve essere un intero >0 \n -> maxIter (opzionale, di default impostato a 100) deve essere un intero >0 " <<endl;
		return 0;
	}
	
	// 	CREAZIONE O LETTURA DELL'ISTANZA
	Istanza* ist;
	if (numeroNodi!=0)
		ist = new Istanza(numeroNodi);
	else 
		ist = new Istanza(fileName);
	
	// 	FLAG DI ATTIVAZIONE / DISATTIVAZIONE DEI SOLVER E DELLE ALTRE FUNZIONALITÀ 
	bool attivoCPLEX = false;				// solver CPLEX attivato
	bool attivoNS = true;				// NearSolver attivato
	bool attivoTWO = true;				// TwoOptSolver attivato
	bool attivoTS = true;				// TabuSearchSolver attivato
	bool scriviIstanza = false;			// scrittura istanza attivata
	bool SolNS = false;					// scrittura soluzione NearSolver attivata
	bool SolTWO = false;					// scrittura soluzione TwoOptSolver attivata
	bool SolTS = false;					// scrittura soluzione TabuSearchSolver attivata
	bool qualsiasiSTART = true;			// ricerca nodo iniziale ottimo attivata
	
	//	VALORI DEFAULT PER I SOLVER (modificabili tramite i parametri inseriti all'inizio)
	int TL_Length = 7;						// lunghezza TabuSearch
	if (tabuLength > 0)	TL_Length = tabuLength;
	int MAX_2OPT = 300;						// Massimo numero di iterazioni per TwoOpt e TabuSearch
	if (maxIter > 0) 	MAX_2OPT = maxIter;
	double TIME_LIMIT = 10.0;				// Limite massimo di tempo esecuzione (in secondi)
	if (maxSecondi > 0.0)
		TIME_LIMIT = maxSecondi;
	int START = 0;							// Punto iniziale
	const int MAX_CPLEX_PROB = 300;			// limite massimo nodi per la generazione del problema CPLEX
	const int MAX_CPLEX_EXEC = 100;			// limite massimo nodi per l'esecuzione del solver CPLEX
	
	
	if (qualsiasiSTART)
		START = -1;
	
	if (scriviIstanza){
		string nomeFileIst = "util/Ist_rand";
		nomeFileIst = nomeFileIst+to_string(ist->getN())+".txt";
		ist->toFileJSON(nomeFileIst);
		string nomeFileMatrice = "util/Ist_rand" + to_string(ist->getN()) + "_MatrDistanze.txt";
		ist->toFileMatriceDistanze(nomeFileMatrice);
		}
		
	if (ist->getN()>MAX_CPLEX_PROB)			// disabilita CPLEX per istanze più grandi di 300 nodi
		attivoCPLEX = false;
	CPLEX_Solver* CPX;
	if (attivoCPLEX){
		CPX = new CPLEX_Solver(ist, env, lp);
		if (ist->getN()<=MAX_CPLEX_EXEC)
		CPX->risolvi();
		}
	
	NearSolver* NS;
	if (attivoNS){
		NS = new NearSolver(ist);
		NS->risolvi(START, TIME_LIMIT);
		if (SolNS)
			NS->getSoluzione()->toFileJSON("util/solNear.txt");
		}
	
	
	TwoOptSolver* TWO;
	if (attivoTWO){
		TWO = new TwoOptSolver(ist, MAX_2OPT);
		TWO->risolvi(TIME_LIMIT);
		if (SolTWO)
			TWO->getSoluzione()->toFileJSON("util/solTwo_Opt.txt");
		} 
	
	TabuSearchSolver* TS;
	if (attivoTS){
		TS = new TabuSearchSolver(ist, TL_Length, MAX_2OPT);
		TS->risolvi(TIME_LIMIT);
		if (SolTS)
			TS->getSoluzione()->toFileJSON("util/solTabu_Search.txt");
	}

	
	cout << "\n -------  RISULTATI FINALI  -------- \n\n";
	if (attivoCPLEX) cout << " Il minimo per CPLEX è: "<< CPX->getFO() << " in "<<CPX->getTempoRisoluzione()<< " secondi " << endl;
	if (attivoNS)	cout << " Il minimo per Nearest Neighbor è: "<< NS->getFO() << " in " << NS->getTempoRisoluzione() << " secondi " << endl;
	if (attivoTWO)	cout << " Il minimo per TwoOpt Solver è: "<< TWO->getFO()  << " in " << TWO->getTempoRisoluzione() << " secondi " << endl;
	if (attivoTS)	cout << " Il minimo per Tabu Search Solver è: "<< TS->getFO()  << " in " << TS->getTempoRisoluzione() << " secondi " << endl;
	
	
	// CANCELLA OGGETTI DALLO STACK
	if (attivoNS) delete NS;
	if (attivoTWO) delete TWO;
	if (attivoTS) delete TS;
	
	delete ist;
	
	if (attivoCPLEX){
		// LIBERA MEMORIA CPLEX
		delete CPX;
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
    		}
    
    return 1;
}
