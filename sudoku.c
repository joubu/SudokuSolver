/*  
    Copyright (C) Jonathan Druart
 
    This Program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file licence.txt.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define appartient(x,e) ((1<<x)&E)
#define UMASQ 65513
#define TRUE  1
#define FALSE 0
#define _DEBUG 1


typedef unsigned short** tSudoku;
typedef unsigned char uchar;
typedef unsigned short ushort;

typedef uchar bool;
typedef struct {ushort i; ushort j;} indicesStruct;

int profondeur = 0;

//Retourne le poids de l'élément passé en paramètre
ushort Poids(ushort E){
	ushort poids = 0;
	while(E > 0){
		E &= E - 1;
		poids ++;
	}
	return poids;
}

//caseij doit être une puissance de 2, donc avec un poids=1
//retourne le nombre correspondant à la puissance de 2
ushort ConvertCase(ushort caseij){
	ushort inc=0;
	while(caseij > 0){
		caseij=caseij >> 1;
		inc++;
	}
	return inc-1;
}

//Retourne 1022 (=1111111110) si la case=0 sinon retourne la puissance de 2 correspondante
ushort Vect(ushort caseij){
	if(caseij == 0){
		return 1022;
	}else{
		return (1 << caseij);
	}
}

//Alloue l'espace mémoire à la grille (9 * 9)
void AllouerS(tSudoku* S){
	uchar i;
	*S = malloc(sizeof(ushort*) * 9);
	for(i = 0 ; i <9 ; i++){
		(*S)[i] = malloc(sizeof (ushort) * 9);
	}
}

//Libère l'espace mémoire allouée à la grille
void FreeS(tSudoku* S){
	ushort i ;
	for ( i = 0 ; i < 9 ; i++ ) {
		free((*S)[i]) ;
	}
	free((*S)) ;
}

//Initialise la matrice S avec le fichier passé en argument
int InitS(tSudoku* S, char* nomFic){
	ushort i = 0, j = 0;
	FILE *fic;
	fic = fopen(nomFic, "r");
	if (fic != NULL){
		for(i = 0 ; i < 9 ; i++)
			for (j = 0 ; j < 9 ; j++) {
				fscanf(fic, "%hu", &((*S)[i][j]));
				(*S)[i][j] = Vect ((*S)[i][j]);
			}
		fclose(fic);
		return 1;
	}else{
		printf("Le fichier %s n'a pas été ouvert, pb de droit ou n'existe pas\n",nomFic);
		return 0;
	}
}

//Retourne le nombre de cases qui ne sont toujours pas résolues dans la grille
ushort NbCasesRestantes(tSudoku S){
	ushort nbCasesRestantes = 9 * 9;
	ushort i = 0 , j = 0;
	for (i = 0 ; i < 9 ; i++){
		for(j = 0 ; j < 9 ; j++){
			if(Poids(S[i][j]) == 1){
				nbCasesRestantes --;
			}
		}
	}
	return nbCasesRestantes;
}

//Affiche un nombre en binaire
void binaire(int nb){
	while (nb>0){
		nb=nb>>1;
		printf("%d",nb%2);
	}
	//printf("\n");
}

//Affiche la matrice S (9 * 9) sans convertir les cases inconnues par 0
void Afficher_test(tSudoku S){
	ushort i=0,j=0;
	for(i = 0 ; i < 9 ; i++){
		printf("|");
		for(j = 0 ; j < 9 ; j++){
			printf(" %hu |",S[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

//Affiche la matrice S (9 * 9)
void Afficher(tSudoku S){
	ushort i=0,j=0;
	for(i = 0 ; i < 9 ; i++){
		printf("|");
		for(j = 0 ; j < 9 ; j++){
			if (Poids(S[i][j]) == 1){
				printf(" %hu |", ConvertCase(S[i][j]));
			}else{
				printf(" 0 |");
			}
		}
		printf("\n");
	}
	printf("\n");
}

//Copie une grille (src) de sudoku dans une autre (dest)
void CopieS(tSudoku src, tSudoku dest){
	ushort i,j;
	for(i = 0 ; i < 9 ; i++){
		for(j = 0 ; j < 9 ; j++){
			dest[i][j] = src[i][j];
		}
	}
}

//Compare 2 grilles de sudoku, retourne 1 si les 2 grilles sont identiques, sinon 0
ushort ComparaisonS(tSudoku S1, tSudoku S2){
	ushort i,j;
	for(i = 0 ; i < 9 ; i++){
		for(j = 0 ; j < 9 ; j++){
			if(S1[i][j] != S2[i][j]){
				return 0; // Si les 2 grilles diffèrent
			}
		}
	}
	return 1; // Si les 2 grilles sont identiques
}

//retourne true si le poids de la case est = 1 sinon FALSE
bool CaseOk(ushort caseij){
	bool booleen;
	if(Poids(caseij) == 1)
		booleen = TRUE;
	else
		booleen = FALSE;
	
	return booleen;
}

//Retourne l'indice de la prochaine case inconnue suivante, s'il n'en existe pas retourne 99
indicesStruct CaseSuiv(tSudoku S, indicesStruct ind){
	indicesStruct suiv;
	ushort i,j;
	if (ind.j == 8) {
		j = 0 ;
		i = ind.i + 1 ;
	}
	else {
		i = ind.i ;
		j = ind.j + 1 ;
	}
	for(;i<9;i++){
		for(;j<9;j++){
			if(Poids(S[i][j])!=1){
				suiv.i=i;
				suiv.j=j;
				return suiv;
			}
		}
		j=0;
	}
	suiv.i=99;
	suiv.j=99;
	return suiv;
}

// Retourne l'indice de la case possédant le moins de possibilités
indicesStruct IndicesMin(tSudoku S){
	unsigned short i,j;
	indicesStruct indicesMin;
	indicesMin.i=0;
	indicesMin.j=0;
	indicesMin=CaseSuiv(S,indicesMin);

	for (i = 0 ; i < 9 ; i++){
		for (j = 0 ; j < 9 ; j++){
			if ((Poids (S[i][j]) > 1) && (Poids(S[i][j]) < Poids(S[indicesMin.i][indicesMin.j]))){
				indicesMin.i = i;
				indicesMin.j = j;
			}
		}
	}
	return indicesMin;
}

// Retourne le nombre correspondant à l'hypothèse suivante d'une case
ushort HypSuiv(ushort caseij, ushort n){
	ushort hyp = 0,cpt = 0;

	for (hyp = 1 ; hyp <= 9 ; hyp++){
		if (Poids (caseij & Vect(hyp)) > 0){
			if (cpt == n){
				return hyp;
			}else{
				cpt ++;
			}
		}
	}
	return 0;
}

// Contrainte sur une ligne
void ContrainteDeLigne(tSudoku S, ushort i, ushort j){
	ushort c = 0;
	for (c = 0 ; c < 9 ; c++){
		if ((c!=j) && (CaseOk(S[i][c])) && (Poids(S[i][j] ^ S[i][c]) < Poids(S[i][j]))){
			S[i][j] = S[i][j] ^ S[i][c];
		}
	}
}

// Contrainte sur une colonne
void ContrainteDeColonne(tSudoku S, ushort i, ushort j){
	ushort l = 0;
	for (l=0; l<9; l++){
		if ((l != i) && (CaseOk(S[l][j])) && (Poids(S[i][j] ^ S[l][j]) < Poids(S[i][j]))){
			S[i][j] = S[i][j] ^ S[l][j];
		}
	}
}

// Contrainte dans un carré
void ContrainteDeCarre(tSudoku S,  ushort i, ushort j){
	ushort k = 0, l = 0;
	for (k=((i/3)*3); k<(((i/3)*3)+3); k++){
		for (l=((j/3)*3); l<(((j/3)*3)+3); l++){
			if ((k != i && l != j) && (CaseOk(S[k][l])) && (Poids(S[i][j] ^ S[k][l]) < Poids(S[i][j]))){
				S[i][j] = S[i][j] ^ S[k][l];
			}
		}
	}
}

// Retourne TRUE si un nombre nb est présent sur une ligne i sinon FALSE
bool NbSurLigne(tSudoku S, ushort i, ushort nb){
	ushort j;
	for(j = 0 ; j < 9 ; j++){
		if(ConvertCase(S[i][j]) == nb)
			return TRUE;
	}
	return FALSE;
}

// Retourne TRUE si un nombre nb est présent sur une colonne j sinon FALSE
bool NbSurColonne(tSudoku S, ushort j, ushort nb){
	ushort i;
	for(i = 0 ; i < 9 ; i++){
		if(ConvertCase(S[i][j]) == nb)
			return TRUE;
	}
	return FALSE;
}

// Retourne TRUE si un nombre nb est présent dans un carré
bool NbSurCarre(tSudoku S, ushort i, ushort j, ushort nb){
	ushort k,l;
	for (k=((i/3)*3); k<(((i/3)*3)+3); k++){
		for (l=((j/3)*3); l<(((j/3)*3)+3); l++){
			if (ConvertCase(S[k][l]) == nb){
				return TRUE;
			}
		}
	}
	return FALSE;
}

// Contrainte pour une possibilité unique sur une ligne
void ContrainteUniqueLigne(tSudoku S, ushort l, ushort c){
	ushort i = 0, j = 0, nbHyp = 0, hypothese;
	bool hypOk = TRUE;
	nbHyp = Poids(S[l][c]);
	
	for (i = 0 ; i < nbHyp ; i++){
		hypOk = TRUE;

		// On passe à l'hypothès suivante
		hypothese = Vect(HypSuiv(S[l][c], i));

		for (j=0 ; j < 9 ; j++){
			//Si différent de la case courante, que son poids!=1 (=pas une puissance de 2) et que l'hypothèse appartient à S[l][j] alors l'hypothèse n'est pas certaine
			if ((j != c) && (Poids(S[l][j]) != 1) && ((hypothese & S[l][j]) > 0)){
				hypOk=FALSE;
			}
		}
		// Si on a pas trouvé de contradiction avec cette hypothèse, on affecte la case
		if (hypOk == TRUE) {
			S[l][c] = hypothese;
			return;
		}
	}
}

// Contrainte pour une possibilité unique sur une colonne
void ContrainteUniqueColonne(tSudoku S, ushort l, ushort c){
	ushort i = 0, j = 0, nbHyp = 0, hypothese;
	bool hypOk = TRUE;
	nbHyp = Poids(S[l][c]);
	
	for (j = 0 ; j < nbHyp ; j++){
		hypOk = TRUE;
		hypothese = Vect(HypSuiv(S[l][c], j));
		for (i=0 ; i < 9 ; i++){
			// Si différent de la case courante, que son poids!=1 (=pas une puissance de 2) et que l'hypothèse appartient à S[l][j] alors l'hypothèse n'est pas certaine
			if ((i != l) && (Poids(S[i][c]) != 1) && ((hypothese & S[i][c]) > 0)){
				hypOk=FALSE;
			}
		}
		// Si l'hypothèse n'a pas été contredit, on affecte la case avec l'hypothèse faite
		if (hypOk == TRUE) {
			S[l][c] = hypothese;
			return;
		}
	}

}

// Contrainte pour une possibilité unique dans un carré
void ContrainteUniqueCarre(tSudoku S, ushort l, ushort c){
	ushort i = 0, iStop = 0, j = 0, jStop = 0, k = 0, nbHyp = 0, hypothese;
	bool hypOk = TRUE;
	nbHyp = Poids(S[l][c]);

	for (k = 0 ; k < nbHyp ; k++){
		hypOk = TRUE;
		hypothese = Vect(HypSuiv(S[l][c], k));
		i = (l/3)*3; iStop = i + 3;
		j = (c/3)*3; jStop = j + 3;
		for (; i < iStop ; i++){
			for (; j < jStop ; j++){
				if (((i != l)||(j != c)) && (Poids(S[i][j]) != 1) && ((hypothese & S[i][j]) > 0)){
					hypOk=FALSE;
				}
			}
			j=jStop-3;
		}
		if (hypOk == TRUE){
			S[l][c] = hypothese;
			return;
		}
	}
}

/* Fonction de résolution du Sudoku */
bool ResoudS(tSudoku S){
	#ifdef _DEBUG
	printf("cases restantes : %d\n",NbCasesRestantes(S));
	#endif

	bool backtracking = FALSE; //backtracking = 1 lorsque l'on a trouvé au moins une case pour un parcours de matrice

	bool sudokuResolu = FALSE; // Passera à TRUE si le sudoku à été résolu

	while(!sudokuResolu){
		backtracking = FALSE;
		indicesStruct indC; // représente l'indice courant
		indC.i = 0;
		indC.j = 0;

		if(Poids(S[0][0])==1){
			indC=CaseSuiv(S, indC); // On recherche la case suivante qui est inconnue
		}

		if((indC.i == 99) || (indC.j == 99)) sudokuResolu = 1; // S'il n'y a plus de case inconnue, on a résolu le sudoku

		else{

		while(indC.j != 99){
			// On passe le sudoku par les différentes contraintes
			ContrainteDeLigne(S, indC.i, indC.j);
			ContrainteDeColonne(S, indC.i, indC.j);
			ContrainteDeCarre(S, indC.i, indC.j);
			ContrainteUniqueLigne(S, indC.i, indC.j);
			ContrainteUniqueColonne(S, indC.i, indC.j);
			ContrainteUniqueCarre(S, indC.i, indC.j);

			// Si le poids de l'indice courant == 1, alors une case à bien été trouvée
			if(Poids(S[indC.i][indC.j]) == 1){
				backtracking = TRUE;
			}

			// Si une case  vaut zéro, on a pas réussi à résoudre le sudoku
			if(!S[indC.i][indC.j]){
				return 0;
			}

			// On passe à la case suivante
			indC = CaseSuiv(S,indC);
		}

		//Si on a pas trouvé de case supplémentaire lors du dernier balayage de la grille, backtracking
		if(!backtracking){
			#ifdef _DEBUG
			printf("--- BACKTRACKING ---\n");
			#endif

			profondeur ++; // Calcul de la profondeur du backtraking
			printf("Backtracking de niveau %d\n",profondeur);

			ushort i, hypothese, nbHyp;
			indicesStruct indMin = IndicesMin(S);

			#ifdef _DEBUG
			printf("La case qui possède le moins de solutions : S[%d][%d] = %d\n", indMin.i, indMin.j, S[indMin.i][indMin.j]);
			#endif
			
			nbHyp = Poids(S[indMin.i][indMin.j]); // Calcul du nombre d'hypothèses de la case possédant le moins d'hypothèses

			// Création d'une grille temporaire
			tSudoku Sbid;
			AllouerS(&Sbid);

			// On teste pour chaque hypothèse si elle aboutit à une résolution
			for (i = 0 ; i < nbHyp ; i++){
				hypothese = Vect(HypSuiv(S[indMin.i][indMin.j], i)); // Recherche de l'hypothèse suivante

				#ifdef _DEBUG
				printf("Hypothèse faite ! -> S[%d][%d] =? %d\n", indMin.i, indMin.j, hypothese);
				#endif

				CopieS(S, Sbid); // On initialise la grille temporaire

				Sbid[indMin.i][indMin.j]=hypothese; // On teste avec l'hypothèse courante

				if(hypothese){
					if (ResoudS(Sbid)){
						#ifdef _DEBUG
						printf("Hypothèse confirmée ! -> S[%d][%d] = %d\n", indMin.i, indMin.j, hypothese);
						#endif

						S[indMin.i][indMin.j] = hypothese; // On affecte la case qui possédait le moins de possibilités avec l'hypothèse correspondante

						i = nbHyp; // La grille se résoud On peut s'arreter, on arrête la boucle
					}
				}else{
					printf("La grille n'a pas l'air d'être résolvable :-( \n");
					return FALSE;
				}
			}
			// On libère l'espace alloué à la grille temporaire
			FreeS(&Sbid);
		}
		#ifdef _DEBUG
		printf("\ncases restantes : %d\n",NbCasesRestantes(S));
		Afficher(S);
		#endif
	}
	}
	return 1;
}



int main(int argc, char* argv[]){
	if(argc == 2){
		char *nomFic;
		nomFic=argv[1];

		tSudoku S;
		AllouerS (&S);
		
		if (!InitS(&S, nomFic)){
			printf("La grille n'a pas été bien initialisée\n");
			return -2; // return -2 si la grille n'a pas été bien initialisée
		}else{
			printf("Grille de sudoku à résoudre : \n");
			Afficher(S);
			printf("cases restantes : %d\n\n\n",NbCasesRestantes(S));
			if(ResoudS(S)){
				printf("Grille de sudoku résolue avec un backtraking de niveau %d: \n", profondeur);
				Afficher(S);
				printf("cases restantes : %d\n\n",NbCasesRestantes(S));
			}else{
				printf("KO\n");
				FreeS(&S);
				return -2;
			}
		}

		FreeS(&S);
	
	}else{
		printf("Utilisation : ./sudoku fichier\n");
		return -1;
	}


	return 0;
}
