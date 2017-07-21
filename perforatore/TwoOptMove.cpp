/*
*	Implementazione della classe TwoOptMove
*
*/

#include "TwoOptMove.h"
#include <iostream>

bool TwoOptMove::verbose = false;

TwoOptMove::TwoOptMove(Istanza* i, Soluzione* s, int primo, int secondo){
	ist = i;
	sol = s;
	primoSegmento = primo;							// il primo segmento è definito dai punti di indice (nella soluzione) primoSegmento e (primoSegmento+1)
	secondoSegmento = secondo;						// il secondo segmento è definito dai punti di indice (nella soluzione) secondoSegmento e (secondoSegmento+1)
	valida = false;								// la mossa non è valida, fino a prova contraria
	miglioramento = -1;
	
	short indexA = sol->getIndicePunto(primoSegmento);
	short indexB = sol->getIndicePunto(primoSegmento+1);
	short indexC = sol->getIndicePunto(secondoSegmento);
	short indexD = sol->getIndicePunto(secondoSegmento+1);
	
	// CONTROLLO SE GLI INDICI SONO RAGIONEVOLI
	if (indexA == indexC || indexB == indexD || indexB == indexC ){
		if (verbose) cout << " ATTENZIONE: indici incoerenti: A="<< indexA << " B=" << indexB << " C=" << indexC << " D=" << indexD << endl;
		//if (verbose) cout << "NO ";
		return;
	}
	// altrimenti mossa valida, anche se peggiora
	valida = true;
	// CONTROLLO SE C'È UN MIGLIORAMENTO
	
	Punto A = ist->getPunto(indexA);
	Punto B = ist->getPunto(indexB);
	Punto C = ist->getPunto(indexC);
	Punto D = ist->getPunto(indexD);
		
	double attualeDist = A.distanza(&B) + C.distanza(&D);
	double nuovaDist = A.distanza(&C) + B.distanza(&D);
	
	//if (nuovaDist < attualeDist){
		
		miglioramento = attualeDist - nuovaDist;
		if (verbose) cout << " Trovata mossa 2-opt che porta miglioramento di "<< miglioramento << " | A="<< indexA << " B=" << indexB << " C=" << indexC << " D=" << indexD << endl;
	//}
	
}

int TwoOptMove::getPrimoSegmento(){
	return primoSegmento;
}

int TwoOptMove::getSecondoSegmento(){
	return secondoSegmento;
}
