#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <random>
#include <cctype>
#include "punto.h"

/*
ISTANZA GOOGLE (in ordine):

([[288, 149], [288, 129], [280, 133], [270, 133], [260, 129], [252, 125], [256, 141], [246, 141], [236, 145], [228, 145], [220, 145], [212, 145], [204, 145], [196, 145], [188, 145], [180, 125], [180, 117], [180, 109], [180, 101], [180, 93], [188, 93], [196, 101], [204, 109], [212, 117], [220, 125], [228, 125], [228, 117], [228, 109], [228, 101], [236, 101], [236, 93], [228, 93], [228, 85], [236, 85], [260, 85], [260, 93], [252, 101], [260, 109], [268, 97], [276, 101], [280, 109], [288, 109], [284, 101], [284, 93], [276, 93], [276, 85], [284, 85], [284, 77], [284, 69], [284, 61], [284, 53], [276, 53], [276, 61], [276, 69], [276, 77], [260, 77], [260, 69], [260, 61], [260, 53], [260, 45], [260, 37], [260, 29], [252, 21], [236, 21], [228, 21], [228, 29], [236, 29], [236, 37], [228, 37], [228, 45], [236, 45], [236, 53], [228, 53], [228, 61], [236, 61], [236, 69], [236, 77], [228, 77], [228, 69], [220, 73], [212, 65], [204, 57], [196, 49], [188, 41], [180, 37], [180, 45], [172, 45], [172, 37], [172, 29], [180, 29], [180, 21], [172, 21], [156, 25], [162, 9], [148, 9], [136, 9], [128, 9], [120, 9], [124, 21], [132, 21], [124, 29], [124, 37], [124, 45], [124, 53], [124, 61], [132, 61], [140, 65], [124, 69], [104, 57], [104, 49], [104, 41], [104, 33], [104, 25], [104, 17], [92, 9], [80, 9], [72, 9], [64, 21], [72, 25], [80, 25], [80, 25], [80, 41], [88, 49], [104, 65], [104, 73], [104, 81], [104, 89], [104, 97], [104, 105], [104, 113], [104, 121], [124, 125], [124, 117], [124, 109], [124, 101], [124, 93], [124, 85], [124, 77], [132, 81], [148, 85], [164, 81], [172, 77], [172, 69], [172, 61], [172, 53], [180, 53], [180, 61], [180, 69], [180, 77], [180, 85], [172, 85], [172, 93], [172, 101], [172, 109], [172, 117], [172, 125], [164, 137], [172, 145], [164, 145], [156, 145], [156, 137], [148, 137], [148, 145], [140, 145], [140, 137], [132, 137], [132, 145], [124, 145], [116, 145], [104, 145], [104, 137], [104, 129], [56, 113], [56, 105], [56, 97], [56, 89], [48, 83], [56, 81], [56, 73], [56, 65], [56, 57], [56, 49], [72, 49], [72, 41], [64, 41], [56, 41], [56, 33], [56, 25], [56, 17], [56, 9], [44, 11], [32, 17], [24, 17], [16, 17], [16, 25], [24, 25], [32, 25], [44, 27], [44, 35], [44, 43], [48, 51], [40, 51], [40, 63], [48, 63], [48, 73], [40, 73], [40, 83], [32, 81], [32, 73], [32, 65], [32, 57], [32, 49], [32, 41], [24, 45], [8, 41], [8, 49], [16, 57], [8, 57], [8, 65], [8, 73], [8, 81], [8, 89], [8, 97], [8, 109], [16, 109], [16, 97], [24, 89], [32, 89], [32, 97], [40, 99], [48, 99], [40, 113], [32, 113], [32, 121], [32, 129], [32, 137], [32, 145], [32, 153], [32, 161], [32, 169], [40, 169], [40, 161], [40, 153], [40, 145], [40, 137], [40, 129], [40, 121], [56, 121], [56, 129], [56, 137], [56, 145], [56, 153], [56, 161], [56, 169], [64, 165], [64, 157], [80, 157], [90, 165], [104, 169], [104, 161], [104, 153], [116, 161], [124, 169], [132, 169], [140, 169], [148, 169], [156, 169], [164, 169], [172, 169], [188, 169], [196, 161], [196, 169], [204, 169], [212, 169], [220, 169], [228, 161], [228, 169], [236, 169], [246, 157], [256, 157] ]);

*/

using namespace std;

int main(int argc, char const *argv[]) {
	string fileName;						// nome del file che contiene l'istanza del problema
	ifstream lettorePunti;
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
	vector <int> arr;
	int num, x, y; num = x = y = 0;
	char c;
	vector<Punto> arr_nodi;
	bool hox=false, hoy=false;
	
        // INIZIO LETTURA PUNTI
        try{
		cout << "Inizio lettura punti" << endl;
		lettorePunti.open(fileName);
		if (lettorePunti.is_open()){
			while (lettorePunti.get(c)){
				//cout << c;
				// leggo gli interi finchè non finisco il file 
				
				if (std::isdigit(c)){
					if (!hox) hox = true;
					num = num*10 + std::stoi(&c);
					}
				else if (num!=0){
					arr.push_back(num);
					if (!hoy){				// ho letto x
						x = num;
						hoy = true;
						hox = false;
						}
					else{				// ho letto y
						y = num;
						}
					arr.push_back(num);
					num = 0;
					if (hox && hoy){
						Punto nuovo(x, y);
						// ATTENZIONE: NO CONTROLLO DUPLICATI
						arr_nodi.push_back(nuovo);
						cout << " Letto ["<< x <<","<< y <<"]\t";
						x = y = 0;
						hox = hoy = false;
					}
				}
					
				//lettorePunti >> x;
			}
		}
		cout << "Letti "<<arr.size() << " interi" << endl;
		lettorePunti.close();
		
		cout << "Memorizzati "<<arr_nodi.size() << " punti" << endl;
		int N = arr_nodi.size();
		double M[N][N] {0.0};
		
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
			   string nomeFileOut = "istanzaReale";
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
		
			   catch (std::exception& e) {
			   std::cout << ">>>Eccezione durante lettura file istanza: " << e.what() << std::endl;
			   return 0;
			}
	
}



















/*

int main(int argc, char* argv[]){
	random_device rd;
	mt19937 mt_generator(rd());
	uniform_int_distribution<int> distribution(0,10000);
	
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
        /*
	for(int i=0; i<N; i++){
		for(int j=0; j<N; j++){
			cout << M[i][j] << "\t\t";
			//if (M[i][j] == 0) cout << "\t";
			}
		cout << " |" << endl;
		}
         */
		/*
// -----	SCRITTURA MATRICE SU FILE		--------		
	FILE* pFile;
        string nomeFileOut = "istanza";
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
*/
