#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#ifndef N_NOTAS
    #define N_NOTAS  50
#endif

int b = 0;
float notas2[N_NOTAS];
void *thread(){
	FILE *arq = fopen("nota.lst", "r");
	int k = b, i, ids;
  	float n1, n2, n3, media;
	if(arq == NULL){
		printf("ERRO NO ARQUIVO\n");
		exit(1);
	}
	for(i = k; i < (k + TAM_PIPE); i++){
		fseek(arq, i * 19,SEEK_SET);
		fscanf( arq, "%i %f %f %f\n", &ids, &n1, &n2, &n3);
   		if((n1 + n2) >= 14.0){
			notas2[i] = (n1 + n2) / 2.0;
		}else{
			notas2[i] = (n1 + n2 + n3) / 3.0;
		}
	}
	fclose(arq);
	b += TAM_PIPE;
    pthread_exit(NULL);
}

int main( void ){
	int i, fd, ids, status, n_processos;
  	float n1, n2, n3, media;
  	clock_t inicio;
  	double tempo[5];
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



/*THREADS */
	inicio = clock();
	printf("\nMedia com Threads  \n");

	pthread_t threads[n_processos];
	for(i = 0; i < n_processos; i++){
		pthread_create(&threads[i], NULL, thread, NULL);
    }
    for (i = 0; i < n_processos; i++){
    	pthread_join(threads[i], NULL);
    }
    tempo = (clock() - inicio) / (double)CLOCKS_PER_SEC;
    for(i = 0; i < N_NOTAS; i++){
    	printf("\t%i %.1f\n",i, notas2[i]);
    }



    inicio = clock();
    float aux2 = 0.0;
	int nd, aux;
    nd = open( "notas.lst", O_WRONLY | O_CREAT | O_TRUNC, 0666 );
  	if( nd < 0 ) {
    	printf( "ERRO AO CRIAR O ARQUIVO\n" );
    	exit( fd );
  	}
	for(i = 0; i < N_NOTAS; i++){
		for(j = 0; j < N_NOTAS; j++){
			if(nota1[j] > aux2){
				aux = j;
				aux2 = nota1[j];
			}
		}
		aux2 = 0.0;
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

  	int resp = remove("nota.lst");


	printf("\nThreads: %.8f\n", tempo);
	return 0;
}

