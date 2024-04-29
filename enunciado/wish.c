#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h> // Para usar las funciones stat() y S_IXUSR en ejecutarComando()

#define FINAL_EXITOSO 0
#define FINAL_ERRONEO 1
#define MAX_PATH 4096

char *paths[100]={"/bin/","/usr/bin/", NULL};
int cantPaths = 3;
char *tokens[100];
int indice = 0 ; // Para el número de tokens
int hijos[100];
int cantHijos = 0 ;

void obtenerTokens(FILE *); // Acepta un archivo como un input y lo convierte en comandos separados alojados en tokens 
void ejecutarComando(); // Ejecuta el sigueinte comando en tokens 
int comandoTieneRedireccion(char *arr[] , int start); // Verifica si el comando tiene redirección '>'
void error(); // Imprime el mensaje de error en stderr 

int main (int argc, char *argv[]) {
	if(argc == 1) { // Modo interactivo
		obtenerTokens(stdin); // Obtiene los tokens del comando que el usuario digita en stdin
	}
	else if (argc == 2) { // Modo batch
		FILE *input = fopen(argv[1],"r"); // Abre el archivo que se ejecutó por terminal 
		if(input != NULL) {// Verifica si el archivo no puede ser abierto
			obtenerTokens(input);
		}
		else {
			error();
			exit(FINAL_ERRONEO);
		}
	}
	else { // Si es error
    error();	
    exit(FINAL_ERRONEO);
	}
	return 0;
} // Final del main()

void obtenerTokens(FILE* input) {
	while(1) { // Para esperar por un input en el terminal 
		if(input == stdin) // Modo interactivo
			printf("wish> "); // Imprime el promt del terminal
		int temp = indice;
		indice =0 ;
		char *original;
		size_t len = 0 ;
		if(getline(&original,&len,input)==EOF) {  
			exit(FINAL_EXITOSO);
		}
		unsigned long long original_size = strlen(original); 
		char *modified = (char *)malloc(sizeof(char)*original_size*6);
		int shift  =0;
		if(!(strcmp(original,"&\n"))) { // Se salta el '\n'
			continue;
		}
		for(int i = 0 ; i<original_size ; i++ ) {
			if(original[i] == '>'|| original[i] == '&' || original[i] == '|') { //check for special signs //** TODO : pipe **//
				modified[i+shift]=' ';
				shift++;
				modified[i+shift]=original[i];
				i++;
				modified[i+shift]=' ';
				shift++;
			}
			modified[i+shift]=original[i];
		}
		char *token;
		while( (token = strsep(&modified," \n")) != NULL ) { //remove white spaces and newlines
			if(!strcmp(token,"&")) { // check for multiple commands
				tokens[indice] = NULL ;
				indice++;
			}
			if(memcmp(token,"\0",1)) { //not equal  (not a null input)
				tokens[indice] = token;
				indice++;
			}
		}
		for(int i = indice ; i < temp ; i++) { // removing executed commands 
			tokens[i]=NULL;
		}
		ejecutarComando(); // execute the token commands 
		free(modified);
		for (int x = 0 ; x<cantHijos ; x++ ) { // wait for the child processes
			int stat;
			waitpid(hijos[x],&stat,0);
		}
	}
}

/*
void ejecutarComando() {
    for (int i = 0; i < indice; i++) {
        // Check if the token is a file path
        struct stat file_stat;
        if (stat(tokens[i], &file_stat) == 0) {
            if (file_stat.st_mode & S_IXUSR) {
                // File is executable, execute it
                int pid = fork();
                if (pid == -1) {
                    // Error handling
                    error();
                    exit(FINAL_ERRONEO);
                } else if (pid == 0) {
                    // Child process
                    execv(tokens[i], tokens + i);
                    // If execv returns, it means execution failed
                    error();
                    exit(FINAL_ERRONEO);
                } else {
                    // Parent process
                    // Wait for child to finish
                    int status;
                    waitpid(pid, &status, 0);
                    // Move to next token
                    while (tokens[i] != NULL) {
                        i++;
                    }
                    i++;
                }
            } else {
                // File is not executable, continue with standard command execution
                // (Your existing logic here)
				for(int i =0 ; i<indice ;i++) { // itrate the tokens 
					if (!strcmp(tokens[i], "exit")) { // special command 
						if(tokens[i+1] == NULL) { // for a test case :"
							exit(FINAL_EXITOSO);
						}
						else {
							i++;
							error();
						}
					}
					else if(!strcmp(tokens[i],"cd")) { // special command
						if(chdir(tokens[++i])) { // changing the directory 
							error();
						}
						while (tokens[i] != NULL) { // moving i to the next token command
							i++;
						}
						i++;

					}
					else if(!strcmp(tokens[i],"path")) { // special command
						cantPaths=0;
						for(int j=1 ;j<indice;j++) {
							char *temp = realpath(tokens[j],NULL); // check for the path 
							if (temp != NULL) {
								paths[j-1] = temp;
							}
							else {
								error();
								cantPaths++;
							}
						}
						i+=cantPaths;
					}
					else {
						int pid = fork();
						hijos[cantHijos++]=pid ;
						if(pid==-1){
							error();
							exit(FINAL_ERRONEO);
						}
						if(pid==0){ 
							//child 
							if(comandoTieneRedireccion(tokens, i) != -1) { // check for '>'
								for(int j =0 ;j<cantPaths;j++) {
									char *exe = malloc(sizeof(char)*MAX_PATH); 
									strcpy(exe,paths[j]);
									strcat(exe,"/");
									strcat(exe,tokens[i]); // get the full path of the executable 
									if(!access(exe,X_OK)) {
										execv(exe,tokens+i); // execute the program with its arguments utill find NULL 
									}
								}
								error();
								exit(FINAL_ERRONEO);
							}
						}
						else {
							//parent
							while (tokens[i] != NULL) { // moving i to the next token command
								i++;
							}
							i++;
							if (i < indice) {
								continue;
							}
						}
					}
				}
            }
        } else {
            // Handle errors if stat fails
            error();
        }
    }
}
*/


void ejecutarComando() {
	for(int i =0 ; i<indice ;i++) { // itrate the tokens 
		if (!strcmp(tokens[i], "exit")) { // special command 
			if(tokens[i+1] == NULL) { // for a test case :"
				exit(FINAL_EXITOSO);
			}
			else {
				i++;
				error();
			}
		}
		else if(!strcmp(tokens[i],"cd")) { // special command
			if(chdir(tokens[++i])) { // changing the directory 
				error();
			}
			while (tokens[i] != NULL) { // moving i to the next token command
				i++;
			}
			i++;

		}
		else if(!strcmp(tokens[i],"path")) { // special command
			cantPaths=0;
			for(int j=1 ;j<indice;j++) {
				char *temp = realpath(tokens[j],NULL); // check for the path 
				if (temp != NULL) {
					paths[j-1] = temp;
				}
				else { 
					error();
				}
				cantPaths++;
			}
			i+=cantPaths;
		}
		else {
			int pid = fork();
			hijos[cantHijos++]=pid ;
			if(pid==-1) { 
				error();
				exit(FINAL_ERRONEO);
			}
			if(pid==0){
				//child 
				if(comandoTieneRedireccion(tokens, i) != -1) { // check for '>'
					for(int j =0 ;j<cantPaths;j++) {
						char *exe = malloc(sizeof(char)*MAX_PATH); 
						strcpy(exe,paths[j]);
						strcat(exe,"/");
						strcat(exe,tokens[i]); // get the full path of the executable 
						if(!access(exe,X_OK)) {
							execv(exe,tokens+i); // execute the program with its arguments utill find NULL 
						}
					}
					error();
					exit(FINAL_ERRONEO);
				}
			}
			else {
				//parent
				while (tokens[i] != NULL) { // moving i to the next token command
					i++;
				}
				i++;
				if (i < indice) {
					continue;
				}
			}
		}
	}
}


int comandoTieneRedireccion(char *arr[] , int start) {
	for(int i = start ; arr[i] != NULL ; i++) {
		if(!strcmp(arr[i],">") && i != start) { // no command before '>'
			arr[i]=NULL;
			if(arr[i+1] == NULL || arr[i+2] != NULL) { // no file after '>'
				error();
				return -1 ;
			}
			else {
				int fd = open(arr[i+1],O_WRONLY | O_CREAT,0777); //open or creat with high permissions 
				dup2(fd,STDOUT_FILENO); // changing the output file (redirect the output)
				arr[i+1]=NULL;
				close(fd);
			}
			return i;
		}
	}
	return 0;
}

void error() {
	char mensaje_error[30] = "An error has occurred\n";
	write(STDERR_FILENO, mensaje_error, strlen(mensaje_error));
}