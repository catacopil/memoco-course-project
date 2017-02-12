

#include "segmento.h"

Segmento::Segmento(Punto A, Punto B):da(A),a(B){			// creazione segmento con 2 oggetti Punto differenti
	if (da == a)
		throw runtime_error(" Creazione segmento con 2 punti identici!");
	lunghezza = da.distanza(&a);
	}

double Segmento::getLunghezza(){					// distanza dal punto "da" al punto "a"
	return lunghezza;
	}

/* TODO: da sistemare per il segmento se serve e se ho tempo
string Punto::stampa(){
	ostringstream out;
	out << " (" << x << ", " << y << ") ";
	return out.str();
	} */

bool Segmento::operator==(const Segmento &elle) const {				// il test di uguaglianza controlla se i due punti coincidono da==da a==a 
        return (da==elle.da && a==elle.a);
    }

