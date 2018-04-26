#include <stdio.h>											//biblioteca de entrada e saida								
#include <unistd.h>											//para processos e pipes
#include <stdlib.h>											//para exit() e afins			
#include <fcntl.h>											//manipulate file descriptor
#include <time.h>											//controle do tempo
#include <sys/types.h>										//para processos e afins
#include <sys/wait.h>										//para processos e afins
#include <pthread.h>										//para threads
#define TAM_PIPE 100										//numero de notas que cada pipe vai calcular
#ifndef N_NOTAS												// define para numeros de notas
#define N_NOTAS  50										//valor pre definido
#endif
#define AZUL "\033[34m"										//cor do shell em AZUL
#define VERDE "\033[32m"									//cor do shell em VERDE
#define LAR "\033[33m"										//cor do shell em LARANJA
#define VERMELHO "\033[31m"									//cor do shell em VERMELHO
#define PRETO "\033[0m"										//cor do shell em PRETO
#define NEGRITO "\x1b[1m"									//texto em negrito
int b = 0;													//variavel de controle para funcao de threads
float notas2[N_NOTAS];										//vetor que guarda medias calculadas em threads
void *thread(){												//funcao para calcular trheads
	FILE *arq = fopen("nota.lst", "r");						//abrindo arquivo
	int k = b, i, ids;										//ids = id da nota, i e k variaveis de controle para o laço
  	float n1, n2, n3, media;								//variaveis para nota
	if(arq == NULL){ 										//se arquivo não abrir corretamento
		printf("Erro, problema no arquivo!!\n");			//mensagem de erro
		exit(1);											//sai do programa
	}
	for(i = k; i < (k + TAM_PIPE); i++){					//calcula notas em um intervalo especifico
		fseek(arq, i * 19,SEEK_SET);						//posiciona o ponteiro em uma determinada linha no codigo
		fscanf( arq, "%i %f %f %f\n", &ids, &n1, &n2, &n3);	//le a linha do arquivo com formatação
   		if((n1 + n2) >= 14.0){								//se a media for acima de 14 divide por 2
			notas2[i] = (n1 + n2) / 2.0;					//calcula media
		}else{												//senão
			notas2[i] = (n1 + n2 + n3) / 3.0;				//calcula media com exame
		}
	}
	fclose(arq);											//fecha arquivo
	b += TAM_PIPE;											//variavel de controle é atualizada para novo intervalo de medias
    pthread_exit(NULL);										//threads exit
}
int main( void ){											//funcao principal
	int i, fd, ids, status, n_processos;					//variavel de controle, para retorno do exit, e numero de processos ou threads
  	float n1, n2, n3, media;								//para manipular as notas
  	clock_t inicio;											//para capturar inicio do clock 
  	double tempo[5];										//tempo de cada operação
  	pid_t pids[N_NOTAS];									//vetor de pids
  	float nota[N_NOTAS], nota1[N_NOTAS];					//vetor para medias calculadas com processos e pipe
  	FILE *arq;												//ponteiro de arquivo
  	inicio = clock();										//captura o clock	
  	fd = open("nota.lst",O_WRONLY | O_CREAT| O_TRUNC, 0666);//cria arquivo, caso nao exista
  	if( fd < 0 ) {											//se menor que 0, deu problema
    	printf("Erro, problema no arquivo!!\n" );			//mensagem de erro
    	exit( fd );											//fecha programa
  	}
  	for( i = 0; i < N_NOTAS; ++i ) {						//laco para gerar as notas
    	n1 = rand()%100/10.0;								//nota aleatoria emtre 0 e 10
    	n2 = rand()%100/10.0;								//nota aleatoria emtre 0 e 10
    	n3 = rand()%100/10.0;								//nota aleatoria emtre 0 e 10
    	if( n1 + n2 >= 14.0 ){								//se media é acima de 14 individuo passou
    		n3 = 0.0;										//zera n3
    	}
    	if(i < 10){											//se numero de digitos do id for 1
    		dprintf( fd, "     %d %.1f %.1f %.1f\n", i, n1, n2, n3 );//escreve no arquivo
    	}else if(i >= 10 && i < 100){						//se numero de digitos do id for 2
    		dprintf( fd, "    %d %.1f %.1f %.1f\n", i, n1, n2, n3 );//escreve no arquivo
    	}else if(i >= 100 && i < 1000){						//se numero de digitos do id for 3
    		dprintf( fd, "   %d %.1f %.1f %.1f\n", i, n1, n2, n3 );//escreve no arquivo
    	}else if(i >= 1000 && i < 10000){					//se numero de digitos do id for 4
    		dprintf( fd, "  %d %.1f %.1f %.1f\n", i, n1, n2, n3 );//escreve no arquivo
    	}else if(i >= 10000 && i < 100000){					//se numero de digitos do id for 5
    		dprintf( fd, " %d %.1f %.1f %.1f\n", i, n1, n2, n3 );//escreve no arquivo
    	}else if(i >= 100000 && i < 1000000){				//se numero de digitos do id for 6
    		dprintf( fd, "%d %.1f %.1f %.1f\n", i, n1, n2, n3 );//escreve no arquivo
    	}
  	}
  	close( fd );											//fecha arquivo
  	tempo[0] = (clock() - inicio) / (double)CLOCKS_PER_SEC;	//calcula tempo para criacao do arquivo
//==============================================================================================processos
	inicio = clock();														//captura clock
	i = 0;																	//zera variavel de controle
	printf("\n\t\t%s%s=======================\n", NEGRITO, VERDE);			//impressao de separador
	printf("\t\t  Media com Processos  \n");								//mensagem de tela
	printf("\t\t%s%s=======================%s\n", NEGRITO, VERDE, AZUL);	//impressao de separador
	while(i < N_NOTAS){														//enquato nao atingir numero maximo de notas
        pids[i] = fork();													//cira um processo	
		if(pids[i] == 0){													//verifica se é filho
			arq = fopen("nota.lst", "r");									//abre arquivo para leitura
			if(arq == NULL)													//se der erro no arquivo
				printf("Erro, problema no arquivo!!\n");					//imprime erro

			while( fscanf( arq, "%i %f %f %f\n", &ids,&n1,&n2,&n3) != EOF){	//le as linhas do arquivo
				if(ids == i){												//se valor do id for o do procurado
					break;													//sai do laco
				}
        	}
        	fclose(arq);													//fecha o arquivo
			if((n1 + n2) >= 14.0){											//se soma for maior que 14
				media = (n1 + n2) / 2;										//divide por dois
			}else{															//senao
				media = (n1 + n2 + n3) / 3;									//divide por 3
			}
            printf("\tFilho -> %d ID = %i\n",getpid(),i );					//imprime id e pid do filho
            exit( (int)(media * 10) );										//retorna valor da media do filho especifico
        }
        i++;																//incrementa controle
    }
    i = 0;																	//zera variavel de controle
    while(i < N_NOTAS){														//laco para pegar retorno de todos os pids
        if(pids[i] > 0){													//se for o pai
            waitpid(pids[i], &status, 0 );									//pega retorno do processo filho de id especifico
            if( WIFEXITED(status) ){										//se filho terminou com sucesso
            	nota[i] = (float)(WEXITSTATUS(status)/10.0);				//add nota para vetor de processos
                printf("\tFilho -> %i retornou Media = %.1f\n" , pids[i], nota[i]);//imprime retorno do filho
            }else
                printf("\nFilho (%d) nao terminou bem!!\n", pids[i]);		//erro para processo com problemas
        }
        i++;																//incrementa variavel de controle
    }
  	tempo[1] = (clock() - inicio) / (double)CLOCKS_PER_SEC;					//calcula tempo que os processo levaram para realizar media
//==============================================================================================pipes
  	inicio = clock();
  	printf("\n\t\t%s%s=======================\n", NEGRITO, VERDE);			//impressao de separador
	printf("\t\t    Media com Pipes  \n");									//mensagem de tela
	printf("\t\t%s%s=======================%s\n", NEGRITO, VERDE, AZUL);	//impressao de separador
    n_processos = N_NOTAS / TAM_PIPE;										//variavel para numeros de processos necessarios para determinado bloco de notas
	int fd2[n_processos][2];												//vetor para leitura e escrita em pipe
	int j = 0, k = 0;														//variaveis de controle
    i = 0;																	//sera variavel de controle
    while(i < n_processos){													//laco que define numero de vezes para processos
    	pipe(fd2[i]);														//cria o pipe
        pids[i] = fork();													//cria um processo
    	if(pids[i] == 0){													//se for filho
            float media[TAM_PIPE];											//variavel interna que recebe intervalo de medias
            close(fd2[i][0]);												//fecha leitura do pipe
            int a = 0;														//variavel iterna de controle
            arq = fopen("nota.lst", "r");									//abre arquivo para leitura
			if(arq == NULL){ printf("Erro no arquivo\n");}					//caso o arquivo nao abra com sucesso

			printf("\t%sCalculando media de %i ate %i\n",LAR, k, k + TAM_PIPE);//imprime intervalo que vaii calcular
			for(j = k; j < (k + TAM_PIPE); j++){							//laco para determina intervalo de notas
				fseek(arq,j * 19,SEEK_SET);									//funcao que desloca ponteiro para determinado local no arquivo
				fscanf( arq, "%i %f %f %f\n", &ids, &n1, &n2, &n3);			//le linha do arquivo formatada
   				if((n1 + n2) >= 14.0){										//se media for maior que 14
					media[a] = (n1 + n2) / 2.0;								//divide por 2
				}else{														//senao
					media[a] = (n1 + n2 + n3) / 3.0;						//divide por 3
				}
				printf("\t%i  %.1f\n ",ids,  media[a]);						//imprime media
				a++;														//incrementa variavel
			}
			fclose(arq);													//fecha arquivo
            write(fd2[i][1], &media, sizeof(media));						//escreve vetor com resultados no pipe
            close(fd2[i][1]);												//fecha pipe de escrita
            exit(0);														//retorna para o pai
        }else if(pids[i] > 0){												//se for pai
            float medias[TAM_PIPE];											//variavel interna para bloco de notas
            close(fd2[i][1]);												//fecha escrita
            int m;															//variavel interna de controle
            read(fd2[i][0], &medias, sizeof(medias));						//le media do pipe
            for(j = 0, m = k; j < TAM_PIPE; j++, m++){						//armazena bloco de notas no vetor de media de pipes
            	nota1[m] = medias[j];										//atribuicao
    		}
    		close(fd2[i][0]);												//fecha leitura
        }
        i++; 																//incrementa variavel de controle 
        k += TAM_PIPE;       												//incrementa intervalo
    }
	tempo[2] = (clock() - inicio) / (double)CLOCKS_PER_SEC;					//calcula tempo gasto pelos pipes
//==============================================================================================threads
	inicio = clock();														//captura tempo
	printf("\n\t\t%s%s=======================\n", NEGRITO, VERDE);			//impressao de separador
	printf("\t\t    Media com Threads  \n");								//mensagem de tela
	printf("\t\t%s%s=======================%s\n", NEGRITO, VERDE, VERMELHO);//impressao de separador
	pthread_t threads[n_processos];											//vetor de threads
	for(i = 0; i < n_processos; i++){										//numero de threads necessarias
		pthread_create(&threads[i], NULL, thread, NULL);					//cria as threads 								
    }
    for (i = 0; i < n_processos; i++){										//numero de threads necessarias
    	pthread_join(threads[i], NULL);										//retorno das threads
    }
    tempo[3] = (clock() - inicio) / (double)CLOCKS_PER_SEC;					//tempo gasto pelas threads
    for(i = 0; i < N_NOTAS; i++){											//laco para imprimir nota
    	printf("\t%i %.1f\n",i, notas2[i]);									//imprime nota
    }
//==============================================================================================ordenando
    inicio = clock();										//captura tempo
    int nd, aux;											//variaveis auxiliares
    float aux2 = 0.0;										//variaveis auxiliares
    nd = open( "notas.lst", O_WRONLY | O_CREAT | O_TRUNC, 0666 );//cria um novo arquivo
  	if( nd < 0 ) {											//se der erro no arquivo
    	printf( "Erro na criacao do arquivo\n" );			//mensagem de erro
    	exit( fd );											//sai do programa
  	}
	for(i = 0; i < N_NOTAS; i++){							//lista todas as notas
		for(j = 0; j < N_NOTAS; j++){						//laco para procurar maior nota
			if(nota1[j] > aux2){							//se nota for maior
				aux = j;									//recebe o indice
				aux2 = nota1[j];							//recebe nota maior
			}
		}
		aux2 = 0.0;											//zera variavel
		arq = fopen("nota.lst", "r");						//abre arquivo com as notas
		while(fscanf( arq, "%i %f %f %f\n", &ids, &n1, &n2, &n3) != EOF){//le linhas do arquivo
			if(ids == aux)									//se o indice for o procurado
				break;										//sai do laco
		}
		fclose(arq);										//fecha o arquivo
		dprintf( nd, "%d 	%.1f	%.1f	%.1f	%.1f	%.1f	%.1f\n", aux, n1, n2, n3, nota[aux], nota1[aux], notas2[aux]);	
		nota1[aux] = 0.0;									//zera o posicao do vetor
	}
  	close( nd );											//fecha o arquivo
  	int resp = remove("nota.lst");							//remove arquivo
    tempo[4] = (clock() - inicio) / (double)CLOCKS_PER_SEC;	//captura o tempo dos threads
//==============================================================================================tempos
  	printf("\t%sTempo para criacao do arquivo:--------------------%.8f\n",PRETO, tempo[0]); //imprime tempo de criacao do arquivo
	printf("\tTempo para calculo de media com varios processos:-%.8f\n", tempo[1]);			//imprime tempo de processamento com processos
	printf("\tTempo para calculo de media com pipes:------------%.8f\n", tempo[2]);			//imprime tempo de processamento com pipes
	printf("\tTempo para calculo de media com threads:----------%.8f\n", tempo[3]);			//imprime tempo de processamento com threads
	printf("\tTempo para ordenar arquivo------------------------%.8f\n", tempo[4]);			//imprime tempo de processamento para ordenacao
	printf("\tTempo total: 	  %.8f\n", (tempo[0] + tempo[1] + tempo[2] + tempo[3]) + tempo[4]);	//imprime tempo total
	return 0;
}
