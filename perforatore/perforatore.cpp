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

using namespace std;

bool verbose = true;				// indica se stampare sulla console messaggi di log sull'avanzamento del programma

int status;
char errmsg[BUF_SIZE];

int main(int argc, char const *argv[]) {
	
	// 	DICHIARAZIONE VARIABILI PER CPLEX
	DECL_ENV(env);
	DECL_PROB(env, lp);

	int numeroNodi=10;						// numero di nodi del problema
	string fileName;						// nome del file che contiene l'istanza del problema
	ifstream lettoreIstanza;
	if (argc != 2){
		cout << " Errore: argomenti non sufficienti, argc= "<< argc << endl;
		for(int i=0; i<argc; i++)
			cout << argv[i]<< " ";
		return 0;
	}
	try{
            fileName = argv[1];
	}
	catch (std::exception& e) {
        cout << ">>> Eccezione durante lettura parametri: " << e.what() << std::endl;
        return 0;
	}
	
	Istanza* ist = new Istanza(fileName);
	ist->stampaNodi();
	
	Soluzione* sol = new Soluzione(ist);
	cout << " Stampo la soluzione di base:"<<endl;
	sol->stampa();
	
	
	ist->toFileJSON("Ist_JSON.txt");
	ist->toFileMatriceDistanze("Ist_MatriceDistanze.txt");
	
	CPLEX_Solver* CPX = new CPLEX_Solver(ist, env, lp);
	CPX->risolvi();




	
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
