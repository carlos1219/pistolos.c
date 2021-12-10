/*pistolos.c 
Segunda practica SS.OO I 
Miguel Gozález Herranz
Carlos León Arjona
Para compilar usar gcc pistolos.c -o pistolos en linux.
El programa crea x  procesos hijos y cada hijo mata otro proceso al azar ,el padre y el hijo hablan mediante un fichero
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#define RESET_COLOR    "\x1b[0m" 
#define AMARILLO_T "\x1b[33m" 
#define CYAN_T     "\x1b[36m" 
#define VERDE_T        "\x1b[32m"  
#define MAGENTA_T  "\x1b[35m"
#define ROJO_T     "\x1b[31m"
#define AZUL_T     "\x1b[34m"    

void signal_alarm(int signal){
	//printf("Esperando a que se escriba el fichero...\n");
}

int main (int argc, char *argv[])
{
//******VARIABLES******//
	int i;
	int j;
	int k = 1;
	int n_pistolos;
	int n_pistolos_o;
	long n_pistolo;
	int status;
	int mata_a;
	pid_t pidC;
	pid_t pidC2[128];
	pid_t pidM[128];
	int matar;
	int fd;
	char mensaje[40];
	int tam;
	int limites[128];
	char linealeida1[1024];
	char linealeida2[1024];
	int limit;
//******MÁSCARAS******//
	sigset_t mask;
	sigset_t mask2;
	sigset_t mask_alarm;
	sigset_t mask_alarmc;
	sigfillset(&mask);
	sigdelset(&mask,SIGTERM);
	sigfillset(&mask2);
	sigdelset(&mask2,SIGTSTP);
	sigfillset(&mask_alarm);
	sigdelset(&mask_alarm,SIGALRM);
	sigdelset(&mask_alarm,SIGINT);
	sigfillset(&mask_alarmc);
	sigdelset(&mask_alarmc,SIGALRM);
	sigdelset(&mask_alarmc,SIGINT);
	sigprocmask(SIG_BLOCK,&mask2,NULL);
	
	if(argc == 1){
		perror("Error, se debe identificar el numero de procesos a crear");
		exit(-1);
	}
	
	
	fd = open("pistolos.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);
	
	if(fd == -1){
		perror("Error en la creacion del fichero");
		exit(-1);
	}
	
	close(fd);
	lockf(fd,F_LOCK,0);
	
	n_pistolo = strtol(argv[1],NULL,10);
	
	if(n_pistolo > 128 || n_pistolo < 3){
		perror("Numero de pistoleros incorrecto");
	}else{
		n_pistolos = n_pistolo;
		printf(AMARILLO_T" 	 ____________________________________ \n"RESET_COLOR );
		printf(AMARILLO_T"	|                                    |\n"RESET_COLOR );
		printf(AMARILLO_T"	|                                    |\n"RESET_COLOR );
		printf(AMARILLO_T"	|       "CYAN_T"PISTOLEROS DE KENTUCKY"AMARILLO_T"       |\n"RESET_COLOR );	      
		printf(AMARILLO_T"	|                                    |\n"RESET_COLOR );
		printf(AMARILLO_T"	|____________________________________|\n"RESET_COLOR );
		printf(VERDE_T"Se han creado %d pistoleros\n"RESET_COLOR, n_pistolos);
		printf(MAGENTA_T "Esperando a que se escriban los procesos en el fichero...\n"RESET_COLOR);
		++n_pistolos;
		n_pistolos_o = n_pistolos;
		
		pid_t pidP = getpid();
		
		for(i = 1; i < n_pistolos; i++){
			pidC = fork();
			switch (pidC){
			case -1:
				perror("Error al crear el proceso");
				exit(-1);
			break;
			case 0:
				//Este es el proceso hijo
				//Nos aseguramos de que el bucle finalice para que no se puedan crear mas hijos
				i = 129;	
			break;
			default:
				fd = open("pistolos.txt", O_WRONLY | O_APPEND, 0777);
				if(fd == -1){
					perror("Error en la creacion del fichero");
					exit(-1);
				}
				lockf(fd,F_LOCK,0);
				if(sprintf(mensaje, "%d|",pidC)==0){
					perror("Error en la escritura del fichero");
				}
				if(write(fd,mensaje,strlen(mensaje))==-1){
					perror("Error en la escritura del fichero");
					exit(-1);
				}
				close(fd);
				//cuando llega al último pistolero, tras escribirlo, desbloquea el fichero
				if(i == (n_pistolos-1)){
					lockf(fd,F_ULOCK,0)==-1;
				}
			}
		}
		
		alarm(3);
		signal(SIGALRM,&signal_alarm);
		sigsuspend(&mask_alarm);
		if(getpid()!=pidP){
			fd = open("pistolos.txt", O_RDONLY | O_APPEND, 0777);
			if(fd == -1){
				perror("Error al abrir el fichero");
				exit(-1);
			}
			if(lseek(fd,0,SEEK_SET)==-1){
				perror("Error al mover el puntero del fichero");
				exit(-1);
			}
			tam = read(fd,linealeida1,16384);
			limites[0] = 0;
			for (j = 0; j < tam; j++){
				if(linealeida1[j] == '|'){
					limites[k] = j;
					k = k + 1;
				}
			}
			if(lseek(fd,0,SEEK_SET)==-1){
				perror("Error al mover el puntero del fichero");
				exit(-1);
			}
					
			for(j = 1; j < n_pistolos; j++){
				if(j==1){
					if(read(fd,linealeida2,limites[j])==-1){
						perror("Error al leer el fichero");
						exit(-1);
					}
				}else{
					limit = (limites[j]-limites[j-1]-1);
					if(read(fd,linealeida2,limit)==-1){
						perror("Error al leer el fichero");
						exit(-1);
					}
				}
				if(sscanf(linealeida2,"%d",&pidC2[j])==-1){
					perror("Error al alamacenar los PIDs en la variable");
					exit(-1);
				}
				if(lseek(fd,limites[j],SEEK_SET)==-1){
					perror("Error al desplazar el puntero del fichero");
					exit(-1);
				}
				if(lseek(fd,1,SEEK_CUR)==-1){
					perror("Error al desplazar el puntero del fichero");
					exit(-1);
				}
			}
					
			srand(getpid());
			do{
				matar = rand() % n_pistolos;
			}while(pidC2[matar] == getpid() || matar == 0);
			printf(CYAN_T"	%d->"ROJO_T "%d\n"RESET_COLOR,getpid(),pidC2[matar]);
			
			if(kill(pidC2[matar],SIGTERM)==-1){
				perror("Error al matar al pistolero");
				exit(-1);
			}
			if(sigsuspend(&mask)==-1){
				perror("Error al desenmascarar la señal SIGTERM");
				exit(-1);
			}
		}else{
			alarm(1);
			signal(SIGALRM,&signal_alarm);
			sigsuspend(&mask_alarm);
			printf(AZUL_T"Soy el padre con PID = %d\n"RESET_COLOR,getpid());
			for(i = 1; i < n_pistolos_o; i++){
				
				pidM[i] = wait(&status);
				if(pidM[i]==-1){
					perror("Error en la espera por la muerte de los hijos");
				}
				if(WEXITSTATUS(status)==0){
					printf(AMARILLO_T"El pistolero con PID = %d ha muerto\n"RESET_COLOR,pidM[i]);
					n_pistolos = n_pistolos-1;
				}
			}
		}
	    return 0;
    }
}
