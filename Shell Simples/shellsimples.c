/***************************************************************
  DISCIPLINA: 203523 - Sistemas Operacionais 

  Shell para execução de comandos simples;
     - Esse código roda em Linux;
	 - Mateus Junges 	
***************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main( void )
{
  printf("Sistemas Operacionais 2018 - Mateus Junges");	
  printf("\nPressione qualquer tecla para continuar";
  getchar();
	
 char lc[ 81 ];
 char *argv[ 20 ];
 int pid, i, status;

 while( 1 ) {
 	printf( "Prompt > " );
	gets( lc );  //recebe comando de entrada
	if( ! strcmp( lc, "" ) ) //se vazio, cancela
 		continue;
 	argv[ 0 ] = strtok( lc, " " );
 	if( ! strcmp( argv[ 0 ], "exit" ) ) /* O campo argv[0] recebe
						o nome do comando passado,
						Se for 0, passa o comando exit(0),
						que encerra o programa */
 		exit( 0 );
	i = 1;

	while( i < 20 && (argv[ i ] = strtok( NULL, " " )) )// char * strtok ( char * str, const char * delimiters ); 
	
			/*
				maximo de 20 char
				A função strtok() dá split no array, 
				usando um espaço em branco como delimitador, 
				e, assim, pega cada arguemento que foi
				passado no comando
			*/
		 ++i;
	if( (pid = fork()) == -1 ) {		/*valor negativo = erro*/
		 printf( "Erro no fork\n" );
		 exit( 1 );
	}
	if( pid == 0 ) 				/*se for processo filho */
	if( execvp( argv[ 0 ], argv ) ) {	/*Executa o vetor de parametros passado	*/
		printf( "Comando nao reconhecido\n" );
		exit( 1 );
 	}
	 wait( &status );
 }
}
