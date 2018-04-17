#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#define TAM_PIPE 20

#ifndef N_NOTAS
    #define N_NOTAS  200
#endif

int main(){


    printf("\n\nMedia com Pipes\n");

    int n_processos = N_NOTAS / TAM_PIPE;/* variavel para numeros de processos necessarios para determinado bloco de notas */
    int fd2[n_processos][2];/* vetor para leitura e escrita em pipe */
    int j = 0;
    int k = 0;
        int i = 0;
    int ids;
    int n1 = 0;
    int n2 = 0;
    int n3 = 0;
    int nota1[N_NOTAS];
FILE *arq;
    pid_t pids[n_processos];
    while(i < n_processos){/* numero de vezes por processos */
        pipe(fd2[i]); /*cria o pipe */
        pids[i] = fork();/* cria um processo */
        if(pids[i] == 0){/* se filho */
            float media[TAM_PIPE];
            close(fd2[i][0]);/* fecha leitura do pipe */
            int a = 0;
            arq = fopen("notas.lst", "r");
        if(arq == NULL){
                printf("Erro no arquivo\n");
                exit(-1);
            }

            printf("\nCalculando media de %d ate %d\n", k, k + TAM_PIPE);/* intervalo de calculo atual */
            for(j = k; j < (k + TAM_PIPE); j++){/* intervalo das notas */
                fseek(arq,j * 19,SEEK_SET);/* funcao que desloca ponteiro para determinado local no arquivo */
                fscanf( arq, "%d %f %f %f\n", &ids, &n1, &n2, &n3);/* le linha do arquivo formatada */
                   if((n1 + n2) >= 14.0){
                    media[a] = (n1 + n2) / 2.0;
                }else{
                    media[a] = (n1 + n2 + n3) / 3.0;
                }
                printf("%d  %.1f\n ",ids,  media[a]);
                a++;
            }
            fclose(arq);
            write(fd2[i][1], &media, sizeof(media));/* escreve vetor com resultados no pipe */
            close(fd2[i][1]);/* fecha pipe de escrita */
            exit(0); /* retorna ao pai */
        }else if(pids[i] > 0){/* se processo pai */
            float medias[TAM_PIPE];
            close(fd2[i][1]);/* fecho a escrita */
            int m;
            read(fd2[i][0], &medias, sizeof(medias));/* le a media do pipe */
            for(j = 0, m = k; j < TAM_PIPE; j++, m++){
                nota1[m] = medias[j];
            }
            close(fd2[i][0]);/* fecho a leitura */
        }
        i++;
        k += TAM_PIPE;
    }
}
