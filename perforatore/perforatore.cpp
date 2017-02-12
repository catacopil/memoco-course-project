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
	ist->toFileJSON(nomeFileIst);
	ist->toFileMatriceDistanze("Ist_MatriceDistanze.txt");
	
	CPLEX_Solver* CPX = new CPLEX_Solver(ist, env, lp);
	//CPX->risolvi();
	
	NearSolver* NS = new NearSolver(ist);
	NS->risolvi(3);
	
	Soluzione* solInteressante;
	Soluzione* solMia;
	
	
	MioSolver* MIO = new MioSolver(ist,ist->getN());
	double minimoMio = 1000000000;
	int maxProve = 30;
	/*for (int i=0; i<maxProve; i++){
		MIO->risolvi(i);
		if (MIO->getFO()<minimoMio){
			minimoMio = MIO->getFO();
			}
	}*/
	MIO->risolvi(0);
	
	
	solInteressante = NS->getSoluzione();
	solMia = MIO->getSoluzione();

	solInteressante->toFileJSON("util/disegno istanze/solNearest.txt");
	solMia->toFileJSON("util/disegno istanze/solMia.txt");
	
	
	cout << " Il minimo per CPLEX è: "<< CPX->getFO() << endl;
//	cout << " Il minimo per Mio Solver è: "<< minimoMio << endl;
	cout << " Il minimo per Mio Solver è: "<< MIO->getFO() << " [partenza: 0]"<< endl;
	cout << " Il minimo per Nearest Neighbor è: "<< NS->getFO() << " [partenza: 0]"<< endl;
	
	
	
	
	
	
	// CANCELLA OGGETTI DALLO STACK
	delete CPX;
	delete NS;
	delete MIO;
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
