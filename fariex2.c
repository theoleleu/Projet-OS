#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h> //getpid, fork
#include <string.h> //strcmp, strcat
#include <stdio.h> //printf
#include <stdlib.h> //exit, atoi

#define max(a,b) (a>=b?a:b)
/*nom des fichiers séparés d'un espace*/
char* lect(char* line,char* tab){
	int ttab=strlen(tab)+1;
	int tline=strlen(line)+1;	
	if (tline<5) {//"E f\n" est le char* le plus court possible (strlen ->5)
		printf("Erreur, ligne non reconnue !\n");
		return tab;
	} 		
	char* newtab=malloc((ttab+tline)*sizeof(char));		
	newtab=strcat(newtab,tab);
	newtab=strcat(newtab," ");
	return strncat(newtab,line+2,strlen(line+4));	//On enleve les 2 premiers char et les 2 derniers (\n)
}

char** cutstrtok(char* line, int* linetablen){
	char* linecopy=malloc(strlen(line)); 
	strcpy(linecopy,line); //linecopy permet de ne pas écraser la valeur de line avec strtok

	char** linetab=malloc(strlen(line)*sizeof(char)); 	//linetab est la chaine line décomposée en char**
	char* delim=" ";
	char* current_str_in_line=strtok(linecopy,delim);
	int index_linetab=0;
	printf("%s la chaîne est transformée en le tableau : ",line);
	while (current_str_in_line){
		linetablen[0]++;
		linetab[index_linetab]=current_str_in_line;
		printf("%s ",current_str_in_line);
		index_linetab++;
		current_str_in_line=strtok(NULL,delim);
	}
	printf("\n");

	return linetab;
}
//fonction qui transforme une char* en char** avec strtok


int entete(char** h1tab, int h1tablength){ //On recherche si le maxtime est plus récent que le maxtime de c
	struct stat buf_h;
	long int maxtime;
	int errstat; //Valeur renvoyée par stat()
    for (int i=0; i<h1tablength;++i){
		if((errstat= stat(h1tab[i],&buf_h))) {printf("Exception, le fichier d'entête %s n'existe pas\n",h1tab[i]);
		return 1;}/* Remplacer printf par des messages d'erreur*/
		long int newtime= buf_h.st_mtime;//Temps de modification (nombre de secondes depuis 1 janvier 1970)
		if (newtime>maxtime) maxtime=newtime;
	}
	return maxtime;
}

int compilc(char* fichier,char** flags,int len){
	if (fork()){
	char **o=malloc(6*sizeof(char*)*(len+max(3,strlen(fichier))));
	char* fichiercopy=malloc(strlen(fichier));
	strcpy(fichiercopy,fichier); 
	//fichiercopy[strlen(fichier)-2]='\0';
	fichiercopy[strlen(fichier)-1]='o';
	printf("On compile %s pour le nom %s\n",fichier,fichiercopy);
	o[0]="gcc";
	int j=1;
	 for (j=0;j<len;j++){
		o[j+1]=flags[j];
	}
	j=len+1;
	o[j]="-c";
	j++;
	o[j]=fichier;
	j++;
	o[j]="-o";
	j++;
	o[j]=fichiercopy;
	printf("La chaine à compiler est :");
	for (int i=0;i<j;i++){
		printf("%s ",o[i]);
	}
	printf("\n");

	execvp("gcc",o);
}
	return 0;
}

//Traite si le fichier doit être recompilé ou non
int source(char** c1tab, int c1tablength, char** h1tab, int h1tablength,int* compt,char* f1){
	int* ftablen=malloc(sizeof(int));
	ftablen[0]=0;
	char** f1tab=cutstrtok(f1, ftablen);
	struct stat buf_c;
	int errstat;
	long int time_fich=0;
	long int time_fich_o=0;
    long int time_fich_h=entete(h1tab,h1tablength);
    for (int i=0; i<c1tablength;++i){
		//On crée un char* qui vaut current_file avec l'extension .o au lieu de .c pour faire des tests
        char* current_file=c1tab[i];
		int current_file_len=strlen(current_file);
        char* current_file_o=malloc(current_file_len);
		strncpy(current_file_o,current_file,current_file_len-1); 
		current_file_o[current_file_len-1]='o';
        if((errstat= stat(current_file,&buf_c))){
            printf("Exception, le fichier source %s n'existe pas\n", current_file);/* Remplacer printf par des messages d'erreur*/
            return errstat;
        }
        time_fich=buf_c.st_mtime;
		errstat= stat(current_file_o,&buf_c);
		time_fich_o= buf_c.st_mtime;//Temps de modification (nombre de secondes depuis 1 janvier 1970)
        if(errstat){
            printf("Il faut compiler %s, le fichier compilé %s n'existe pas\n", current_file, current_file_o);
			compilc(current_file,f1tab,ftablen[0]);
			compt[0]++;
        }
        else if((time_fich_o<time_fich)||(time_fich_o<time_fich_h)){
            printf("Il faut recompiler le fichier %s qui est trop vieux\n", current_file_o);
			compilc(current_file,f1tab,ftablen[0]);
			compt[0]++;
        }	
	}
	return 0;
}


int fari(char* prog) {
	char typ='I'; //option permet de reconnaitre les fichiers JAVA
    char* farifile=prog;
	char* f1=""; char* b1=""; char* e1="";
	char* c1=""; char* h1=""; char * line="";
	char* j1="";
    size_t line_buf_size = 0;
    int line_count = 0;  //Numéro de la dernière ligne comptée
    ssize_t line_size;
    FILE *f = fopen(farifile, "r");
   /*Erreur ouverture fichier*/
    if (!f){
        fprintf(stderr, "Impossible d'ouvrir '%s'\n", farifile);
        return 1;
    }
    line_size = getline(&line, &line_buf_size, f);

	/*Lecture du document*/
    while (line_size >= 0){ //On élimine les lignes blanches
        line_count++;
        printf("La ligne %d contient : %s", line_count,line);
		/*On regarde la première lettre*/
		/* Attention ! Mieux vaut utiliser strtok pour être plus flexibles */
		if (line[1]==' '){
			if (line[0]=='F') f1=lect(line,f1);
			else {
				if (line[0]=='B'){
					b1=lect(line,b1);
				}
				else{	
					if (line[0]=='E'){
						if (strcmp(e1,"")) {
								printf("Exception 2 entêtes E");	/*Utiliser stderr ici */
								return 1;
						}
						e1=lect(line,e1);		
					}
					else{
						if (line[0]=='C'){
							if (typ=='I') {
								typ='C';
								c1=lect(line,c1);
							}
							else {if (typ=='C'){
								c1=lect(line,c1);
							}}
							
						}
						else{
							if (line[0]=='H'){
								h1=lect(line,h1);
							}
							else{
								if (line[0]=='J'){ //permet de compiler du java avec javac
										if (typ=='I') {
											typ='J';
											j1=lect(line,j1);
							}
							else {if (typ=='J'){
								j1=lect(line,j1);
							}}
								}
								else { if (line[0]!='#') {
										printf("Exception ligne inconnue");	/*Utiliser stderr ici */
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
	printf("J'ai reconnu : \nC=%s, H=%s, F=%s, E= %s \n",c1,h1,f1,e1);
    free(line);
    line = NULL;

	
   	int* h1tablength=malloc(sizeof(int));
	int* c1tablength=malloc(sizeof(int));
	h1tablength[0]=0;
	c1tablength[0]=0;
	//Décortiquons avec strtok la valeur de h1 dans le tableau h1tab
	char** h1tab=cutstrtok(h1, h1tablength); //h1tab est la chaine h1 décomposée en char**
	char** c1tab=cutstrtok(c1, c1tablength);
	int* otablen=malloc(sizeof(int));
	otablen[0]=0;
	char** o1tab=cutstrtok(c1, otablen); 
	
	printf("Le tableau des o est :");
	for (int i=0; i<c1tablength[0];++i){
		//o1tab[i][strlen(o1tab[i])-2]='\0';
		o1tab[i][strlen(o1tab[i])-1]='o';
		printf("%s ",o1tab[i]);
	}
	printf("\n");
	int* compt=malloc(sizeof(int));
	compt[0]=0;
	int rslt_source=source(c1tab, c1tablength[0], h1tab, h1tablength[0],compt,f1);
	printf("rslt_source vaut :%d\n", rslt_source);
//Il faudra verifier si rien n'a été recompilé on arretera avant
/*if (compt==0){
	return 0;
}*/

	int i = strlen(e1)+strlen(c1)+strlen(b1)+strlen(f1)+18; //18 pour être large
	char** o=malloc(i*sizeof(char*));
	int j=0;
	o[0]="gcc";
	int* ftablen=malloc(sizeof(int));
	ftablen[0]=0;
	char** f1tab=cutstrtok(f1, ftablen);
	 for (j=0;j<ftablen[0];j++){
		o[j+1]=f1tab[j];
	}
	j=ftablen[0]+1;
	int p=j;
	for(j=0;j<otablen[0];j++){
		o[j+p]=o1tab[j];
	}
	j=otablen[0]+p;
	o[j]="-o";
	j++;
	o[j]=e1;
	j++;
	int* btablen=malloc(sizeof(int));
	btablen[0]=0;
	char** b1tab=cutstrtok(b1, btablen);
	p=j;
	 for (j=0;j<btablen[0];j++){
		o[j+p]=b1tab[j];
	}
	printf("La chaine à compiler est :");
	for (int i=0;i<ftablen[0]+p;i++){
		printf("%s ",o[i]);
	}
	printf("\n");
	//Voila je crois que c'est bon reste à placer les B à l'édition de lien, ln : http://manpages.ubuntu.com/manpages/trusty/fr/man1/ln.1.html
	if (!fork()) { 
		execvp("gcc",o);
	}
    else fclose(f);
    return 0;
}

int main(){
	fari("Farifile.txt");
	return 0;
}

