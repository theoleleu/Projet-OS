#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <wordexp.h>
#include <unistd.h> //getpid, fork
#include <string.h> //strcmp, strcat
#include <stdio.h>  //printf
#include <stdlib.h> //exit, atoi

int max(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}

int est(char c, char *chaine)
{
	int est = 0;
	for (int i = 0; i < strlen(chaine); i++)
		if (c == chaine[i])
			est = 1;
	return est;
}

/*nom des fichiers séparés d'un espace*/
char *lect(char *line, char *tab)
//On souhaite rajouter l'option de globbing
{
	int ttab = strlen(tab) + 1;
	int tline = strlen(line) + 1;
	char *newtab = malloc((ttab + tline) * sizeof(char));
	newtab = strcat(newtab, tab);
	newtab = strcat(newtab, " ");
	if (line[tline-2] != '\n')
	return strcat(newtab, line + 2);
	return strncat(newtab, line + 2, strlen(line + 3)); //On enleve les 2 premiers char et les 2 derniers (\n)
}

char **cutstrtok(char *line, int *linetablen, int mult)
{	
	int taillemax = strlen(line)*mult;
	char *linecopy = malloc(strlen(line));
	strcpy(linecopy, line);							    //linecopy permet de ne pas écraser la valeur de line avec strtok
	char **linetab = malloc(taillemax* sizeof(char*)); //linetab est la chaine line décomposée en char**
	char *delim = " ";
	char *current_str_in_line = strtok(linecopy, delim);
	int index_linetab = 0;
	printf("%s la chaîne est transformée en le tableau : ", line);
	while (current_str_in_line)
	{
		if (est('*', current_str_in_line)) // Le globbing
		//S'il on reconnait * alors il faut ajouter tous les mots correspondants au tableau
		{
			wordexp_t p;
			wordexp(current_str_in_line, &p, 0);
			if ((index_linetab+p.we_wordc+1)>taillemax) {
				printf("Taille insuffisante, on recommence.\n");
				return cutstrtok(line,linetablen,mult*10);
			}
			for (int i = 0; i < p.we_wordc; i++)
			{
				linetab[index_linetab] = malloc(strlen(p.we_wordv[i]));
				strcpy(linetab[index_linetab], p.we_wordv[i]);
				index_linetab++;
				printf("%s ", p.we_wordv[i]);
			}
			wordfree(&p);
		}
		else
		{ //Sans le globbing, facile
			linetab[index_linetab] = current_str_in_line;
			index_linetab++;
			if ((index_linetab+1)>taillemax) {
				printf("Taille insuffisante, on recommence %d",10*mult);
				return cutstrtok(line,linetablen,mult*10);
			}
			printf("%s ", current_str_in_line);
		}
		current_str_in_line = strtok(NULL, delim);
	}
	linetablen[0] = index_linetab;
	printf("\n");

	//Suppression doublons causés par les * successifs
	for (int i = 0; i < linetablen[0]; i++)
	{
		for (int j = i + 1; j < linetablen[0];)
		{
			if (/*(strcmp("fari.c", linetab[j]) == 0) ||*/ (strcmp(linetab[j], linetab[i]) == 0) || est('*', linetab[j]))
			{
				for (int k = j; k < linetablen[0]; k++)
					linetab[k] = linetab[k + 1];
				linetablen[0]--;
			}
			else
				j++;
		}
	}
	printf("Soit, sans doublons : ");
	for (int i = 0; i < linetablen[0]; i++)
		printf("%s ", linetab[i]);
	printf("\n");
	return linetab;
}
//fonction qui transforme une char* en char** avec strtok

int entete(char **h1tab, int h1tablength)
{ //On recherche si le maxtime est plus récent que le maxtime de c
	struct stat buf_h;
	long int maxtime = 0;
	int errstat = 0; //Valeur renvoyée par stat()
	for (int i = 0; i < h1tablength; ++i)
	{
		if ((errstat = stat(h1tab[i], &buf_h)))
		{
			printf("Exception, le fichier d'entête %s n'existe pas\n", h1tab[i]);
			fprintf(stderr,"fichier d'entête inexistant");
			return -1;
		}								   /* Remplacer printf par des messages d'erreur*/
		long int newtime = buf_h.st_mtime; //Temps de modification (nombre de secondes depuis 1 janvier 1970)
		if (newtime > maxtime)
			maxtime = newtime;
	}
	return maxtime;
}

// entêtes liées CH module
int entetch(char **chtab, int chlen, char *c1)
{
	char **tab = malloc(chlen * sizeof(char *));
	int len = 0;
	for (int i = 0; i < chlen; i = i + 2)
	{
		if (strcmp(chtab[i], c1) == 0)
		{
			tab[len] = chtab[i + 1];
			len++;
		}
	}
	int i = entete(tab, len);
	return i;
}

//On compile les .c
int compilc(char *fichier, char **flags, int len)
{
	int fd[2];
	pipe(fd);

	if (fork() == 0)
	{
		char **o = malloc(6 * sizeof(char *) * (len + max(4, strlen(fichier))));
		char *fichiercopy = malloc(strlen(fichier));
		strcpy(fichiercopy, fichier);
		//fichiercopy[strlen(fichier)-2]='\0';
		fichiercopy[strlen(fichier) - 1] = 'o';
		printf("On compile %s pour le nom %s\n", fichier, fichiercopy);
		o[0] = "gcc";
		int j = 1;
		for (j = 0; j < len; j++)
			o[j + 1] = flags[j];
		j = len + 1;
		o[j] = "-c";
		j++;
		o[j] = fichier;
		j++;
		o[j] = "-o";
		j++;
		o[j] = fichiercopy;
		j++;
		o[j]=NULL;
		close(fd[0]);
		dup2(fd[1],1);
		close(fd[1]);
		printf("La chaine à compiler est :");
		for (int i = 0; i < j; i++)
			printf("%s ", o[i]);
		printf("\n");
		execvp("gcc", o);
	}
	else
	{
		wait(NULL);
		close(fd[1]);
		return (fd[0]);
	}
	return 0;
}

//Traite si le fichier doit être recompilé ou non
int source(char **c1tab, int c1tablength, char **h1tab, int h1tablength, int *compt, char **f1tab, int flen, char *flag, char *ch)
{
	int *chlen = malloc(sizeof(int));
	chlen[0] = 0;
	char **chtab = cutstrtok(ch, chlen, 10);
	struct stat buf_c;
	int errstat;
	long int time_fich = 0;
	long int time_fich_o = 0;
	long int time_fich_h = entete(h1tab, h1tablength);
	int ok = 0;
	if (time_fich_h == -1)
	{
		ok = 1;
		//if (!(flag[0] - '-' == 0 && flag[1] - 'k' == 0 && strlen(flag) == 2))
		
		printf("Pas de .h, on s'arrête immédiatement\n");
		fprintf(stderr,"fichier h inexistant");
		return 1;
		
	}
	int p=0;
	for (int i = 0; i < c1tablength; ++i)
	{	p=0;
		//On crée un char* qui vaut current_file avec l'extension .o au lieu de .c pour faire des tests
		char *current_file = c1tab[i];
		int current_file_len = strlen(current_file);
		char *current_file_o = malloc(current_file_len);
		strcpy(current_file_o, current_file);
		current_file_o[current_file_len - 1] = 'o';

		if ((errstat = stat(current_file, &buf_c)))
		{
			printf("Exception, le fichier source %s n'existe pas\n", current_file); /* Remplacer printf par des messages d'erreur*/
			p=1;
			ok = errstat;
			if (!(flag[0] - '-' == 0 && flag[1] - 'k' == 0 && strlen(flag) == 2))
			{
				printf("Pas de continuation sur erreur, on s'arrête immédiatement\n");
				return errstat;
			}
		}
		time_fich = buf_c.st_mtime;
		errstat = stat(current_file_o, &buf_c);
		time_fich_o = buf_c.st_mtime; //Temps de modification (nombre de secondes depuis 1 janvier 1970)

		if (p==0 && errstat)
		{
			printf("Il faut compiler %s, le fichier compilé %s n'existe pas\n", current_file, current_file_o);
			compilc(current_file, f1tab, flen);
			compt[0]++;
		}
		else if ((p==0) && ((time_fich_o < time_fich) || (time_fich_o < max((time_fich_h), entetch(chtab, chlen[0], current_file)))))
		{
			printf("Il faut recompiler le fichier %s qui est trop vieux\n", current_file_o);
			compilc(current_file, f1tab, flen);
			compt[0]++;
		}
	}
	if (ok!=0){
		fprintf(stderr,"Erreur");
	}
	return ok;
}
//On compile les .java
int compilj(char *fichier, char **flags, int len, char **b1tab, int btablen)
{
	int fd[2];
	pipe(fd);

	if (fork() == 0)
	{
		char **o = malloc(6 * sizeof(char *) * (len + btablen + max(6, strlen(fichier))));
		//fichiercopy[strlen(fichier)-2]='\0';
		printf("On compile %s pour le nom en .class\n", fichier);
		o[0] = "javac";
		o[1] = fichier;
		int j = 2;
		
		for (int p = 0; p < btablen; p++)
				o[j + p] = b1tab[p];
		j+=btablen;
		for (int p = 0; p < len; p++)
			o[p + j] = flags[p];
		j += len;
		o[j]=NULL;
		j++;
		printf("La chaine à compiler est :");
		for (int i = 0; i < j; i++)
		printf("%s ", o[i]);
		printf("\n");
		close(fd[0]);
		dup2(fd[1],1);
		close(fd[1]);
		execvp("javac", o);
	}
	else
	{
		wait(NULL);
		close(fd[1]);
		return (fd[0]);
	}
	return 0;
}
int sourcej(char **j1tab, int j1tablength, char **h1tab, int h1tablength, int *compt, char **f1tab, int flen, char *flag,char *b1)
{
	int *btablen = malloc(sizeof(int));
	btablen[0] = 0;
	char **b1tab = cutstrtok(b1, btablen,10);
	struct stat buf_c;
	int errstat;
	long int time_fich = 0;
	long int time_fich_o = 0;
	long int time_fich_h = entete(h1tab, h1tablength);
	int ok = 0;
	if (time_fich_h == -1)
	{
		ok = 1;
		if (!(flag[0] - '-' == 0 && flag[1] - 'k' == 0 && strlen(flag) == 2))
		{
			printf("Pas de continuation sur erreur, on s'arrête immédiatement\n");
			return 1;
		}
	}

	for (int i = 0; i < j1tablength; ++i)
	{
		//On crée un char* qui vaut current_file avec l'extension .o au lieu de .c pour faire des tests
		char *current_file = j1tab[i];
		int current_file_len = strlen(current_file);
		char *current_file_o = malloc(current_file_len+1);
		strcpy(current_file_o, current_file);
		current_file_o[current_file_len] = 's';
		current_file_o[current_file_len - 1] = 's';
		current_file_o[current_file_len - 2] = 'a';
		current_file_o[current_file_len - 3] = 'l';
		current_file_o[current_file_len - 4] = 'c';
		if ((errstat = stat(current_file, &buf_c)))
		{
			printf("Exception, le fichier source %s n'existe pas\n", current_file); /* Remplacer printf par des messages d'erreur*/
			ok = errstat;
			if (!(flag[0] - '-' == 0 && flag[1] - 'k' == 0 && strlen(flag) == 2))
			{
				printf("Pas de continuation sur erreur, on s'arrête immédiatement\n");
				fprintf(stderr,"fichier source inexistant");
				return errstat;
			}
		}
		time_fich = buf_c.st_mtime;
		errstat = stat(current_file_o, &buf_c);
		time_fich_o = buf_c.st_mtime; //Temps de modification (nombre de secondes depuis 1 janvier 1970)

		if (errstat)
		{
			printf("Il faut compiler %s, le fichier compilé %s n'existe pas\n", current_file, current_file_o);
			compilj(current_file, f1tab, flen,b1tab,btablen[0]);
			compt[0]++;
		}
		else if ((time_fich_o < time_fich) || (time_fich_o < time_fich_h))
		{
			printf("Il faut recompiler le fichier %s qui est trop vieux\n", current_file_o);
			compilj(current_file, f1tab, flen,b1tab,btablen[0]);
			compt[0]++;
		}
	}
	if (ok!=0){
		fprintf(stderr,"Erreur");
	}
	return ok;
}


int fari(char *prog, char *flag)
{
	char typ = 'I'; //option permet de reconnaitre les fichiers JAVA
	char *farifile = prog;
	char *f1 = "";
	char *b1 = "";
	char *e1 = "";
	char *c1 = "";
	char *h1 = "";
	char *line = "";
	char *ch = "";
	char *j1 = "";
	size_t line_buf_size = 0;
	int line_count = 0; //Numéro de la dernière ligne comptée
	ssize_t line_size;
	FILE *f = fopen(farifile, "r");
	//Erreur ouverture fichier
	if (!f)
	{
		fprintf(stderr, "Impossible d'ouvrir '%s'\n", farifile);
		return 1;
	}
	line_size = getline(&line, &line_buf_size, f);
	//Lecture du document
	while (line_size >= 0)
	{ //On élimine les lignes blanches
		line_count++;
		printf("La ligne %d contient : %s", line_count, line);
		//On regarde la première lettre
		// Attention ! Mieux vaut utiliser strtok pour être plus flexibles
		if (line[0] == 'C' && line[1] == 'H' && line[2] == ' ')
			ch = lect(line, ch);
		if (line[1] == ' ')
		{
			if (line[0] == 'F')
				f1 = lect(line, f1);
			else
			{
				if (line[0] == 'B')
					b1 = lect(line, b1);
				else
				{
					if (line[0] == 'E')
					{
						if (strlen(e1))
						{
							printf("Exception 2 lignes d'entêtes E\n"); /*Utiliser stderr ici */
							return 1;
						}
						e1 = lect(line, e1);
						char* delim=" ";
						e1 = strtok(e1, delim);
						if (strtok(NULL, delim))
						{
							printf("Plusieurs noms d'exécutables E.\n");
							return 1;
							}
					}
					else
					{
						if (line[0] == 'C')
						{
							if (typ == 'I')
								typ = 'C';
							c1 = lect(line, c1);
						}
						else
						{
							if (line[0] == 'H')
								h1 = lect(line, h1);
							else
							{
								if (line[0] == 'J')
								{ //permet de compiler du java avec javac
									if (typ == 'I')
										typ = 'J';
									j1 = lect(line, j1);
								}
								else
								{
									if (line[0] != '#' && line[0] != ' ' && line[0] != '\t')
									{
										printf("Exception ligne inconnue"); /*Utiliser stderr ici */
										return 2;
									}
								}
							}
						}
					}
				}
			}
		}
		line_size = getline(&line, &line_buf_size, f);
	}
	printf("J'ai reconnu : \nC=%s, H=%s, F=%s, E= %s, CH=%s, J=%s \n", c1, h1, f1, e1, ch,j1);
	free(line);
	line = NULL;
	int *ftablen = malloc(sizeof(int));
	ftablen[0] = 0;
	char **f1tab = cutstrtok(f1, ftablen,10);
	int *btablen = malloc(sizeof(int));
	btablen[0] = 0;
	char **b1tab = cutstrtok(b1, btablen,10);
	if (typ == 'C')
	{
		//Décortiquons avec strtok la valeur de h1 dans le tableau h1tab
		int *h1tablength = malloc(sizeof(int));
		h1tablength[0] = 0;
		char **h1tab = cutstrtok(h1, h1tablength,10); //h1tab est la chaine h1 décomposée en char**

		int *c1tablength = malloc(sizeof(int));
		c1tablength[0] = 0;
		char **c1tab = cutstrtok(c1, c1tablength,10); //c1tab est la chaine h1 décomposée en char**
		int *otablen = malloc(sizeof(int));
		otablen[0] = 0;
		char **o1tab = cutstrtok(c1, otablen,10);
		printf("Le tableau des o est :");
		for (int i = 0; i < c1tablength[0]; ++i)
		{
			o1tab[i][strlen(o1tab[i]) - 1] = 'o';
			printf("%s ", o1tab[i]);
		}
		printf("\n");
		int *compt = malloc(sizeof(int));
		compt[0] = 0;

		int rslt_source = source(c1tab, c1tablength[0], h1tab, h1tablength[0], compt, f1tab, ftablen[0], flag, ch);
		if (rslt_source)
		{
			printf("rslt_source positif(%d) : Arrêt immédiat \n", rslt_source);
			fprintf(stderr,"Erreur");
			return 1;
		}
		//Il faudra verifier si rien n'a été recompilé on arretera avant
		struct stat buf_exe;
		int errstat = stat(e1, &buf_exe);

		int max = 0;
		int time_fich_o = 0;
		for (int j = 0; j < otablen[0]; j++)
		{
			struct stat buf_o;
			stat(o1tab[j], &buf_o);
			time_fich_o = buf_o.st_mtime;
			if (max < time_fich_o)
				max = time_fich_o;
		}

		if ((compt[0] == 0) && !(errstat))
		{
			printf("Le fichier exe existe vérifions s'il est récent\n");
			int time_fich_exe = buf_exe.st_mtime;
			if (max < time_fich_exe)
			{
				printf("Votre exécutable :%s était déjà à jour. Vous pouvez l'utiliser dès à présent.\n", e1);
				return 0;
			}
		}
		printf("Nous mettons l'executable à jour. \n");
		
		int i = strlen(e1) + strlen(c1) + strlen(b1) + strlen(f1) + 18; //18 pour être large
		char **o = malloc(i * sizeof(char *));
		o[0] = "gcc";
		for (int j = 0; j < ftablen[0]; j++)
			o[j + 1] = f1tab[j];
		int p = ftablen[0] + 1;
		for (int j = 0; j < otablen[0]; j++)
			o[j + p] = o1tab[j];
		p += otablen[0];
		o[p] = "-o";
		p++;
		o[p] = e1;
		p++;
		for (int j = 0; j < btablen[0]; j++)
			o[j + p] = b1tab[j];
		p+=btablen[0];
		o[p]=NULL;
		p++;
		printf("La chaine à compiler est :");
		for (int j = 0; j < p; j++)
			printf("%s ", o[j]);
		printf("\n");
		int fd[2];
		pipe(fd);
		if (!fork() == 0) {
			close(fd[0]);
			dup2(fd[1],1);
			close(fd[1]);
			execvp("gcc", o);
		}
		else
		{
			wait(NULL);
			fclose(f);
			close(fd[1]);
			return (fd[0]);
		}
	}
	else {
		if (typ =='J'){

		int *h1tablength = malloc(sizeof(int));
		h1tablength[0] = 0;
		char **h1tab = cutstrtok(h1, h1tablength,10); //h1tab est la chaine h1 décomposée en char**

		int *j1tablength = malloc(sizeof(int));
		j1tablength[0] = 0;
		char **j1tab = cutstrtok(j1, j1tablength,10); //c1tab est la chaine h1 décomposée en char**
		int* compt=malloc(sizeof(int));
		compt[0] = 0;
		int rslt_source = sourcej(j1tab, j1tablength[0], h1tab, h1tablength[0], compt, f1tab, ftablen[0], flag,b1);
		if (rslt_source)
		{
			printf("rslt_source positif(%d) : Arrêt immédiat \n", rslt_source);
			fprintf(stderr,"Erreur");
			return 1;
		}
		}
	}
	return 0;
}

int main(int argc, char *args[])
{
	if (argc < 2)
		fari("farifile", "");
	if (argc == 2)
	{if (strlen(args[1])>1 && args[1][0]=='-' && args[1][1]=='k'){
		fari("farifile", args[1]);
	}
	else{
		fari(args[1], "");
	}
	}
	if (argc > 2)
		fari(args[1], args[2]);
	return 0;
}
