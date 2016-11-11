#include <iostream>
#include <sstream> 
#include <cstdlib>
#include <string.h>
#include <math.h>
#include <random>

using namespace std;

class Punto{
public:
	int x, y;
	Punto(int, int);						// creazione punto standard
	//Punto(int, int, int, int);			// creazione punto in un determinato range ??
	double distanza(Punto*);		// calcola la distanza assoluta tra due punti
	string stampa();
	bool operator== (const Punto&) const;
};
