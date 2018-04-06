#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

using namespace std;

	int vet_pipe[2]; /* Vetor para criar o pipe */
	int res;
	int estado;
	int d;
	
	pid_t id_processo;
	char buffer[200];


	int criapipe(){
		pipe(vet_pipe);
		return 0;	
	}

	int main(){

		FILE *fp;
		fp = popen( "sort file.txt", "r" );

		if( criapipe() == 0 ) /* Crio o pipe antes 
					do fork(), para 
					que o processso filho e o pai
					tenham os mesmos descritores
					de arquivo. Se o resultado 
					for igual 0, significa
					que deu certo
				      */
		id_processo = fork(); /* Faço o fork */		
		if( id_processo == 0 ){ /* Processo filho */
			/*
			*Descritores de arquivo do processo filho
			*		
			* 0 = stdin
			* 1 = stdout
			* 2 = stderr
			* 3 = vet_pipe[0]
			* 4 = vet_pipe[1]			
			*/
			close(vet_pipe[0]); /* Fecha o stdin do 
					       pipe para reutilizar em 
					       seguida		
					    */
			dup2( vet_pipe[1], 1 ); /* Faço uma cópia do descritor 
						   de escrita do pipe para a saída 
					           do processo, que no caso é o stdout (1)          		
						 */		
			/*
				Agora, depois de executar o close e o dup2, 
				os descritores de arquivo do processo filho
				
				0 = stdin
				1 = stdout
				2 = stderr
				3 = NULL
				4 = vet_pipe[1]

			*/

			close( vet_pipe[1] ); /* Fecho o descritor de escrita do pipe 
						a saída do processo filho vai ser escrita 
						no pipe 							
						*/
			execlp( "sort", "sort", "file.txt", 0);
		}
		else {
	 		/* Processo Pai */
			close(vet_pipe[1]); /* Fecha o descritor de escrita do pipe */
			while((d = read( vet_pipe[0], buffer, 200 ))){ /* Lê os dados a partir do pipe */
				buffer[d] = '\0';
				cout << buffer << endl;
			}
			wait( &estado );
		}
		pclose( fp );
		
	}
