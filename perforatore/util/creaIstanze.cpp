#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <math.h>
#include <random>
#include "punto.h"

using namespace std;

int main(int argc, char* argv[]){
	random_device rd;
	mt19937 mt_generator(rd());
	uniform_int_distribution<int> distribution(0,1000);			// generatore casuale di interi tra 0 e 1000
	
	int N = 50;					// numero di nodi dell'istanza di default
        if (argc>=2){
            try{
                N = stoi(argv[1]);
            }
            catch (std::exception& e) {
            std::cout << ">>>Eccezione nella lettura parametri input: la sintassi corretta è 'istanza NrNodi' " << e.what() << std::endl;
            return 0;
            }
        }
            
	double M[N][N] {0.0};
	vector<Punto> arr_nodi;
        cout << "-----Genero "<<N<<" istanze------" << endl;
	cout << "RAND_MAX: "<< RAND_MAX<<endl;
	
// -----	CREAZIONE NODI		--------
	for(int k=0; k<N; k++){			// popola vector
		Punto nuovo(distribution(mt_generator),distribution(mt_generator));
		//if (k==3||k==4)
		//	nuovo = Punto(3,5);
		bool trovato = true;
		while (trovato){				// crea nuovi nodi se quello generato è uguale ad uno dei precedenti
			trovato = false;
			for (int s=0; s<arr_nodi.size() && not trovato; s++){
                                //Punto ciccio(arr_nodi[s]);
				trovato = (arr_nodi[s]==nuovo);
				}
			if (trovato){
				cout<< "Punto già presente: " << nuovo.stampa()<<endl;
				nuovo = Punto(distribution(mt_generator),distribution(mt_generator));
				}
			}
		arr_nodi.push_back(nuovo);
		}
		
		
// -------	STAMPA DEI NODI CREATI
	for(int k=0; k<arr_nodi.size(); k++){		
		Punto p = arr_nodi[k];
		cout << k << ") " << p.stampa() << endl;
		}
		
		
// -----		GENERAZIONE ISTANZA CON I PUNTI IN FORMATO JSON
	FILE* punt;
	string nomeFileJSON = "istanzaJSON_";
	nomeFileJSON = nomeFileJSON+to_string(N)+"nodi.txt";
	punt = fopen(nomeFileJSON.c_str(), "w");
	if (punt!= NULL){
     	fprintf(punt, " N = %d \n ([", N);
     	for (int i=0; i<arr_nodi.size(); i++){
     		fprintf(punt, "[ %d,", arr_nodi[i].x);
     		if (i==arr_nodi.size()-1)
     			fprintf(punt, " %d]", arr_nodi[i].y);			// stampo l'ultimo senza la virgola dopo le quadre
     		else
	     		fprintf(punt, " %d],", arr_nodi[i].y);
     		}
		fprintf(punt, "]) \n");
		fclose(punt);
	}
	else
		cout << "Errore scrittura file "<< nomeFileJSON << endl;
        
    cout << "Generato il file " << nomeFileJSON << " con l'istanza in formato JSON dei " << N << " nodi" << endl;
    

// -----	CALCOLO MATRICE DISTANZE (M)	--------
	for(int i=0; i<arr_nodi.size(); i++)
		for(int j=0; j<arr_nodi.size(); j++){
			if (i==j) continue;
			Punto A = arr_nodi[i];
			Punto B = arr_nodi[j];
			M[i][j] = A.distanza(&B);
			}
	
// -----	STAMPA MATRICE		--------
        /*
	for(int i=0; i<N; i++){
		for(int j=0; j<N; j++){
			cout << M[i][j] << "\t\t";
			//if (M[i][j] == 0) cout << "\t";
			}
		cout << " |" << endl;
		}
         */
		
// -----	SCRITTURA MATRICE SU FILE		--------			
	FILE* pFile;
     string nomeFileOut = "istanza_";
     nomeFileOut = nomeFileOut+to_string(N)+"nodi.txt";
	pFile = fopen(nomeFileOut.c_str(), "w");
        fprintf(pFile, " N = %d \n", N);
        for (int i=0; i<N; i++){
		for(int j=0; j<N; j++)
                    fprintf(pFile, "%f ",M[i][j]);
		fprintf(pFile, " \n");
		}
	fclose(pFile);
        
    cout << "Generato il file " << nomeFileOut << " con la matrice delle distanze tra i " << N << " nodi" << endl;
		
}
