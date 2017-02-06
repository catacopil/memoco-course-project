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

using namespace std;

bool verbose = true;				// indica se stampare sulla console messaggi di log sull'avanzamento del programma

// error status and messagge buffer
int status;
char errmsg[BUF_SIZE];

const int NAME_SIZE = 512;		// creiamo un array di caratteri per salvare i nomi delle variabili
char name[NAME_SIZE];

void setupLP(CEnv env, Prob lp, int N, double C[]) {
int nodoStart = 0;			// impostiamo il nodo iniziale
int startIndice = 0;
int indiciX[N][N];			// matrici che restituiscono la posizione degli indici delle variabili
int indiciY[N][N];
//double C[N*N];				// distanza tra nodi in array

// --------  INIT MATRICI CHE RESTITUISCONO GLI INDICI -------
// X01,X02,X10,X12,X20,X21,Y01,Y02,Y10,Y12,Y20,Y21
//  0   1   2   3   4   5   6   7   8   9  10   11
    for(int j=0;j<N;j++){
       for(int k=0;k<N;k++){
           if(j!=k){
               indiciX[j][k]=startIndice;
               startIndice=startIndice+1;
           }
       }
    }
    for(int j=0;j<N;j++){
       for(int k=0;k<N;k++){
           if(j!=k){
               indiciY[j][k]=startIndice;
               startIndice=startIndice+1;
           }
       }
    }
// --------  AGGIUNTA VARIABILI AL MODELLO -------
// crea le variabili x_ij e y_ij, specificandone tipo, limiti del dominio (lb ed ub), nome e costo in funzione obiettivo
    // ----- VARIABILI X
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
			if (i==j) continue;
            char xtype = 'I'; 		// variabile di tipo intero
            double lb = 0.0; 		// limite inferiore del dominio, notare che è un double
            double ub = CPX_INFBOUND; 		// limite superiore
            snprintf(name, NAME_SIZE, "X_%d_%d", i, j);         // costruisce array con i nomi delle variabili X
            // std::cout << "|" << name[0] << "| i=" <<i<<" j="<<j<< std::endl;
            char* xname = (char*)(&name[0]);
            // inserimento delle variabili X_i_j nel modello
            CHECKED_CPX_CALL( CPXnewcols, env, lp, 1, 0, &lb, &ub, &xtype, &xname );    // lo zero in posizione obj e' il coefficiente della variabile nella funzione obiettivo
		}
	}
    // ----- VARIABILI Y
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
			if (i==j) continue;
            char ytype = 'I';
            double lb = 0;
            double ub = 1;
            snprintf(name, NAME_SIZE, "Y_%d_%d", i, j);
            char* yname = (char*)(&name[0]);
            // inserimento delle variabili Y_i_j nel modello
            CHECKED_CPX_CALL( CPXnewcols, env, lp, 1, &C[i*N+j], &lb, &ub, &ytype, &yname );
        }
    }

// --------  AGGIUNTA VINCOLI AL MODELLO -------
	// Aggiunta vincolo 1 

    std::vector<int> idx1(N-1);
    std::vector<double> coef1(N-1, 1);          // vector con N-1 elementi di valore 1.0
    char sense1 = 'E';
    int matbeg1 = 0;
    for (int i = 0; i < idx1.size(); i++){
    		idx1[i] = 
		idx1[i] = i+((nodoStart+1)*(N-1)-N+1);            //TODO: perché non uso le matrici per recuperare gli indici
	}
	double terminiNoti[1] = {N};
	CHECKED_CPX_CALL( CPXaddrows, env, lp, 0, 1, idx1.size(), terminiNoti, &sense1, &matbeg1, &idx1[0], &coef1[0], NULL , NULL );
// SIGNIFICATO Argomenti: 
// int CPXXaddrows( CPXCENVptr env, CPXLPptr lp, CPXDIM ccnt, CPXDIM rcnt, CPXNNZ nzcnt, double const * rhs, char const * sense, CPXNNZ const * rmatbeg, CPXDIM const * rmatind, double const * rmatval, char const *const * colname, char const *const * rowname )

	// Aggiunta vincolo 2
	for (int k = 0; k < N; k++){
		if (k==nodoStart) continue;
            std::vector<int> idx((N-1)*2);
            std::vector<double> coef((N-1)*2, 1);        // coeff contiene solo i coefficienti diversi da zero
            char sense = 'E';
            int index = 0;
            for (int j = 0; j <= (N-1); j++){
                            if (j==k) continue;
                idx[index] = indiciX[j][k];
                index++;
            }
            for (int j = 0; j <= (N-1); j++){
                            if (j==k) continue;
                idx[index] = indiciX[k][j];
                coef[index]=-1;
                index++;
            }
            int matbeg = 0;
            double termineNoto[1] = {1};
            CHECKED_CPX_CALL( CPXaddrows, env, lp, 0, 1, idx.size(), termineNoto, &sense, &matbeg, &idx[0], &coef[0], 0, 0 );
        }
	
	//Aggiunta vincolo 3
    for (int i = 0; i < N; i++){
        std::vector<int> idx(N-1);
        std::vector<double> coef(N-1, 1);        // coeff contiene solo i coefficienti diversi da zero
        char sense = 'E';
        int index = 0;
        for (int j = 0; j <= (N-1); j++){
			if (j==i) continue;
            idx[index] = indiciY[i][j];
            index++;
        }
        int matbeg = 0;
        double termineNoto[1] = {1};
        CHECKED_CPX_CALL( CPXaddrows, env, lp, 0, 1, idx.size(), termineNoto, &sense, &matbeg, &idx[0], &coef[0], 0, 0 );
    }
    
    //Aggiunta vincolo 4
    for (int j = 0; j < N; j++)
    {
        std::vector<int> idx(N-1);
        std::vector<double> coef(N-1, 1);        // coeff contiene solo i coefficienti diversi da zero
        char sense = 'E';
        int index = 0;
        for (int i = 0; i <= (N-1); i++){
			if (i==j) continue;
            idx[index] = indiciY[i][j];
            index++;
        }
        int matbeg = 0;
        double termineNoto[1] = {1};
        CHECKED_CPX_CALL( CPXaddrows, env, lp, 0, 1, idx.size(), termineNoto, &sense, &matbeg, &idx[0], &coef[0], 0, 0 );
    }
    
    //Aggiunta vincolo 5
    for (int i = 0; i < N; i++){
	   for (int j = 0; j <= (N-1); j++){
		 if (j==i) continue;
            std::vector<int> idx(2);
            std::vector<double> coef(2, 1);        	// coeff contiene solo i coefficienti diversi da zero
            coef[1]=-N;

            char sense = 'L';
            idx[0]=indiciX[i][j];
            idx[1]=indiciY[i][j];  
            int matbeg = 0;
            double termineNoto[1] = {0};
            CHECKED_CPX_CALL( CPXaddrows, env, lp, 0, 1, idx.size(), termineNoto, &sense, &matbeg, &idx[0], &coef[0], 0, 0 );
            }
	}
}

int main(int argc, char const *argv[]) {
	int numeroNodi=10;						// numero di nodi del problema
	string fileName;						// nome del file che contiene l'istanza del problema
	ifstream lettoreIstanza;
	if (argc != 2){
		std::cout << "Errore: argomenti non sufficienti, argc= "<< argc << endl;
		for(int i=0; i<argc; i++)
			cout << argv[i]<< " ";
		return 0;
	}
	try{
            fileName = argv[1];
	}
	catch (std::exception& e) {
        std::cout << ">>>Eccezione durante lettura parametri: " << e.what() << std::endl;
        return 0;
	}
	
	Istanza* ist = new Istanza(fileName);
	ist->stampaNodi();
	
	Soluzione* sol = new Soluzione(ist);
	cout << " Stampo la soluzione di base:"<<endl;
	sol->stampa();
	
	
	ist->toFileJSON("Ist_JSON.txt");
	ist->toFileMatriceDistanze("Ist_MatriceDistanze.txt");
	
	
	
	
	
	
	
	
	
	
	
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
	double arr_distanze[numeroNodi*numeroNodi];							// distanze in forma di array monodimensione
	for(int i=0; i<numeroNodi; i++)
		for(int j=0; j<numeroNodi; j++)
			arr_distanze[i*numeroNodi+j] = distanze[i][j];
	
    try {
        DECL_ENV(env);
        DECL_PROB(env, lp);
        setupLP(env, lp, numeroNodi, arr_distanze);			// eseguo la funzione setupLP che imposta il modello
        CHECKED_CPX_CALL(CPXwriteprob, env, lp, "perforatore.lp", NULL);		// scrive il modello su file
        cout << "Inizia l'esecuzione del solver...." << endl;
        clock_t tempo = clock();
        CHECKED_CPX_CALL(CPXmipopt, env, lp);									// esecuzione del solver
        tempo = clock() - tempo;
        cout << "Problema risolto in "<< tempo <<" clocks ("<< ((float)tempo/CLOCKS_PER_SEC) <<" secondi)"<< endl;
        double val_soluzione;
        CHECKED_CPX_CALL(CPXgetobjval, env, lp, &val_soluzione);
        cout << "Valore funzione obiettivo: " << val_soluzione << endl;				// stampa il valore della funzione obiettivo per la soluzione ottima
        CHECKED_CPX_CALL(CPXsolwrite, env, lp, "perforatore.sol");				// stampa la soluzione su file

        // libera memoria
        CPXfreeprob(env, &lp);
        CPXcloseCPLEX(&env);
    } catch (std::exception& e) {
        cout << ">>>Eccezione durante l'esecuzione del solver: " << e.what() << endl;
    } */
    
    
    return 1;
}
