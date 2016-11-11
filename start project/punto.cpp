#include "punto.h"

Punto::Punto(int posx=-1, int posy=-1):x(posx),y(posy){			// creazione punto standard
	if (x==-1)
		x = rand();
	if (y==-1)
		y = rand();
	}

double Punto::distanza(Punto* B){			// distanza dal punto B
	return sqrt(pow(fabs(x-B->x),2)+pow(fabs(y-B->y),2));
	}

string Punto::stampa(){
	ostringstream out;
	out << "Punto: (" << x << ", " << y << ")";
	return out.str();
	}

bool Punto::operator==(const Punto &p) const {
        return (x==p.x && y==p.y);
    }
