#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <curses.h>
#include <ncurses.h>
#include <termios.h>
#define FPS 50
#define HEIGH 17
#define WIDTH 13

// Color code
#define KNRM  "\x1B[0m"
#define FLRED  "\x1B[31m"
#define FLGRN  "\x1B[32m"
#define FLYEL  "\x1B[33m"
#define FLBLU  "\x1B[34m"
#define FLMAG  "\x1B[35m"
#define FLCYN  "\x1B[36m"
#define FLWHT  "\x1B[37m"
#define BLRED  "\x1B[41m"
#define BLGRN  "\x1B[42m"
#define BLYEL  "\x1B[43m"
#define BLBLU  "\x1B[44m"
#define BLMAG  "\x1B[45m"
#define BLCYN  "\x1B[46m"
#define BLWHT  "\x1B[47m"

#define FHRED  "\x1B[91m"
#define FHGRN  "\x1B[92m"
#define FHYEL  "\x1B[93m"
#define FHBLU  "\x1B[94m"
#define FHMAG  "\x1B[95m"
#define FHCYN  "\x1B[96m"
#define FHWHT  "\x1B[97m"
#define BHRED  "\x1B[101m"
#define BHGRN  "\x1B[102m"
#define BHYEL  "\x1B[103m"
#define BHBLU  "\x1B[104m"
#define BHMAG  "\x1B[105m"
#define BHCYN  "\x1B[106m"
#define BHWHT  "\x1B[107m"
// /Color code


using namespace std;

typedef struct TETRI{ //type utilisé dans chaque tuile d'une piece
	int val;
	int couleur;
}tetri;

typedef struct RAPPORT_VERIF{ //type utilisé dans chaque tuile d'une piece
	bool droite;
	bool gauche;
	bool bas;
	bool mauvais;
}rapport_verif;

tetri tet[WIDTH][HEIGH]; //grille qui va representer le terrain
char c[10]; //utilisé pour la saisie des caracteres dans le terminal
char touche; //idem

void afficher();
void jouer();

typedef struct COORD{ //type utilisé dans chaque tuile d'une piece
	int x;
	int y;
	int couleur;
}coord;


typedef struct EVT{ //les evenements sont des choses qui arrive independament des inputs du joueur, ils sont gérés dans la fonction jouer()
	unsigned int last_event;
	int duration;
	bool act;
}evenement;

evenement FALL; //piece qui tombe
evenement CLIGNE; //clignotement lorsque une ligne est completée
evenement PERDU; //fait apparaitre des lignes lorsque le joueur perd

class Piece { //La piece
	public : 
		coord shard[5]; //tableau contenant l'emplacement des tuiles
		coord next[5]; //pas encore utilisé, mais servira lorsque la piece suivante devra etre affichée
		coord ghost[5]; //tableau contenant l'emplacement des tuiles de la position la plus basse possible de la piece
		int score; //score
		Piece(); //constructeur
		rapport_verif Verifier(coord shard_veri[5]); //recupere les tuiles d'une piece en argument et renvois 1 si la piece est dans une position possible et 0 si elle est dans une position impossible
		void Translater(int dir); //Translater la piece
		void Tourner(); //Tourner la piece
		void Tomber(); //Tomber la piece
		void Terminer(); //fige la piece dans le tableau tet[][]
		void Nouveau(); //creer une nouvelle piece
		void CalcGhost();
		bool IsGhost();
		void Slam();
};

Piece::Piece() //constructeur
{
	score = 0;
	for(int i=0;i<5;i++)
	{
		shard[i].x = 0;
		shard[i].y = 0;
		next[i].x = 0;
		next[i].y = 0;
		shard[i].couleur = 41;
		next[i].couleur = 41;
	}
	this->Nouveau();
}

rapport_verif Piece::Verifier(coord shard_veri[5])
{
	rapport_verif temp_rapport;
	temp_rapport.droite = false;
	temp_rapport.gauche = false;
	temp_rapport.bas = false;
	temp_rapport.mauvais = false;
	for(int i=0;i<4;i++)
	{
		if(shard_veri[i].x > WIDTH-1) 
		{
			temp_rapport.droite = true;
			temp_rapport.mauvais = true;
		}
		if(shard_veri[i].x < 0) 
		{
			temp_rapport.gauche = true;
			temp_rapport.mauvais = true;
		}
		if(shard_veri[i].y > HEIGH-1) 
		{
			temp_rapport.bas = true;
			temp_rapport.mauvais = true;
		}
		if(shard_veri[i].y < 0) continue;
		if(temp_rapport.mauvais == true) continue;
		if(tet[shard_veri[i].x][shard_veri[i].y].val == 1) 
		{
			temp_rapport.mauvais = true;
			if(shard_veri[i].x < shard_veri[4].x) temp_rapport.gauche = true;
			if(shard_veri[i].x > shard_veri[4].x) temp_rapport.droite = true;
			if(shard_veri[i].y > shard_veri[4].y) temp_rapport.bas = true;
		}
	}
	return temp_rapport;
}

void Piece::Translater(int dir)
{
	coord temp_shard[5];
	for(int i=0;i<5;i++)
	{
		temp_shard[i].x = shard[i].x + dir;
		temp_shard[i].y = shard[i].y;
		temp_shard[i].couleur = shard[i].couleur;
	}
	rapport_verif temp_rapport = Verifier(temp_shard);
	if(temp_rapport.mauvais == false)
	{
		for(int i=0;i<5;i++)
		{
			shard[i] = temp_shard[i];
		}
	}
	CalcGhost();
}

void Piece::Tourner()
{
	coord temp_shard[5];
	rapport_verif temp_rapport;
		for(int i=0;i<5;i++)
		{
			temp_shard[i].x = (-( shard[i].y - shard[4].y)) + shard[4].x;
			temp_shard[i].y = (shard[i].x - shard[4].x) + shard[4].y;
			temp_shard[i].couleur = shard[i].couleur;
		}
		temp_rapport = Verifier(temp_shard);
		if(temp_rapport.mauvais == true)
		{
			if(temp_rapport.gauche == true)
			{
				do
				{
					for(int i=0;i<5;i++)
					{
						temp_shard[i].x++;
						temp_shard[i].y;
					}
					temp_rapport = Verifier(temp_shard);
				}while (temp_rapport.gauche == true);
			}
			else if(temp_rapport.droite == true)
			{
				do
				{
					for(int i=0;i<5;i++)
					{
						temp_shard[i].x--;
						temp_shard[i].y;
					}
					temp_rapport = Verifier(temp_shard);
				}while (temp_rapport.droite == true);
			}
			else if(temp_rapport.bas == true)
			{
				do
				{
					for(int i=0;i<5;i++)
					{
						temp_shard[i].x;
						temp_shard[i].y--;
					}
					temp_rapport = Verifier(temp_shard);
				}while (temp_rapport.bas == true);
			}
		}
		if(temp_rapport.mauvais == false)
		{
			for(int i=0;i<5;i++)
			{
				shard[i] = temp_shard[i];
			}
			CalcGhost();
		}
}

void Piece::Tomber()
{
	coord temp_shard[5];
	for(int i=0;i<5;i++)
	{
		temp_shard[i].x = shard[i].x;
		temp_shard[i].y = shard[i].y + 1;
		temp_shard[i].couleur = shard[i].couleur;
	}
	rapport_verif temp_rapport = Verifier(temp_shard);
	if(temp_rapport.mauvais == false)
	{
		for(int i=0;i<5;i++)
		{
			shard[i] = temp_shard[i];
		}
	}
	else
	{
		this->Terminer();
	}
}

void Piece::Terminer()
{
	for(int i=0;i<4;i++)
	{
		if(shard[i].x > WIDTH-1 || shard[i].x < 0 || shard[i].y > HEIGH-1) break;
		if(shard[i].y < 0) 
		{
			FALL.act = false;
			CLIGNE.act = false;
			PERDU.act = true;
			break;
		}
		tet[shard[i].x][shard[i].y].val = 1;
		tet[shard[i].x][shard[i].y].couleur = shard[i].couleur;
	}
	this->Nouveau();
}

void Piece::Nouveau()
{
	for(int i=0;i<5;i++)
	{
		shard[i].x = next[i].x;
		shard[i].y = next[i].y;
		shard[i].couleur = next[i].couleur;
	}
	int randvar = rand()%7;
	if(randvar == 0)
	{
		// #
		//#0
		// #
		next[0].x = 0+(WIDTH/2);
		next[0].y = 1-2;
		next[1].x = 1+(WIDTH/2);
		next[1].y = 1-2;
		next[2].x = 1+(WIDTH/2);
		next[2].y = 0-2;
		next[3].x = 1+(WIDTH/2);
		next[3].y = 2-2;
		next[4].x = 1+(WIDTH/2);
		next[4].y = 1-2;
		for(int i=0;i<5;i++)
		{
			next[i].couleur = 31;
		}
	}
	if(randvar == 1)
	{
		//##
		// 0
		// #
		next[0].x = 0+(WIDTH/2);
		next[0].y = 0-2;
		next[1].x = 1+(WIDTH/2);
		next[1].y = 0-2;
		next[2].x = 1+(WIDTH/2);
		next[2].y = 1-2;
		next[3].x = 1+(WIDTH/2);
		next[3].y = 2-2;
		next[4].x = 1+(WIDTH/2);
		next[4].y = 1-2;
		for(int i=0;i<5;i++)
		{
			next[i].couleur = 32;
		}
	}
	if(randvar == 2)
	{
		//##
		//0
		//#
		next[0].x = 0+(WIDTH/2);
		next[0].y = 0-2;
		next[1].x = 1+(WIDTH/2);
		next[1].y = 0-2;
		next[2].x = 0+(WIDTH/2);
		next[2].y = 1-2;
		next[3].x = 0+(WIDTH/2);
		next[3].y = 2-2;
		next[4].x = 0+(WIDTH/2);
		next[4].y = 1-2;
		for(int i=0;i<5;i++)
		{
			next[i].couleur = 33;
		}
	}
	if(randvar == 3)
	{
		//#
		//0
		//#
		//#
		next[0].x = 0+(WIDTH/2);
		next[0].y = 0-3;
		next[1].x = 0+(WIDTH/2);
		next[1].y = 1-3;
		next[2].x = 0+(WIDTH/2);
		next[2].y = 2-3;
		next[3].x = 0+(WIDTH/2);
		next[3].y = 3-3;
		next[4].x = 0+(WIDTH/2);
		next[4].y = 1-3;
		for(int i=0;i<5;i++)
		{
			next[i].couleur = 34;
		}
	}
	if(randvar == 4)
	{
		//#
		//0#
		// #
		next[0].x = 0+(WIDTH/2);
		next[0].y = 0-2;
		next[1].x = 0+(WIDTH/2);
		next[1].y = 1-2;
		next[2].x = 1+(WIDTH/2);
		next[2].y = 1-2;
		next[3].x = 1+(WIDTH/2);
		next[3].y = 2-2;
		next[4].x = 0+(WIDTH/2);
		next[4].y = 1-2;
		for(int i=0;i<5;i++)
		{
			next[i].couleur = 35;
		}
	}
	if(randvar == 5)
	{
		// #
		//#0
		//#
		next[0].x = 1+(WIDTH/2);
		next[0].y = 0-2;
		next[1].x = 1+(WIDTH/2);
		next[1].y = 1-2;
		next[2].x = 0+(WIDTH/2);
		next[2].y = 1-2;
		next[3].x = 0+(WIDTH/2);
		next[3].y = 2-2;
		next[4].x = 1+(WIDTH/2);
		next[4].y = 1-2;
		for(int i=0;i<5;i++)
		{
			next[i].couleur = 36;
		}
	}
	if(randvar == 6)
	{
		//0#
		//##
		next[0].x = 0+(WIDTH/2);
		next[0].y = 0-1;
		next[1].x = 1+(WIDTH/2);
		next[1].y = 0-1;
		next[2].x = 0+(WIDTH/2);
		next[2].y = 1-1;
		next[3].x = 1+(WIDTH/2);
		next[3].y = 1-1;
		next[4].x = 0+(WIDTH/2);
		next[4].y = 0-1;
		for(int i=0;i<5;i++)
		{
			next[i].couleur = 37;
		}
	}
	bool flag = true;
	int cummul = 1;
	for(int j=0;j<HEIGH;j++) //verifie si une ligne est completée
	{
		flag = true;
		for(int i=0;i<WIDTH;i++) 
		{
			if(tet[i][j].val == 0) flag = false;
		}
		if(flag == true) //ligne completée
		{
			score += cummul*100;
			cummul++;
			for(int i=0;i<WIDTH;i++)
			{
				tet[i][j].val = 2;
			}
			FALL.act = false;
			CLIGNE.act = true;
		}
	}
	this->CalcGhost();
}

void Piece::CalcGhost()
{
	coord temp_shard[5];
	rapport_verif temp_rapport;
	int u=0;
	do
	{
		u++;
		for(int i=0;i<5;i++)
		{
			temp_shard[i].x = shard[i].x;
			temp_shard[i].y = shard[i].y+u;
			temp_shard[i].couleur = shard[i].couleur;
		}
		temp_rapport = Verifier(temp_shard);
	} while (temp_rapport.mauvais == false);
	for(int i=0;i<5;i++)
	{
		ghost[i].x = temp_shard[i].x;
		ghost[i].y = temp_shard[i].y-1;
		ghost[i].couleur = temp_shard[i].couleur;
	}
}

bool Piece::IsGhost()
{
	for(int i=0;i<4;i++)
	{
		if(ghost[i].x != shard[i].x || ghost[i].y != shard[i].y) return false;
	}
	return true;
}

void Piece::Slam()
{
	if(this->IsGhost() == false);
	{
		for(int i=0;i<5;i++)
		{
			shard[i].x = ghost[i].x;
			shard[i].y = ghost[i].y;
		}
		FALL.last_event = 0;
	}
}


Piece chunk; //la piece chunk est celle utilisé dans le programme

void changemode(int dir) //truc a william pour recevoir les inputs sans attendre
{
  static struct termios oldt, newt;
 
  if ( dir == 1 )
  {
    	tcgetattr( STDIN_FILENO, &oldt);
    	newt = oldt;
		newt.c_lflag &= ~( ICANON | ECHO);
    	tcsetattr( STDIN_FILENO, TCSANOW, &newt);
  }
  else
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}

int kbhit (void) //truc a william pour recevoir les inputs sans attendre
{
  	struct timeval tv;
  	fd_set rdfs;
 
 	tv.tv_sec = 0;
 	tv.tv_usec = 0;
 
 	FD_ZERO(&rdfs);
  	FD_SET (STDIN_FILENO, &rdfs);
  	select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
 	return FD_ISSET(STDIN_FILENO, &rdfs);
 
}

int checkKey(int key) //truc a william pour recevoir les inputs sans attendre (pas utilisé)
{
	return ((char)(key) == 'z') || ((char)(key) == 'q') || ((char)(key) == 's') || ((char)(key) == 'd');
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	clock_t t; //pour le calcul des FPS
	bool flag = false; //euh
	FALL.last_event = 0; //init de l'evenement FALL
	FALL.duration = FPS/2;
	FALL.act = true;
	CLIGNE.last_event = 0; //init de l'evenement CLIGNE
	CLIGNE.duration = FPS;
	CLIGNE.act = false;
	PERDU.last_event = 0; //init de l'evenement PERDU
	PERDU.duration = FPS*4;
	PERDU.act = false;
	for(int j=0;j<HEIGH;j++) //init du tableau tet[][]
	{
		for(int i=0;i<WIDTH;i++)
		{
			tet[i][j].val = 0;
			tet[i][j].couleur = 47;
		}
	}
	chunk.Nouveau(); //on creer une nouvelle piece au debut
	while(1) //boucle principale
	{
		t = clock(); //on demarre un chronometre
		system("clear"); //efface le terminal
		changemode(1); //truc a william pour recevoir les inputs sans attendre
		flag = false; //euh
		if(kbhit())
		{
			read(STDIN_FILENO, c, 10); //on prend 10 caracteres au max dans le terminal (meme si l'utilisateur n'a pas appuyé sur "entrer"
			flag = true;
		}
		changemode(0); //truc a william pour recevoir les inputs sans attendre
		if(flag == true) touche = c[0]; //si ya pas eu d'input flag == false
		else touche = -1;
		jouer(); //boucle de gestion des evenements
		afficher(); //affichage

		t = clock() - t; //on arrete le chronometre, t contient la durée d'execution du programme
		int SLPTIME; //On veux faire un certain nombre de FPS par seconde. 
					 //Si on en veux 50, chaque Frame dure 20ms. 
					 //or le prog dure beaucoup moins longtemps que ca. 
					 //donc on utilise un chrono pour savoir combien de temps le prog a duré, puis on met le prog en sommeil pour qu'il se reveille 20ms apres que le chrono ai commencé.
					 //ex : si le prog dure 3ms, on le met en sleep() pendant 17ms
		//long int truefps;
		if (t==0) //si t==0
		{
			SLPTIME = (int)((1/(float)FPS)*1000000);
			//truefps = -1;
		}
		else //si t!=0
		{
			SLPTIME = (int)(((1/(float)FPS) - (float)t/CLOCKS_PER_SEC)*1000000);
			//truefps = CLOCKS_PER_SEC/(float)t;
		}
		printf("SLPTIME : %d\n", SLPTIME);
		usleep(SLPTIME);
	}
	return 0;
}

void jouer() //ici on gere les evenements, a chaque frame on regarde ce qui doit ce passer (pour l'instant on check juste si la piece doit tomberm, si une ligne doit clignoter ou si la partie est perdue)
{
	printf("FALL.last_event : %d\nX : %d  Y : %d\n", FALL.last_event, chunk.shard[4].x, chunk.shard[4].y);
	if(FALL.act == true) //act = variable qui dit si un evenement est actif
	{
		if(touche == 'a') chunk.Translater(-1);
		if(touche == 'd') chunk.Translater(1);
		if(touche == 'w') chunk.Tourner();
		if(touche == ' ') chunk.Slam();
		if(touche == 's') FALL.duration = (FPS/25)-1;
		else FALL.duration = FPS/2-(chunk.score/(5000/(FPS/2)));
		if(FALL.last_event++ > FALL.duration) //si le compteur de FALL est atteint, on fait tomber la piece d'une ligne
		{
			FALL.last_event = 0; //last event est afficher dans le terminal pour plus de comprehension
			chunk.Tomber();
		}
	}
	else if(CLIGNE.act == true) //ca cligne quand une ligne est complétée
	{
		if(CLIGNE.last_event++ > CLIGNE.duration) //cligne est fini, on efface les lignes concernées
		{
			CLIGNE.last_event = 0;
			CLIGNE.act = false;
			for(int j=0;j < HEIGH;j++)
			{
				if(tet[0][j].val == 2 || tet[0][j].val == -2)
				{
					for(int u=j;u >= 0;u--)
					{
						for(int i=0;i<WIDTH;i++)
						{
							if(u-1 < 0) 
							{
								tet[i][u].val = 0;
								tet[i][u].couleur = 47;
							}
							else 
							{
								tet[i][u].val = tet[i][u-1].val;
								tet[i][u].couleur = tet[i][u-1].couleur;
							}
						}
					}
				}

			}
			FALL.act = true;
		}
		else if(CLIGNE.last_event%(FPS/2) >= FPS/4)
		{
			for(int j=0;j<HEIGH;j++)
			{
				for(int i=0;i<WIDTH;i++)
				{
					if(tet[i][j].val == -2) tet[i][j].val = 2;
				}
			}
		}
		else if(CLIGNE.last_event%(FPS/2) < FPS/4)
		{
			for(int j=0;j<HEIGH;j++)
			{
				for(int i=0;i<WIDTH;i++)
				{
					if(tet[i][j].val == 2) tet[i][j].val = -2;
				}
			}
		}
	}
	else if(PERDU.act == true) //affiche des lignes qui montent
	{
		if(PERDU.last_event <= PERDU.duration)
		{
			if(PERDU.last_event < PERDU.duration/2)
			{
				for(int j=HEIGH-1;j>=0;j--)
				{
					if(HEIGH-1-j < ((float)(PERDU.last_event) / (PERDU.duration/2)) * HEIGH)
					{
						for(int i=0;i<WIDTH;i++)
						{
							tet[i][j].val = 3;
							tet[i][j].couleur = 47;
						}
					}
				}
			}
			else if(PERDU.last_event >= PERDU.duration/2)
			{
				for(int j=0;j<HEIGH;j++)
				{
					if(j < ((float)(PERDU.last_event-(PERDU.duration/2)) / (PERDU.duration/2)) * HEIGH)
					{
						for(int i=0;i<WIDTH;i++)
						{
							tet[i][j].val = 0;
							tet[i][j].couleur = 47;
						}
					}
				}
			}
			PERDU.last_event++;
		}
		else if(touche != -1)
		{
			PERDU.act = false;
			PERDU.last_event = 0;
			chunk.score = 0;
			FALL.act = true;
			chunk.CalcGhost();
		}
	}
}

void afficher()
{
	bool flag = false;
	for(int j=0;j<HEIGH;j++)
	{
		printf("#");
		for(int i=0;i<WIDTH;i++)
		{
			if(FALL.act == true)
			{
				for(int u=0;u<4;u++)
				{
					if(i==chunk.shard[u].x && j==chunk.shard[u].y) 
					{
						printf("\x1B[%dm", chunk.shard[u].couleur);
						printf("@"KNRM);
						//printf("\x1B[31mO\x1B[0m");
						flag = true;
						break;
					}
				}
				if(flag == false) 
				{
					for(int u=0;u<4;u++)
					{
						if(i==chunk.ghost[u].x && j==chunk.ghost[u].y) 
						{
							printf("\x1B[%dm", chunk.shard[u].couleur);
							printf("X"KNRM);
							//printf("\x1B[31mO\x1B[0m");
							flag = true;
							break;
						}
					}
				}
			}
			if(flag == false)
			{
				if(tet[i][j].val==1 || tet[i][j].val==2)
				{
					printf("\x1B[%dm", tet[i][j].couleur);
					printf("@"KNRM);
				}
				else if(tet[i][j].val==3)
				{
					printf("\x1B[37m");
					printf("#"KNRM);				
				
				}
				else printf(" ");
			}
			
			flag = false;
		}
		printf("#");
		if(j==0) printf("Score : %d\n", chunk.score);
		else if(j==2) printf("####\n");
		else if(j>2 && j<7) 
		{
			printf(" ");
			for(int v=0;v<3;v++)
			{
				flag = false;
				if(PERDU.act == false)
				{
					for(int u=0;u<4;u++)
					{
						if(chunk.next[u].x-(WIDTH/2) == v && chunk.next[u].y+3 == j-3)
						{
							printf("\x1B[%dm", chunk.next[u].couleur);
							printf("@"KNRM);
							flag = true;
							break;
						}
					}
				}
				if(flag == false) printf(" ");
			}
			printf("#\n");
		}
		else if(j==7) printf("#####\n");
		else puts("");
	}
	printf("###############\n");
}
