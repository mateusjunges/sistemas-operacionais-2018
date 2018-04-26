#include <stdio.h>																		
#include <unistd.h>										
#include <stdlib.h>												
#include <fcntl.h>											
#include <time.h>									
#include <sys/types.h>								
#include <sys/wait.h>								
#include <pthread.h>								
#define TAM_PIPE 100								
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
		printf("Erro, problema no arquivo!!\n");			
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
  	inicio = clock();										
  	fd = open("nota.lst",O_WRONLY | O_CREAT| O_TRUNC, 0666);
  	if( fd < 0 ) {											
    	printf("Erro, problema no arquivo!!\n" );			
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
  	tempo[0] = (clock() - inicio) / (double)CLOCKS_PER_SEC;	
	
	
	
	
//==============================================================================================processos
	inicio = clock();														
	i = 0;																	
	printf("\n=======================\n");			
	printf("  Media com Processos  \n");								
	printf("=======================\n");	
	while(i < N_NOTAS){														
        pids[i] = fork();													
		if(pids[i] == 0){													
			arq = fopen("nota.lst", "r");									
			if(arq == NULL)													
				printf("Erro,no arquivo\n");					

			while( fscanf( arq, "%i %f %f %f\n", &ids,&n1,&n2,&n3) != EOF){	
				if(ids == i){												
					break;													
				}
        	}
        	fclose(arq);													
			if((n1 + n2) >= 14.0){											
				media = (n1 + n2) / 2;										
			}else{															
				media = (n1 + n2 + n3) / 3;									
			}
            printf("\tFilho -> %d ID = %i\n",getpid(),i );					
            exit( (int)(media * 10) );										
        }
        i++;																
    }
    i = 0;																	
    while(i < N_NOTAS){														
        if(pids[i] > 0){													
            waitpid(pids[i], &status, 0 );									
            if( WIFEXITED(status) ){										
            	nota[i] = (float)(WEXITSTATUS(status)/10.0);				
                printf("\tFilho -> %i retornou Media = %.1f\n" , pids[i], nota[i]);
            }else
                printf("\nFilho (%d) nao terminou bem!!\n", pids[i]);		
        }
        i++;																
    }
  	tempo[1] = (clock() - inicio) / (double)CLOCKS_PER_SEC;					
	
	
	
	
//==============================================================================================pipes
  	inicio = clock();
  	printf("\n=======================\n");			
	printf("    Media com Pipes  \n");									
	printf("=======================\n");	
    n_processos = N_NOTAS / TAM_PIPE;										
	int fd2[n_processos][2];												
	int j = 0, k = 0;														
    i = 0;																
    while(i < n_processos){													
    	pipe(fd2[i]);														
        pids[i] = fork();												
    	if(pids[i] == 0){											
            float media[TAM_PIPE];									
            close(fd2[i][0]);											
            int a = 0;													
            arq = fopen("nota.lst", "r");									
			if(arq == NULL){ printf("Erro no arquivo\n");}				

			printf("\tCalculando media de %i ate %i\n", k, k + TAM_PIPE);
			for(j = k; j < (k + TAM_PIPE); j++){							
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
            write(fd2[i][1], &media, sizeof(media));				
            close(fd2[i][1]);												
            exit(0);														
        }else if(pids[i] > 0){												
            float medias[TAM_PIPE];										
            close(fd2[i][1]);												
            int m;															
            read(fd2[i][0], &medias, sizeof(medias));						
            for(j = 0, m = k; j < TAM_PIPE; j++, m++){						
            	nota1[m] = medias[j];										
    		}
    		close(fd2[i][0]);												
        i++; 																
        k += TAM_PIPE;       												
    }
	tempo[2] = (clock() - inicio) / (double)CLOCKS_PER_SEC;					

	
	
	
	
	
	
	/*    THREADS*/
	inicio = clock();												
	printf("\n=======================\n");		
	printf("\t    Media com Threads  \n");								
	printf("=======================\n");
	pthread_t threads[n_processos];										
	for(i = 0; i < n_processos; i++){								
		pthread_create(&threads[i], NULL, thread, NULL);										
    }
    for (i = 0; i < n_processos; i++){									
    	pthread_join(threads[i], NULL);									
    }
    tempo[3] = (clock() - inicio) / (double)CLOCKS_PER_SEC;			
    for(i = 0; i < N_NOTAS; i++){											
    	printf("\t%i %.1f\n",i, notas2[i]);									
    }

	
	
	
	
    inicio = clock();									
    float aux2 = 0.0;										
    nd = open( "notas.lst", O_WRONLY | O_CREAT | O_TRUNC, 0666 );
  	if( nd < 0 ) {											
    	printf( "Erro na criacao do arquivo\n" );			
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
    tempo[4] = (clock() - inicio) / (double)CLOCKS_PER_SEC;	

  	printf("\tTempo para criacao do arquivo:--------------------%.8f\n", tempo[0]); 
	printf("\tTempo para calculo de media com varios processos:-%.8f\n", tempo[1]);			
	printf("\tTempo para calculo de media com pipes:------------%.8f\n", tempo[2]);	
	printf("\tTempo para calculo de media com threads:----------%.8f\n", tempo[3]);			
	printf("\tTempo para ordenar arquivo------------------------%.8f\n", tempo[4]);			
	printf("\tTempo total: 	  %.8f\n", (tempo[0] + tempo[1] + tempo[2] + tempo[3]) + tempo[4]);	
	return 0;
}
