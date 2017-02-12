
#ifndef SEGMENTO_H
#define SEGMENTO_H

#include <iostream>
#include <sstream> 
#include <cstdlib>
#include <string.h>
#include "punto.h"

using namespace std;

class Segmento{
public:
	Punto da, a;
	Segmento(Punto, Punto);					// creazione segmento passando 2 Punti diversi
	double getLunghezza();					// restituisce la lunghezza del segmento
	//string stampa();			TODO: se ne ho tempo
	bool operator== (const Segmento&) const;
private:
	double lunghezza;						// la lunghezza del segmento tramite il metodo distanza della classe Punto	
};

#endif // SEGMENTO_H
