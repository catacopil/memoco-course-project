

#include "segmento.h"

Segmento::Segmento(Punto A, Punto B):da(A),a(B){			// creazione segmento con 2 oggetti Punto differenti
	if (da == a){
		string m = " Creazione segmento con 2 punti identici!";
		m = m + da.stampa() + " --> "+a.stampa();
		throw runtime_error(m);
		}
	lunghezza = da.distanza(&a);
	}

double Segmento::getLunghezza(){					// distanza dal punto "da" al punto "a"
	return lunghezza;
	}


string Segmento::stampa(){
	ostringstream out;
	out << " " << da.stampa() << "->" << a.stampa() << "| l="<<lunghezza;
	return out.str();
	}

bool Segmento::operator==(const Segmento &elle) const {				// il test di uguaglianza controlla se i due punti coincidono da==da a==a 
        return (da==elle.da && a==elle.a);
    }


Punto Segmento::getDa(){
	return da;
}

Punto Segmento::getA(){
	return a;
}

void Segmento::gira(){
	Punto temp = da;
	da = a;
	a = temp;
}
