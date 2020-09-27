#include <json-c/json.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> //getpid, fork
#include <string.h> //strcmp, strcat
#include <stdio.h>  //printf
#include <stdlib.h> //exit, atoi


int main(){
	// Initialisation
	char** c1tab = malloc(30);
	c1tab[0]= "f.c";
	c1tab[1]= "f1.c";
	c1tab[2]= "f2.c";
	int c1tablength= 3;
	char** f1tab = malloc(30);
	f1tab[0]= "-g";
	int f1tablength= 1;
	char** b1tab = malloc(30);
	b1tab[0]= "-lm";
	int b1tablength= 1;
	char** h1tab = malloc(30);
	h1tab[0]= "f.h";
	int h1tablength= 1;
	char* e1 = "f";
	char* fari_error_msg = "je suis fari_error_msg"; //Ã  changer
	char** commands = malloc(50);
	commands[0]= "gcc -g -c f.c";
	commands[1]= "gcc -g -c f1.c";
	commands[2]= "gcc -g -c f2.c";
	int commandslength = 3; 
	char* error_msg_1="je suis error_msg_1"; //Ã  changer
	char* command = "gcc -g -o E f.o f1.o f2.o -lm";
	char* error_msg_2="je suis error_msg_2"; //Ã  changer
	char* fari_msg= "Compilation terminÃ©e"; 



    FILE *farifile = fopen("Farifile.txt", "r");    
    FILE *jsonfile = fopen("logs.json", "w");
	if(jsonfile==NULL){
		printf("ProblÃ¨me lors de l'ouverture du fichier\n");
		return(1);
	}
	json_object *jobj_1 = json_object_new_object();


    json_object *jarray_1 = json_object_new_array();
	for (int i=0; i<c1tablength;++i){
		json_object_array_add(jarray_1, json_object_new_string(c1tab[i]));}
	json_object_object_add(jobj_1,"sources", jarray_1);

    json_object *jarray_2 = json_object_new_array();
	for (int i=0; i<h1tablength;++i){
		json_object_array_add(jarray_2, json_object_new_string(h1tab[i]));}
	json_object_object_add(jobj_1,"headers", jarray_2);

    json_object *jarray_3 = json_object_new_array();
	for (int i=0; i<b1tablength;++i){
		json_object_array_add(jarray_3, json_object_new_string(b1tab[i]));}
	json_object_object_add(jobj_1,"libraries", jarray_3);

    json_object *jarray_4 = json_object_new_array();
	for (int i=0; i<f1tablength;++i){
		json_object_array_add(jarray_4, json_object_new_string(f1tab[i]));}
	json_object_object_add(jobj_1,"flags", jarray_4);

	json_object *jstring_1 = json_object_new_string(e1);
	json_object_object_add(jobj_1,"executable_name", jstring_1);

	json_object *jstring_2 = json_object_new_string(fari_error_msg);
	json_object_object_add(jobj_1,"fari_error_msg", jstring_2);

	json_object *jobj_2 = json_object_new_object();
	json_object *jarray_5 = json_object_new_array();
	for (int i=0; i<commandslength;++i){
		json_object_array_add(jarray_5, json_object_new_string(commands[i]));}
	json_object_object_add(jobj_2, "commands", jarray_5);
	json_object *jstring_3 = json_object_new_string(error_msg_1);
	json_object_object_add(jobj_2,"error_msg", jstring_3);
	json_object_object_add(jobj_1,"compilation", jobj_2);

	json_object *jobj_3 = json_object_new_object();
	json_object *jstring_4 = json_object_new_string(command);
	json_object_object_add(jobj_3, "command", jstring_4);
	json_object *jstring_5 = json_object_new_string(error_msg_2);
	json_object_object_add(jobj_3,"error_msg", jstring_5);
	json_object_object_add(jobj_1,"linking", jobj_3);

	json_object *jstring_6 = json_object_new_string(fari_msg);
	json_object_object_add(jobj_1,"fari_error_msg", jstring_6);




	const char* strfinal = json_object_to_json_string(jobj_1);
	printf("Le strfinal vaut : %s\n",strfinal);
	fprintf(jsonfile, strfinal);

	fclose(jsonfile);
    return 0;
}






// Comment parser un fichier json :
/*
int main(int argc, char **argv) {
	FILE *fp;
	char buffer[1024];
	struct json_object *parsed_json;
	struct json_object *name;
	struct json_object *age;
	struct json_object *friends;
	struct json_object *friend;
	size_t n_friends;

	size_t i;	

	fp = fopen("test.json","r");
	fread(buffer, 1024, 1, fp);
	fclose(fp);

	parsed_json = json_tokener_parse(buffer);

	json_object_object_get_ex(parsed_json, "name", &name);
	json_object_object_get_ex(parsed_json, "age", &age);
	json_object_object_get_ex(parsed_json, "friends", &friends);

	printf("Name: %s\n", json_object_get_string(name));
	printf("Age: %d\n", json_object_get_int(age));

	n_friends = json_object_array_length(friends);
	printf("Found %lu friends\n",n_friends);

	for(i=0;i<n_friends;i++) {
		friend = json_object_array_get_idx(friends, i);
		printf("%lu. %s\n",i+1,json_object_get_string(friend));
	}	
}*/