#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <iostream>


#ifndef N_NOTAS
   #define N_NOTAS  20
#endif

using namespace std;

struct notas{
	int id_nota;
	float nota1;
	float nota2;
	float nota3;
	float media;
	pid_t pid;
};


void ler_arq(struct notas notas[]);

void print(struct notas notas[]);

void calcular_media(struct notas notas[]);

void media_fork(struct notas notas[]);

int comparacao(const void *valorA, const void *valorB);



int main(){

	struct notas notas[N_NOTAS];

	ler_arq(notas);
	media_fork(notas);
	print(notas);

	return 0;
}


void print(struct notas notas[]){
	int i = 0;
	while ( i < N_NOTAS ){
		cout << notas[i].id_nota << " " << notas[i].nota1 << " "
		     << notas[i].nota2 << " " << notas[i].nota3 << " "
		     << notas[i].media << endl;
		i++;
	}
}


void calcular_media(struct notas notas[]){
	float media_final;
	int i = 0;
	while( i<N_NOTAS ){
		if(notas[i].nota3){/* Se existir a nota 3: */
		    media_final = (
		                notas[i].nota1+
		                notas[i].nota2+
		                notas[i].nota3
		                  ) / 3.0;
		    notas[i].media = media_final;
		}else{ /* Se não existir a nota 3: */
		    media_final = (
		                notas[i].nota1+
		                notas[i].nota2
		                  ) / 2.0;
		    notas[i].media = media_final;
		}

	}

}

void ler_arq(struct notas notas[]){
	int   i;

	FILE *fp; /* Ponteiro para o arquivo com as notas */
	char nome[20];
	cout << "Digite o nome do arquivo: ";
	cin >> nome;

	fp = fopen(nome, "r");

    while(!feof(fp))
		fscanf(fp, "%d %f %f  %f", &notas[i].id_nota,
                                   &notas[i].nota1,
                                   &notas[i].nota2,
                                   &notas[i].nota3);
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

void media_fork(struct notas notas[]){

	pid_t num_pid;
	int i = 0;
	while ( i < N_NOTAS ){ /* Enquanto não terminar as médias */
		num_pid = fork();
		if (num_pid == 0){/* Se o processo for filho, entao eu calculo a média */
			float media;
			if (notas[i].nota1 +  notas[i].nota2 >= 14.0){ /*  Se a soma das duas primeiras notas for maior ou igual a 14,
                                                               não precisa a terceira nota
                                                            */
				media  = ( ( notas[i].nota1 + notas[i].nota2 ) / 2.0);
			}else
				media = ( notas[i].nota1 + notas[i].nota2 + notas[i].nota3 ) / 3.0;

			cout << getpid() << " " << media << endl;
			exit( media * 10.0 ); /* Passa o valor pelo exit */
		}else if(num_pid > 0) /* pid > 0 significa processo pai */
			notas[i].pid = num_pid;

		i++;
	}

        int contador = 0;
        while(contador < N_NOTAS){
            int status;
			waitpid(notas[contador].pid, &status,0);
			notas[contador].media = (float) WEXITSTATUS(status) / 10.0;
			cout << "PAI: " << notas[contador].pid << " "
			     << notas[contador].media << endl;
            contador++;
        }

}
