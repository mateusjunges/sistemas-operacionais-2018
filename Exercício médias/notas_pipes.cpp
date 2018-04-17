#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <iostream>

   #define N_NOTAS  200
   #define  N_BLOCO 10


using namespace std;

struct notas{
	int id_nota;
	float nota1;
	float nota2;
	float nota3;
	float media;
	pid_t pid;
};


/*pipes*/
int numero_processos = N_NOTAS / N_BLOCO;
int fd[numero_processos][2];

void ler_arq(struct notas notas[]);

void print(struct notas notas[]);

void calcular_media(struct notas notas[]);

void media_fork(struct notas notas[]);


int comparacao(const void *a, const void *b);



int main(){

	struct notas notas[N_NOTAS];

	ler_arq(notas);

	 pipes(&fd);
	 print(notas);



	return 0;
}
float pipes(){
    pipe(fd[numero_processos]);
    pid_t pid[numero_processos];
    float medias[numero_processos];

    int i = j = k = z = 0;

    while(i < numero_processos){/*enquanto i menor que o numero de pipes do bloco*/
        pipe(fd[i]); /* Cria o pipe antes do fork para processos pai e filho terem os mesmos descritores de arquivo */
		pid[i] = fork(); /* Crio o processo filho */
        if(pid[i] == 0){ //é filho
            close(fd[i][0]); // fecha leitura do pipe
			
			cout << "Calcula media de " << k << "ate " << k+N_BLOCO << endl;
            for(j = k; j < k + N_BLOCO; j++){
                if (notas[i].nota1 +  notas[i].nota2 >= 14.0){
                    media  = ( ( notas[i].nota1 + notas[i].nota2 ) / 2.0);
                }else {
                    media = ( notas[i].nota1 + notas[i].nota2 + notas[i].nota3 ) / 3.0;
                }
				cout << notas[i].id_nota << " " << pid[i] << " " << media << endl;
                medias[z] = media;
                z++;
            }
            write(fd[i][1], &medias, sizeof(medias));
            close(fd[i][1]);
            exit(0);
        }

            float medias_1[numero_processos];
               if(pid[i] > 0){ // se pai
                   close(fd[i][1]); // fecha escrita
                   read(fd[i][0], &medias_1, sizeof(medias_1));

                   int m = 0;

                   for(j = 0, m = k; j < numero_processos; j++, m++){
                        notas[m].media = medias_1[j];
						
					}
                }
                i++;
                k += N_BLOCO;
        }


    }
}


void print(struct notas notas[]){
	int i = 0;
	while ( i < N_NOTAS ){
		cout << "\n" << notas[i].id_nota << " " << notas[i].nota1 << " "
		     << notas[i].nota2 << " " << notas[i].nota3 << " "
		     << notas[i].media << endl;
		i++;
	}
}


void ler_arq(struct notas notas[]){
	int   i;

	FILE *fp; /* Ponteiro para o arquivo com as notas */
	char nome[20];
	cout << "Digite o nome do arquivo: ";
	cin >> nome;

	fp = fopen(nome, "r");

    while(!feof(fp)){
		fscanf(fp, "%d %f %f  %f", &notas[i].id_nota,
                                   &notas[i].nota1,
                                   &notas[i].nota2,
                                   &notas[i].nota3);
		/*cout << notas[i].id_nota
                                    << " " << notas[i].nota1
                                  << " "<< notas[i].nota2
	                          << " " << notas[i].nota3 << endl;*/
	i++;
    }
	fclose(fp);
}

int comparacao(const void *valorA, const void *valorB){
	if((*(struct notas*)valorA).media > (*(struct notas*)valorB).media) /* Se o valor A for maior que o valor B, ele vem antes */
		return -1;
	else if( (*(struct notas*)valorA).media < (*(struct notas*)valorB).media ) /* Se for menor que B */
		return 1;
    else /* Se for igual a B */
		return 0;
}




