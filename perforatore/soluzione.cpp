/*
*	Implementazione della classe Soluzione
*
*/

#include "soluzione.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

bool Soluzione::verbose = false;

Soluzione::Soluzione(Istanza* i){
//  -------	COSTRUTTORE SEMPLICE, CHE UTILIZZA L'ORDINE ATTUALE DEI NODI NELL'ISTANZA COME SOLUZIONE
	ist = i;
	N = ist->getN();
	for (int i=0; i<N; i++)
		nodiOrdinati.push_back(i);
	//ordinati = *(ist->getNodi());
	//N = ordinati.size();
	FO = calcolaFO();
	if (verbose) cout << " Soluzione iniziale generata per i "<<N<<" punti in ordine di istanza \n";
}


Soluzione::Soluzione(Istanza* i, vector<short>* v){
// ---------	COSTRUTTORE CON ARRAY DI NODI ORDINATI	---------
	ist = i;
	N = ist->getN();
	nodiOrdinati = *v;
	FO = calcolaFO();
	if (verbose) cout << " Soluzione creata con i "<<N<<" punti dati \n";
}


int Soluzione::getN(){
	return N;
}

short Soluzione::getIndicePunto(int index){
	return nodiOrdinati[index];
}


double Soluzione::getFO(){
	return FO;
}


double Soluzione::calcolaFO(){
//	-------	CALCOLA LA FUNZIONE OBIETTIVO  	-------
	double tot = 0.0;
	for(int i=0; i<nodiOrdinati.size(); i++){
		Punto A = ist->getPunto(nodiOrdinati[i]);
		Punto B(0,0);
		if ((i+1)<nodiOrdinati.size())					// calcola sempre la distanza con il successivo, a parte l'ultimo dove prende la distanza tra ultimo e primo
			B = ist->getPunto(nodiOrdinati[i+1]);
		else
			B = ist->getPunto(nodiOrdinati[0]);
		tot = tot + A.distanza(&B);
		//cout << " tot FO = " << tot << endl;
		}
	return tot;
}


void Soluzione::stampa(){
//  -------	STAMPA A VIDEO I PUNTI DELLA SOLUZIONE  	--------
	cout << "\n Nodi soluzione: "<<endl;
	for(int k=0; k<nodiOrdinati.size(); k++){		
		Punto p = ist->getPunto(nodiOrdinati[k]);
		cout <<" "<< k << ") " << p.stampa() << endl;
		}
	cout << endl;
	if (verbose) cout << " Stampati i "<<nodiOrdinati.size()<< " nodi della soluzione \n";
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
     	for (int i=0; i<nodiOrdinati.size(); i++){
     		Punto p = ist->getPunto(nodiOrdinati[i]);
     		fprintf(pFile, "[ %d,", p.x);
     		if (i==nodiOrdinati.size()-1)
     			fprintf(pFile, " %d]", p.y);			// stampo l'ultimo senza la virgola dopo le quadre
     		else
	     		fprintf(pFile, " %d],", p.y);
     		}
		fprintf(pFile, "]) \n");
		fclose(pFile);
	}
	else
		cout << "Errore scrittura file "<< nomeFileJSON << endl;
	cout << " Generato il file " << nomeFileJSON << " con la soluzione in formato JSON | " << N << " nodi" << endl;
	return nomeFileJSON;
}

