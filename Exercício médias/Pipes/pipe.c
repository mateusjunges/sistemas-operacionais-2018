#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#define TAMANHOPIPE 10
#ifndef N_NOTAS
#define N_NOTAS  50
#endif

int b = 0;
float notas2[N_NOTAS];

int main( void ){
	printf("Sistemas Operacionais 2018 - Mateus Junges");	
	printf("\nPressione qualquer tecla para continuar";
	getchar();
	
	int i, fd, ids, status, n_processos;
  	float n1, n2, n3, media;
  	clock_t startedAt;
  	double time_exec;
  	pid_t pids[N_NOTAS];
  	float nota[N_NOTAS], nota1[N_NOTAS];
  	FILE *arq;
  	fd = open("nota.lst",O_WRONLY | O_CREAT| O_TRUNC, 0666);
  	if( fd < 0 ) {
    	printf("ERRO NO ARQUIVO\n" );
    	exit( fd );
  	}
  	for( i = 0; i < N_NOTAS; ++i ) {
    	n1 = rand()%100/10.0;
    	n2 = rand()%100/10.0;
    	n3 = rand()%100/10.0;
    	if( n1 + n2 >= 14.0 ){
    		n3 = 0.0;
    	}
    	if(i < 10){
    		dprintf( fd, "     %d %.1f %.1f %.1f\n", i, n1, n2, n3 );
    	}else if(i >= 10 && i < 100){
    		dprintf( fd, "    %d %.1f %.1f %.1f\n", i, n1, n2, n3 );
    	}else if(i >= 100 && i < 1000){
    		dprintf( fd, "   %d %.1f %.1f %.1f\n", i, n1, n2, n3 );
    	}else if(i >= 1000 && i < 10000){
    		dprintf( fd, "  %d %.1f %.1f %.1f\n", i, n1, n2, n3 );
    	}else if(i >= 10000 && i < 100000){
    		dprintf( fd, " %d %.1f %.1f %.1f\n", i, n1, n2, n3 );
    	}else if(i >= 100000 && i < 1000000){
    		dprintf( fd, "%d %.1f %.1f %.1f\n", i, n1, n2, n3 );
    	}
  	}
  	close( fd );

	startedAt = clock();

	printf("\n\n\nMedia com Pipe\n");
	
    n_processos = N_NOTAS / TAMANHOPIPE;
	int file_descriptor[n_processos][2];
	int j = 0, k = 0;
    i = 0;
    while(i < n_processos){
    	pipe(file_descriptor[i]);
        pids[i] = fork();
    	if(pids[i] == 0){
            float media[TAMANHOPIPE];
            close(file_descriptor[i][0]);
            int a = 0;
            arq = fopen("nota.lst", "r");
			if(arq == NULL){ printf("ERRO NO ARQUIVO\n");}

			printf("\n\tCalculando media no intervalo de %i ate %i\n\n", k, k + TAMANHOPIPE);
			for(j = k; j < (k + TAMANHOPIPE); j++){
				fseek(arq,j * 19,SEEK_SET);
				fscanf( arq, "%i %f %f %f\n", &ids, &n1, &n2, &n3);
   				if((n1 + n2) >= 14.0){
					media[a] = (n1 + n2) / 2.0;
				}else{
					media[a] = (n1 + n2 + n3) / 3.0;
				}
				printf("\t%i  %.1f\n ",ids,  media[a]);
				a++;
			}
			fclose(arq);
            write(file_descriptor[i][1], &media, sizeof(media));
            close(file_descriptor[i][1]);
            exit(0);
        }else if(pids[i] > 0){
            float medias[TAMANHOPIPE];
            close(file_descriptor[i][1]);
            int m;
            read(file_descriptor[i][0], &medias, sizeof(medias));
            for(j = 0, m = k; j < TAMANHOPIPE; j++, m++){
            	nota1[m] = medias[j];
    		}
    		close(file_descriptor[i][0]);
		}
        i++;
        k += TAMANHOPIPE;
    }
	time_exec = (clock() - startedAt) / (double)CLOCKS_PER_SEC;



	/* ORDENA��O */

    int nd, aux;
    float varAux = 0.0;
    nd = open( "notas.lst", O_WRONLY | O_CREAT | O_TRUNC, 0666 );
  	if( nd < 0 ) {
    	printf( "Erro na criacao do arquivo\n" );
    	exit( fd );
  	}
	for(i = 0; i < N_NOTAS; i++){
		for(j = 0; j < N_NOTAS; j++){
			if(nota1[j] > varAux){
				aux = j;
				varAux = nota1[j];
			}
		}
		varAux = 0.0;
		arq = fopen("nota.lst", "r");
		while(fscanf( arq, "%i %f %f %f\n", &ids, &n1, &n2, &n3) != EOF){
			if(ids == aux)
				break;
		}
		fclose(arq);
		dprintf( nd, "%d 	%.1f	%.1f	%.1f	%.1f	%.1f	%.1f\n", aux, n1, n2, n3, nota[aux], nota1[aux], notas2[aux]);
		nota1[aux] = 0.0;
	}

  	close( nd );
  	int resposta = remove("nota.lst");

	printf("\ntime_exec: %.8f\n", time_exec);
	return 0;
}
