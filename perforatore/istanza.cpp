/*
*	Implementazione della classe Istanza
*
*/

#include "istanza.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

bool Istanza::verbose = false;

Istanza::Istanza(int numeroNodi = 10){			// costruttore di default oppure costruttore di istanze casuali
	N = numeroNodi;
	matriceDistanzeCalcolata = false;
	random_device rd;
	mt19937 mt_generator(rd());
	uniform_int_distribution<int> distribution(0,1000);			// generatore casuale di interi tra 0 e 1000
	
	for(int k=0; k<N; k++){			// popola il vector dei Punti
		Punto nuovo(distribution(mt_generator),distribution(mt_generator));
		bool trovato = true;
		while (trovato){				// crea nuovi nodi se quello generato è uguale ad uno dei precedenti
			trovato = false;
			for (int s=0; s<arr_nodi.size() && !trovato; s++)
				trovato = (arr_nodi[s]==nuovo);
			if (trovato){
				if (verbose) cout<< " !! Punto già presente: " << nuovo.stampa()<<endl;
				nuovo = Punto(distribution(mt_generator),distribution(mt_generator));
				}
			}
		arr_nodi.push_back(nuovo);
		}
	if (verbose) cout << " Generati "<<N<<" punti random \n";
}

Istanza::Istanza(string nomeFile){				// costruttore che legge l'istanza da un file
	matriceDistanzeCalcolata = false;
	int num, x, y; num = x = y = 0;
	char c;
	bool hox=false, hoy=false;
	ifstream lettorePunti;
	int input, numeroNodi;
	
     // INIZIO LETTURA PUNTI
     try{
	if (verbose) cout << "Inizio lettura punti" << endl;
	lettorePunti.open(nomeFile);
	char* in = new char[100];
	lettorePunti.getline(in,100);					// leggo la prima riga per capire quanti nodi ha l'istanza
	string primaLinea(in);
	primaLinea.erase(0,4);
	numeroNodi = stoi(primaLinea);
	if (verbose) cout << "Nodi: " << numeroNodi << endl;
	lettorePunti.get(c);
	streampos vecchiaPosizione = lettorePunti.tellg();
	if (lettorePunti.is_open()){	
		while (lettorePunti.get(c)){					// leggo ogni carattere finchè non finisco il file 	
			if (isdigit(c)){
				lettorePunti.seekg (vecchiaPosizione);
				lettorePunti >> input;
				if (!hox){					// ho letto x
					hox = true;
					x = input;
					}
				else if (!hoy){				// ho letto y
						hoy = true;
						y = input;
						}
				while (isdigit(c)){		// va avanti finchè ci sono cifre da leggere
					lettorePunti.get(c);
					vecchiaPosizione = lettorePunti.tellg();
					}
				}

			if (hox && hoy){
				Punto nuovo(x, y);
				// CONTROLLO DUPLICATI
				bool trovato = true;
				while (trovato){				// crea nuovi nodi se quello generato è uguale ad uno dei precedenti
					trovato = false;
					for (int s=0; s<arr_nodi.size() && !trovato; s++)
						trovato = (arr_nodi[s]==nuovo);
					if (trovato){
						cout<< " !! Punto già presente: " << nuovo.stampa()<<endl;
						throw runtime_error(" Trovati 2 punti identici ----> "); 
						}
					}
				arr_nodi.push_back(nuovo);
				if (verbose) cout << " Letto ["<< x <<","<< y <<"]\t";
				x = y = 0;
				hox = hoy = false;
				}
			vecchiaPosizione = lettorePunti.tellg();
			}
		}
	}
	catch (std::exception& e) {
		cout << ">>>Eccezione durante lettura file istanza: " << e.what() << endl;
	}
	N = numeroNodi;
	//stampaNodi();
}

void Istanza::stampaNodi(){
	cout << "\n Nodi dell'istanza: "<<endl;
	for(int k=0; k<arr_nodi.size(); k++){		
		Punto p = arr_nodi[k];
		cout << k << ") " << p.stampa() << endl;
		}
	cout << endl;
	if (verbose) cout << " Stampati i "<<arr_nodi.size()<< " nodi dell'istanza \n";
}


int Istanza::getN(){
	return N;
}

Punto Istanza::getPunto(int index){
	if (index>=0 && index<arr_nodi.size())
		return arr_nodi[index];
	else 
		return Punto(0,0);
}


vector<Punto>* Istanza::getNodi(){
//  --------	CREA UNA COPIA DEI NODI DELL'ISTANZA	----------
	vector<Punto>* copia = new vector<Punto>;
	*copia = arr_nodi;
	return copia;
}


vector<vector<double>>* Istanza::getDistanze(){
//  --------	CREA UNA COPIA DELLA MATRICE DELLE DISTANZE 	-------
	if (!matriceDistanzeCalcolata)
		calcolaMatriceDistanze();
	vector<vector<double>>* copia = new vector<vector<double>>;
	*copia = M;
	return copia;
}


string Istanza::toFileJSON(string nomeFile = ""){
// -----		GENERAZIONE ISTANZA CON I PUNTI IN FORMATO JSON		--------
	FILE* pFile;
	string nomeFileJSON = nomeFile;
	if (nomeFileJSON == "")
		nomeFileJSON = "istanzaJSON_"+to_string(N)+"nodi.txt";
	pFile = fopen(nomeFileJSON.c_str(), "w");
	if (pFile!= NULL){
     	fprintf(pFile, " N = %d \n ([", N);
     	for (int i=0; i<arr_nodi.size(); i++){
     		fprintf(pFile, "[ %d,", arr_nodi[i].x);
     		if (i==arr_nodi.size()-1)
     			fprintf(pFile, " %d]", arr_nodi[i].y);			// stampo l'ultimo senza la virgola dopo le quadre
     		else
	     		fprintf(pFile, " %d],", arr_nodi[i].y);
     		}
		fprintf(pFile, "]) \n");
		fclose(pFile);
	}
	else
		cout << "Errore scrittura file "<< nomeFileJSON << endl;
	cout << " Generato il file " << nomeFileJSON << " con l'istanza in formato array | " << N << " nodi" << endl;
	return nomeFileJSON;
}

string Istanza::toFileMatriceDistanze(string nomeFile = ""){
// -------	SCRITTURA MATRICE DISTANZE SU FILE		--------		
	if (!matriceDistanzeCalcolata)
		calcolaMatriceDistanze();
	FILE* pFile;
	string nomeFileOut = nomeFile;
	if (nomeFileOut == "")
		nomeFileOut = "istanzaReale"+to_string(N)+"nodi.txt";
	pFile = fopen(nomeFileOut.c_str(), "w");
	fprintf(pFile, " N = %d \n", N);
	for (int i=0; i<N; i++){
		for(int j=0; j<N; j++)
			fprintf(pFile, "%f ", M[i][j]);
			fprintf(pFile, " \n");
			}
		fclose(pFile);
	cout << " Generato il file " << nomeFileOut << " con la matrice delle distanze tra i " << N << " nodi" << endl;
	return nomeFileOut;
}

void Istanza::calcolaMatriceDistanze(){
//  --------	CALCOLA MATRICE DELLE DISTANZE TRA I NODI	----------
	M.resize(N);
	for (int k=0; k<N; k++)			// sistemo la dimensione dell'array
		M[k].resize(N);
		
	for(int i=0; i<arr_nodi.size(); i++)
		for(int j=0; j<arr_nodi.size(); j++){
			if (i==j) continue;					// non calcola le distanze tra gli stessi punti
			Punto A = arr_nodi[i];
			Punto B = arr_nodi[j];
			M[i][j] = A.distanza(&B);
			}
	matriceDistanzeCalcolata = true;
	if (verbose) cout << " Effettuato il calcolo della matrice delle distanze tra " << arr_nodi.size() << " nodi" << endl;
}

