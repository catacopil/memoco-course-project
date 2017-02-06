/*
*	Implementazione della classe Istanza
*
*/

#include "soluzione.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

bool Soluzione::verbose = true;

Soluzione::Soluzione(Istanza* i){			// costruttore di default oppure costruttore di istanze casuali
	ist = i;
	ordinati = *(ist->getNodi());
	N = ordinati.size();
	if (verbose) cout << " Soluzione generata per i "<<N<<" punti in ordine di istanza \n";
}


int Soluzione::getN(){
	return N;
}

void Soluzione::stampa(){
//  -------	STAMPA A VIDEO I PUNTI DELLA SOLUZIONE  	--------
	cout << "\n Nodi soluzione: "<<endl;
	for(int k=0; k<ordinati.size(); k++){		
		Punto p = ordinati[k];
		cout <<" "<< k << ") " << p.stampa() << endl;
		}
	cout << endl;
	if (verbose) cout << " Stampati i "<<ordinati.size()<< " nodi della soluzione \n";
}

string Soluzione::toFileJSON(string nomeFile = ""){
// -----		STAMPA SU FILE I PUNTI IN FORMATO JSON		--------
	FILE* pFile;
	string nomeFileJSON = nomeFile;
	if (nomeFileJSON == "")
		nomeFileJSON = "soluzioneJSON_"+to_string(N)+"nodi.txt";
	pFile = fopen(nomeFileJSON.c_str(), "w");
	if (pFile!= NULL){
     	fprintf(pFile, " N = %d \n ([", N);
     	for (int i=0; i<ordinati.size(); i++){
     		fprintf(pFile, "[ %d,", ordinati[i].x);
     		if (i==ordinati.size()-1)
     			fprintf(pFile, " %d]", ordinati[i].y);			// stampo l'ultimo senza la virgola dopo le quadre
     		else
	     		fprintf(pFile, " %d],", ordinati[i].y);
     		}
		fprintf(pFile, "]) \n");
		fclose(pFile);
	}
	else
		cout << "Errore scrittura file "<< nomeFileJSON << endl;
	cout << " Generato il file " << nomeFileJSON << " con la soluzione in formato JSON | " << N << " nodi" << endl;
	return nomeFileJSON;
}

