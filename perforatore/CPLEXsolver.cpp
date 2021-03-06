/*
*	Implementazione della classe CPLEX_Solver
*
*/

#include "CPLEXsolver.h"
#include <iostream>



bool CPLEX_Solver::verbose = true;

CPLEX_Solver::CPLEX_Solver(Istanza* i, CEnv e, Prob prob):Solver(i), ENV(e), LP(prob){
//  -------	COSTRUTTORE SEMPLICE
	tempoRisoluzione = -1.0;
	valoreFO = -1.0;
	
	try {		
		setupLP();			// imposta il modello 
		CHECKED_CPX_CALL(CPXwriteprob, ENV, LP, "util/perforatore.lp", NULL);		// scrive il modello su file
		
	} catch (std::exception& e) {
		cout << ">>>Eccezione durante la creazione e impostazione del solver CPLEX: " << e.what() << endl;
		return;
	} 
	
	if (verbose) cout << " Solver CPLEX creato per l'istanza richiesta \n";
}

CPLEX_Solver::~CPLEX_Solver(){
//  -------	DISTRUTTORE
	if (verbose) cout << " Distrutto Solver CPLEX ";
}

void CPLEX_Solver::risolvi(){	
//  ------- 	AVVIA IL SOLVER PER TROVARE LA SOLUZIONE
	try{
		cout << " Inizia l'esecuzione del solver CPLEX.... buona attesa ^_^ " << endl;
		
		chrono::high_resolution_clock::time_point inizio = std::chrono::high_resolution_clock::now();
		CHECKED_CPX_CALL(CPXmipopt, ENV, LP);									// esecuzione del solver
		
		chrono::high_resolution_clock::time_point fine = std::chrono::high_resolution_clock::now();
		//secondi duration = fine - inizio;
		auto duration = chrono::duration_cast<chrono::microseconds>(fine - inizio);
		double microsec = duration.count();
		tempoRisoluzione = microsec/1000000;
		if (tempoRisoluzione >= 60.0){
			int minuti = tempoRisoluzione/60;
			double secondi = fmod(tempoRisoluzione, 60);
			cout << " Problema risolto in "<< minuti <<" minuti e "<< secondi <<" secondi"<< endl;
			}
		else
			cout << " Problema risolto in "<< tempoRisoluzione <<" secondi"<< endl;
		
		CHECKED_CPX_CALL(CPXgetobjval, ENV, LP, &valoreFO);

		cout << " Valore funzione obiettivo: " << valoreFO << endl;					// stampa il valore della funzione obiettivo per la soluzione ottima
		CHECKED_CPX_CALL(CPXsolwrite, ENV, LP, "util/perforatore.sol");				// stampa la soluzione su file
		
	}
	catch (std::exception& e) {
		cout << ">>> Eccezione durante l'esecuzione del Solver CPLEX: " << e.what() << endl;
	}
	if (verbose) cout << " Problema risolto con il Solver CPLEX! \n";
}

double CPLEX_Solver::getFO(){
	return valoreFO;
}

double CPLEX_Solver::getTempoRisoluzione(){
	return tempoRisoluzione;
}

Soluzione* CPLEX_Solver::getSoluzione(){
// TODO: sistemare, creando la soluzione reale 
	sol = new Soluzione(ist);
	return sol;
}

void recuperaSoluzione(){
//  ---------	GENERA UN OGGETTO SOLUZIONE, RECUPERANDO I DATI DALLA SOLUZIONE DI CPLEX
//	TODO: da implementare 
	return;
}


void CPLEX_Solver::setupLP() {
	const int NAME_SIZE = 512;					// array di caratteri per salvare i nomi delle variabili
	char name[NAME_SIZE];
	
	int N = ist->getN();
	vector<vector<double>> distanze = *(ist->getDistanze());

	int nodoStart = 0;			// imposto il nodo iniziale
	int startIndice = 0;
	int indiciX[N][N];			// matrici che restituiscono la posizione degli indici delle variabili
	int indiciY[N][N];

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
            char xtype = 'I'; 			// variabile di tipo intero
            double lb = 0.0; 				// limite inferiore del dominio, notare che è un double
            double ub = CPX_INFBOUND; 		// limite superiore
            snprintf(name, NAME_SIZE, "X_%d_%d", i, j);         // costruisce array con i nomi delle variabili X
            // std::cout << "|" << name[0] << "| i=" <<i<<" j="<<j<< std::endl;
            char* xname = (char*)(&name[0]);
            // inserimento delle variabili X_i_j nel modello
            CHECKED_CPX_CALL( CPXnewcols, ENV, LP, 1, 0, &lb, &ub, &xtype, &xname );    // lo zero in posizione obj e' il coefficiente della variabile nella funzione obiettivo
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
            CHECKED_CPX_CALL( CPXnewcols, ENV, LP, 1, &distanze[i][j], &lb, &ub, &ytype, &yname );
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
		idx1[i] = i+((nodoStart+1)*(N-1)-N+1);  
	}
	double terminiNoti[1] = {N};
	CHECKED_CPX_CALL( CPXaddrows, ENV, LP, 0, 1, idx1.size(), terminiNoti, &sense1, &matbeg1, &idx1[0], &coef1[0], NULL , NULL );
// SIGNIFICATO Argomenti: 
// int CPXXaddrows( CPXCENVptr env, CPXLPptr LP, CPXDIM ccnt, CPXDIM rcnt, CPXNNZ nzcnt, double const * rhs, char const * sense, CPXNNZ const * rmatbeg, CPXDIM const * rmatind, double const * rmatval, char const *const * colname, char const *const * rowname )

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
            CHECKED_CPX_CALL( CPXaddrows, ENV, LP, 0, 1, idx.size(), termineNoto, &sense, &matbeg, &idx[0], &coef[0], 0, 0 );
        }
	
	//Aggiunta vincolo 3
    for (int i = 0; i < N; i++){
        std::vector<int> idx(N-1);
        std::vector<double> coef(N-1, 1); 
        char sense = 'E';
        int index = 0;
        for (int j = 0; j <= (N-1); j++){
			if (j==i) continue;
            idx[index] = indiciY[i][j];
            index++;
        }
        int matbeg = 0;
        double termineNoto[1] = {1};
        CHECKED_CPX_CALL( CPXaddrows, ENV, LP, 0, 1, idx.size(), termineNoto, &sense, &matbeg, &idx[0], &coef[0], 0, 0 );
    }
    
    //Aggiunta vincolo 4
    for (int j = 0; j < N; j++)
    {
        std::vector<int> idx(N-1);
        std::vector<double> coef(N-1, 1);  
        char sense = 'E';
        int index = 0;
        for (int i = 0; i <= (N-1); i++){
			if (i==j) continue;
            idx[index] = indiciY[i][j];
            index++;
        }
        int matbeg = 0;
        double termineNoto[1] = {1};
        CHECKED_CPX_CALL( CPXaddrows, ENV, LP, 0, 1, idx.size(), termineNoto, &sense, &matbeg, &idx[0], &coef[0], 0, 0 );
    }
    
    //Aggiunta vincolo 5
    for (int i = 0; i < N; i++){
	   for (int j = 0; j <= (N-1); j++){
		 if (j==i) continue;
            std::vector<int> idx(2);
            std::vector<double> coef(2, 1); 
            coef[1]=-N;

            char sense = 'L';
            idx[0]=indiciX[i][j];
            idx[1]=indiciY[i][j];  
            int matbeg = 0;
            double termineNoto[1] = {0};
            CHECKED_CPX_CALL( CPXaddrows, ENV, LP, 0, 1, idx.size(), termineNoto, &sense, &matbeg, &idx[0], &coef[0], 0, 0 );
            }
	} 
	if (verbose) cout << " La descrizione del problema è stata completata per il Solver CPLEX \n";
}
/*

void CPLEX_Solver::printCols(){
	// docs: https://www.ibm.com/support/knowledgecenter/SSSA5P_12.6.3/ilog.odms.cplex.help/refcallablelibrary/cpxapi/getcols.html?view=kc
	int N = ist->getN();
	int nonzeroReturned;
	int begin = 0;
	int end = N * (N-1);						// prendo solo le X_i_j
	int cmatspace =  (N*3) *(N-1);					// dimensione massima possibile per i valori che recupero
	std::vector<int> cmatbeg(end-begin+1);				// conterrà gli indici che specificano dove inizia ogni colonna con i valori richiesti
	std::vector<int> cmatRowIndex(cmatspace);				// conterrà gli indici della riga associata ad ogni valore di cmatval
	std::vector<double> cmatval(cmatspace);					// conterrà i valori non_zero recuperati 
	int surplus_p;									// conterrà il valore del surplus

	CHECKED_CPX_CALL(CPXgetcols, ENV, LP, &nonzeroReturned, &cmatbeg[0], &cmatRowIndex[0], &cmatval[0], cmatspace, &surplus_p, begin, end);
	

	cout << " Cmatbeg: \n";
	for(int i=0; i<end-begin+1; i++)
		cout << " "<< i <<") \t"<< cmatbeg[i]<<endl;
	
	cout << " cmatRowIndex: \n";
	for(int i=0; i<cmatspace; i++)
		cout << " "<< i <<") \t"<< cmatRowIndex[i]<<endl;
			
	cout << " cmatval: \n";
	for(int i=0; i<cmatspace; i++)
		cout << " "<< i <<") \t"<< cmatval[i]<<endl;
		
	cout << " Surplus_p = " << surplus_p << endl;

} */
