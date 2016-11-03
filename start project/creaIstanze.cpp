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
	uniform_int_distribution<int> distribution(0,RAND_MAX);		// da sostituire con 

	cout << "-----Genero istanze------" << endl;
	cout << "RAND_MAX: "<< RAND_MAX<<endl;
	int N = 10;					// numero di nodi dell'istanza
	double M[N][N] {0.0};
	vector<Punto> arr_nodi;
	
// -----	CREAZIONE NODI		--------
	for(int k=0; k<N; k++){			// popola vector
		Punto nuovo(distribution(mt_generator),distribution(mt_generator));
		if (k==3||k==4)
			nuovo = Punto(3,5);
		bool trovato = true;
		while (trovato){				// crea nuovi nodi se quello generato è uguale ad uno dei precedenti
			trovato = false;
			for (int s=0; s<arr_nodi.size() && not trovato; s++){
				Punto 
				trovato = (arr_nodi[s]==nuovo);
				}
			if (trovato){
				cout<< "Punto già presente: " << nuovo.stampa()<<endl;
				nuovo = Punto(distribution(mt_generator),distribution(mt_generator));
				}
			}
		arr_nodi.push_back(nuovo);
		}
		
		// TODO: CONTROLLO CHE NON CE NE SIA GIA' UNO UGUALE
	
	for(int k=0; k<arr_nodi.size(); k++){			// stampa dei NODI
		Punto p = arr_nodi[k];
		cout << k << ") " << p.stampa() << endl;
		}

// -----	CALCOLO MATRICE DISTANZE (M)	--------
	for(int i=0; i<arr_nodi.size(); i++)
		for(int j=0; j<arr_nodi.size(); j++){
			if (i==j) continue;
			Punto A = arr_nodi[i];
			Punto B = arr_nodi[j];
			M[i][j] = A.distanza(&B);
			}
	
// -----	STAMPA MATRICE		--------
	for(int i=0; i<N; i++){
		for(int j=0; j<N; j++){
			cout << M[i][j] << "\t\t";
			//if (M[i][j] == 0) cout << "\t";
			}
		cout << " |" << endl;
		}
		
// -----	SCRITTURA MATRICE SU FILE		--------		
	FILE* pFile;
	pFile = fopen("istanza10b.txt", "w");
	for (int i=0; i<N; i++){
		for(int j=0; j<N; j++)
		//fwrite (M[i], sizeof(double), sizeof(M[i]), pFile);
			fprintf(pFile, "%f ",M[i][j]);
		fprintf(pFile, " \n");
		}
	fclose(pFile);
		
}
