#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>

const int MED = 200;

using namespace std;

/* Estrutura para armazenar as notas: */
struct notas{
    int   id_aluno;
    float nota1;
    float nota2;
    float nota3;
};

/* Estrutura para armazenar as medias: */

struct medias{
    int id_media;
    float media;
};

int comparacao(const void *valorA, const void *valorB);
int main(){

	using namespace std;
    /*
        Pegar o tempo de inicio e fim do programa:
    */
    time_t tempo_inicio;
    time_t tempo_fim;

    struct tm *t_inicio;
    struct tm *t_fim;

    time(&tempo_inicio);

    t_inicio = localtime(&tempo_inicio);
    cout << "Tempo inicio -> " << asctime(t_inicio) << endl;

    struct notas students[MED]; /* Gera um vetor de struct notas */
    struct medias medias_alunos[MED]; /* Gera um vetor de struct media */

    FILE *fp; /* Ponteiro para arquivo */

    int i; /* Variavel para armazenar na posição correta do vetor */
    i = 0;


    char nome[20];
    cout << "Digite o nome do arquivo de notas: " << endl;
    cin >> nome;

    fp = fopen(nome, "r");

    /* Ler todas as notas e armazenar no vetor,
       enquanto não chegar no fim do aquivo ( !feof() )
     */
    while( !feof(fp) ){
        fscanf( fp, "%d %f %f %f\n", &students[i].id_aluno,
                                     &students[i].nota1,
                                     &students[i].nota2,
                                     &students[i].nota3 );
        i++;
    }

    int j;
    j = 0;

    const int maximo = i; /*Variavel para saber até onde ordenar a struct */

    while ( j <= i ){
        /* Verificar se o aluno possui duas ou tres notas,
           antes de fazer o cálculo da média:
        */

        if( students[j].nota3 ){/*Se existir a nota3: */
            medias_alunos[j].media = (
                            students[j].nota1 +
                            students[j].nota2 +
                            students[j].nota3 ) / 3;
            medias_alunos[j].id_media = students[j].id_aluno;
        } else{
            medias_alunos[j].media = (
                            students[j].nota1 +
                            students[j].nota2 ) / 2;
            medias_alunos[j].id_media = students[j].id_aluno;

        }
        j++;
    }

    /* Ordenar a struct de medias: */
    qsort(medias_alunos, maximo, sizeof(struct medias), comparacao);

    j = 0;

    while( j < maximo ){
        cout << medias_alunos[j].id_media << " " << medias_alunos[j].media << endl;
        j++;
    }

    time(&tempo_fim); /* Tempo final */
    t_fim = localtime(&tempo_fim);
    cout << "Tempo inicio -> " << asctime(t_fim);

}

/* Função que uso para comparar dois valores
na hora de ordenar a struct */

int comparacao(const void *valorA, const void *valorB){

    if ((*(struct medias*)valorA).media > (*(struct medias*)valorB).media) /* Se o valor A for maior que o valor B, ele vem antes */
        return -1;
    else if ( (*(struct medias*)valorA).media < (*(struct medias*)valorB).media )
            return 1; /* Se o valor A for menor que o valor B, vem depois */
    else
        return 0;
}


