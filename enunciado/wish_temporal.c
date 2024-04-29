// Librerías utilizadas:
#include <stdio.h> // Para entrada y salida estándar
#include <unistd.h> // Para la función fork(), exec,(), exit(), entre otras
#include <string.h> // Para manipulación de cadenas de caracteres
#include <stdlib.h> // Para funciones de utilidad genreal, como 'malloc()' y 'free()'
#include <sys/wait.h> // Para la función 'wait()', que espera a que un proceso hijo termine
#include <fcntl.h> // Para manipulación de descriptores de archivo

// Método principal
int main(int argc, char* argv[])
{	
    // Bucle principal, se queda en iteración infinita hasta que el usuario digite la palabra 'exit'
	while(1)
	{	
        // Definición de variables
		char A[50]; // Un array de caracteres para almacenar la entrada del usuario
		char* myargv[10]; // Un array de punteros a caracteres para almacenar los argumentos del comando
        char* tok; // Un puntero a caracteres para usar en la tokenización de la entrada del usuario
        int i = 0; // Un contador para iterar sobre los argumentos del comando
		char* str; // Una cadena de caracteres auxiliar para manipulación
		int redirection = 0; // Un indicador para señalar si hay redirección en el comando
		int parallel = 0; // Un indicador para señalar si el comando se debe ejecutar en segundo plano

		char error_message[30] = "An error has occurred\n";

		// Ciclo 'for' para inicializar el array ''myargv' con valores nulos para asegurarse de que esté limpio antes de cada nueva entrada de comando 
        for(i = 0; i < 10; i++)                
		{
            myargv[i] = NULL;                
		}                 
		printf("wish> "); // Promt para indicarle al usuario que ingrese un comando
		fgets(A, 50, stdin); // Lectura de la entrada del usuario utilizando 'fgets()' y se almacena en el array 'A'

		if(strcmp(A, "exit\n")!=0) // Verificar si la entrada del usuario es igual a "exit\n". Si es así, el programa sale del bucle y termina
		{   
            // Tokenización de la entrada del usuario
			str = strdup(A); // Se copia la entrada del usuario en la cadena auxiliar str
			i = 0;
			while ((tok = strsep(&str, " ")) != NULL) // Se utiliza 'strstep()' para dividir la cadena en tokens utilizando el espacio como delimitador
			 {     
				myargv[i] = strdup(tok); // Se copia cada token en myargv
				myargv[i][strcspn(myargv[i], "\n")] = 0; // Se elimina el caracter de nueva línea '\n' al final de cada token
				i++;
            }
			int rc = fork(); // Se usa fork() para crear un nuevo proceso hijo
			if(rc == 0) // El proceso hijo (rc == 0) ejecuta el código dentro del primer bloque 'if'
			{
				for(i = 0; myargv[i] != NULL; i++) // Iterar sobre cada argumento de myargv para determinar si hay redirección '>' o si el comando debe ejecutarse en segundo plano '&'
				{
					if(strcmp(myargv[i], ">")==0)
					{
						redirection = 1;
					}
					if(strcmp(myargv[i], "&")==0)
                    {   
                        parallel = 1;                                         
					}
				}

				if(strcmp(myargv[0], "ls")==0) // Si el comando es "ls", se reemplaza por "/bin/ls" y se ejecuta utilizando execv()
				{
					printf("In ls");
					myargv[0] = strdup("/bin/ls"); //**************** BUSCAR strdup
                    execv(myargv[0], myargv); ////**************** BUSCAR execv
				}
				else if(strcmp(myargv[0], "cd")==0) // Si el comando es "cd", se cambia el directorio de trabajo utilizando chdir()
				{
					if(myargv[1] == NULL) {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(1); // Código de retorno 0 para indicar error
                    }
					else {
					printf("%s\n", getcwd(str, 100)); //**************    BUSCAR getcwd
					chdir(myargv[1]);  
					printf("%s\n", getcwd(str, 100));
					}
				}
				else if ( (strcmp(myargv[0], "cat")==0) && (redirection == 1) ) // OJO (cambiar eso del p4.out) Si el comando es "cat" y hay redirección, se redirige la salida estándar a un archivo "p4.out".
				{
					printf("redirection 1");
					close(STDOUT_FILENO);
					open("./p4.out", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU); 
					// O_CREAT: Crea el archivo si no existe
                    // O_WRONLY: Abre el archivo solo para escritura
                    // O_TRUNC: Trunca el archivo a longitud cero, si existe, o lo crea vacío si no existe
                    // S_IRWXU: Otorga al propietario del archivo permisos de lectura, escritura y ejecución.
					myargv[2] = NULL; 
					execvp(myargv[0], myargv); //**************** BUSCAR execvp
				}
				else if ((strcmp(myargv[0], "hello")==0) && ( redirection == 1)) // OJO (hello)Si el comando es "hello" y hay redirección, se ejecuta un programa "hello" que redirige su salida estándar a "p4.out".
                {
                    close(STDOUT_FILENO);   
                    open("./p4.out", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU); 
					myargv[0] = strdup("./hello");
					myargv[1] = NULL;
                    execvp(myargv[0], myargv);  

				}
				 if(parallel == 1) // Si se detecta que el comando debe ejecutarse en segundo plano (parallel == 1), se crea un nuevo proceso hijo (rc2) para ejecutar el comando
				{

					int rc2 = fork();
					printf("In parallel");
					//myargv[0] = strdup("./hello");
					myargv[1] = NULL;


					if(rc2 == 0) // Se muestra un mensaje indicando si es el proceso hijo o el padre
					{
						printf("child");
						execv(myargv[0], myargv);
					}
					else
					{

						rc2 = (int) wait(NULL);
						printf("parent");
						execv(myargv[0], myargv);	
					}
				}
				else  
				{
                    execv(myargv[0], myargv);                                 
				}

				exit(0); // Proceso hijo sale 'exit()' después de ejecutar el comando 
			}			
			else // 
			{
				rc = (int) wait(NULL); // El proceso padre espera a que termine el proceso hijo utilizando 'wait()'.
			}

		}
		else if(strcmp(A, "exit\n")==0) // Si el usuario escribe 'exit', termina el bucle principal
		{
			break; 
			exit(0);
		}
	}

exit(0);
}