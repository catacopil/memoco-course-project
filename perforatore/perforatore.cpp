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
#include "Miosolver.h"
#include "TwoOptSolver.h"

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
		cout << " Errore: argomenti non sufficienti, argc= "<< argc << endl;
		for(int i=0; i<argc; i++)
			cout << argv[i]<< " ";
		return 0;
	}
	try{
		numeroNodi = stoi(argv[1]);
	}
	catch (std::exception& e) {
		cout << ">>>Eccezione nella lettura del parametro numerico in input " << e.what() << endl;
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
	
	Soluzione* sol = new Soluzione(ist);
	//cout << " Stampo la soluzione di base:"<<endl;
	//sol->stampa();
	
	string nomeFileIst = "Ist_JSON";
	nomeFileIst = nomeFileIst+to_string(ist->getN())+".txt";
	//ist->toFileJSON(nomeFileIst);
	//ist->toFileMatriceDistanze("Ist_MatriceDistanze.txt");
	
	//CPLEX_Solver* CPX = new CPLEX_Solver(ist, env, lp);
	//CPX->risolvi();
	
	// TODO: da notare il fatto che iniziare dal punto 0 o 3 o 5 cambia abbastanza le cose
	int START = 0;
	
	NearSolver* NS = new NearSolver(ist);
	MioSolver* MIO = new MioSolver(ist,30);
	TwoOptSolver* TWO = new TwoOptSolver(ist, 1000);

	Soluzione* solInteressante;
	Soluzione* solMia;
	
	
	double minimoMio = 1000000000;
	int maxProve = 30;
	/*for (int i=0; i<maxProve; i++){
		MIO->risolvi(i);
		if (MIO->getFO()<minimoMio){
			minimoMio = MIO->getFO();
			}
	}*/
	
	double minNS = 1000000000;
	double minMIO = 1000000000;
	double minTWO = 1000000000;
	double minTWOtempo;
	int recordTWO = START;
	int recordNS = START;
	int recordMIO = START;
	
	//for (START=0; START<ist->getN(); START++){
		NS->risolvi(START);
		MIO->risolvi(START);
		TWO->risolvi(START);
		/*
		if (NS->getFO()<minNS){
			minNS = NS->getFO();
			recordNS = START;
			}
		if (MIO->getFO()<minMIO){
			minMIO = MIO->getFO();
			recordMIO = START;
			}
		if (TWO->getFO()<minTWO) {
			minTWO = TWO->getFO();
			minTWOtempo = TWO->getTempoRisoluzione();
			recordTWO = START;
			}
		/*
		cout << " Il minimo per Nearest Neighbor è: "<< NS->getFO() << " [partenza: "<<START<<"]"<< endl;
		cout << " Il minimo per Mio Solver è: "<< MIO->getFO() << " [partenza: "<<START<<"]"<< endl;
		cout << " Il minimo per TwoOpt Solver è: "<< TWO->getFO() << " [partenza: "<<START<<"]"<< endl<<endl;
		*/
		
		//} 
	// solInteressante = NS->getSoluzione();
	// TWO->risolvi(recordSTART);
	// solMia = TWO->getSoluzione();

	// solInteressante->toFileJSON("util/disegno istanze/solNearest.txt");
	// solMia->toFileJSON("util/disegno istanze/solTwo.txt");
	
	cout << "\n\n -------  RISULTATI FINALI  -------- \n\n";
	//cout << " Il minimo per CPLEX è: "<< CPX->getFO() << " in "<<CPX->getTempoRisoluzione()<< " secondi " << endl;
	//cout << " Il minimo per Mio Solver è: "<< minimoMio << endl;
	cout << " Il minimo per Nearest Neighbor è: "<< NS->getFO() << " in " << NS->getTempoRisoluzione() << " secondi [start: "<<recordNS<<"]" << endl;
	cout << " Il minimo per Mio Solver è: "<< MIO->getFO() << " in " << MIO->getTempoRisoluzione() << " secondi [start: "<<recordMIO<<"]" << endl;
	cout << " Il minimo per TwoOpt Solver è: "<< TWO->getFO() << " in " << TWO->getTempoRisoluzione() << " secondi [start: "<<recordTWO<<"]" << endl;
	
	
	
	
	
	// CANCELLA OGGETTI DALLO STACK
	//delete CPX;
	delete NS;
	delete MIO;
	delete TWO;
	delete sol;
	delete ist;
	
	
	// LIBERA MEMORIA CPLEX
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	
	
	
	
	
	
	
	/*
        // lettura della prima riga
        try{
            lettoreIstanza.open(fileName);
            char* in = new char[100];
            lettoreIstanza.getline(in,100);
            string primaLinea(in);
            primaLinea.erase(0,4);
            numeroNodi = stoi(primaLinea);
            cout << "Nodi: " << numeroNodi << endl;
        }
        catch (std::exception& e) {
        std::cout << ">>>Eccezione durante lettura file istanza: " << e.what() << std::endl;
        return 0;
	}
        double distanze[numeroNodi][numeroNodi];				
        try{
            // matrice con le distanze del problema
            // lettura file istanza
		for(int i=0; i<numeroNodi; i++)
                    for(int j=0; j<numeroNodi; j++){
                        if (lettoreIstanza.is_open()){
                            double num;
                            if (lettoreIstanza >> num)
				distanze[i][j] = num;
                            }
			}
		// STAMPA DELLA MATRICE LETTA
		/*for(int i=0; i<numeroNodi; i++){
			for(int j=0; j<numeroNodi; j++){
				cout << distanze[i][j] << "\t";
				}
			cout << " |" << endl;
			} */  /*
            }
	catch (std::exception& e) {
        std::cout << ">>>Eccezione durante lettura file istanza: " << e.what() << std::endl;
        return 0;
	}
	
	*/
    
    
    return 1;
}
