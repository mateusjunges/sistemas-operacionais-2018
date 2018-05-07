#include <stdio.h>
#include <stdlib.h>

struct hole{
	int tam;				//qual o tamanho do buraco
	struct hole *ant; //buraco anterior;
	struct hole *prox;	//ptr para a proximo buraco
};

struct hole *listaMemoria, *ptrNextFit; //ptr p/ lista de buracos
void *enderecoInicio;

int tamHole = sizeof(struct hole);

int memoria_base = 1024;

int mod(int valor){
	if(valor < 0)
		return valor * (-1);
	else return valor;
}

void mostra_mem(){
	int livre = 0, ocupada = 0, i=0;
	printf("\n");
	struct hole *atual = listaMemoria;
	while (atual != NULL) {
		if(atual->tam >= 0)
			livre += atual->tam;
		else ocupada += mod(atual->tam);
		printf("%d | ", atual->tam);
		atual = atual->prox;
		i++;
	}
	printf("\nTotal Livre: %d | Total Ocupada: %d\n", livre, ocupada+tamHole*(i-1));
}

void inicializa_mem(){
	listaMemoria = malloc(memoria_base); //listaMemoria aponta para um espaco de hole de espaco tam;
	//printf("%p", listaMemoria);
	listaMemoria->tam = memoria_base;
	listaMemoria->ant = NULL;
	listaMemoria->prox = NULL;
	ptrNextFit = listaMemoria;
	/*void *aux = listaMemoria + listaMemoria->tam;
	listaMemoria = aux;*/
}

void merge(){
	struct hole *atual = listaMemoria, *anterior, *proximo;
	int aux;
	while(atual != NULL){
		if(atual->tam >= 0){//O buraco esta livre e pode ser juntado
			proximo = atual->prox;
			if(proximo != NULL){//Se Nao esta no fim da lista
				if(proximo->tam >= 0){//Seu antecessor esta livre e pode ser juntado
					atual->prox = proximo->prox;
					aux = atual->tam + tamHole + proximo->tam;
					atual->tam = aux;
					if(proximo->prox != NULL) //Se nao estou no fim da lista
						proximo->prox->ant = atual; //Faco o anterior apontar pro atual
				}else{
					atual = atual->prox; //Se nao esta livre, vou para o proximo
				}
			}else{
				return; //Cheguei no fim da lista
			}
		}else{
			atual = atual->prox; //Se estiver sendo usado, vou pro proximo
		}
	}
}

void *firstfit(int tam){
	struct hole *atual, *anterior;
	void *aux, *aux2;
	atual = listaMemoria;
	while (atual != NULL) { //Percorro os buracos
		if(atual->tam >= (tam + tamHole)){ //Posso meu_aloca aqui
			int tamanhoOriginal = atual->tam;
			aux = atual + atual->tam - tam;
			anterior = atual;
			atual = aux;

			atual->ant = anterior;
			atual->prox = anterior->prox;
			anterior->prox = atual;
			anterior->tam = (-1)*tam; //Marco com -1 pra dizer que está sendo usada
			atual->tam = tamanhoOriginal-tam-tamHole;

			if(atual->prox != NULL){ //Se Nao Estou no fim da lista
				atual->prox->ant = atual; //faz o proximo apontar pra tras
			}
			printf("\nAlocado %d", tam);
			ptrNextFit = aux;//Ponteiro que diz onde foi a ultima alocacao, para usar NextFit
			return aux+tamHole;
		}
		anterior = atual;
		atual = atual->prox; //Continuo Percorrendo a Lista
	}
	printf("\nImpossivel Alocar\n");
	return NULL;
}

void *bestfit(int tam){
	struct hole *atual = listaMemoria, *ptrMenor = NULL, *anterior;
	void *aux;
	int menorTam = memoria_base;

	while(atual != NULL){//Percorro a lista para encontrar o menor buraco que se encaixa
		if(atual->tam >= tam+tamHole){//Posso alocar aqui
			if(atual->tam <= menorTam){
				menorTam = atual->tam; //salvo o tamanho para comparar
				ptrMenor = atual; //e um ponteiro para esse buraco
			}
		}
		atual = atual->prox;
	}

	if(ptrMenor == NULL){ //Quer dizer que nao achei nenhum lugar
		printf("\nImpossivel Alocar\n");
		return NULL;
	}
	if(menorTam == memoria_base)//Se a lista esta vazia, aponto pro inicio dela
		atual = listaMemoria;
	else
		atual = ptrMenor;
	int tamanhoOriginal = atual->tam;
	aux = atual + atual->tam - tam;
	anterior = atual;
	atual = aux;
	atual->ant = anterior;
	atual->prox = anterior->prox;
	anterior->prox = atual;
	anterior->tam = tamanhoOriginal-tam-tamHole;
	atual->tam = (-1)*tam; //Marco com -1 pra dizer que está sendo usada

	if(atual->prox != NULL){ //Se Nao Estou no fim da lista
		atual->prox->ant = atual; //faz o proximo apontar pra tras
	}
	printf("\nAlocado %d", tam);
	ptrNextFit = aux; //Ponteiro que diz onde foi a ultima alocacao, para usar NextFit
	return aux+tamHole;
}

void *worstfit(int tam){
	struct hole *atual = listaMemoria, *ptrMaior, *anterior;
	void *aux;
	int maiorTam = 0;

	while(atual != NULL){//Percorro a lista procurando o maior buraco
		if(atual->tam >= tam+tamHole){//Posso alocar aqui
			if(atual->tam > maiorTam){
				maiorTam = atual->tam; //Tamanho do buraco
				ptrMaior = atual; //Salvo o local do maior buraco
			}
		}
		atual = atual->prox;
	}
	if(maiorTam == 0){//Nao encontrei nenhum lugar
		printf("\nImpossivel Alocar!\n");
		return NULL;
	}

	atual = ptrMaior;
	int tamanhoOriginal = atual->tam;
	aux = atual + atual->tam - tam;
	anterior = atual;
	atual = aux;
	atual->ant = anterior;
	atual->prox = anterior->prox;
	anterior->prox = atual;
	anterior->tam = tamanhoOriginal-tam-tamHole;
	atual->tam = (-1)*tam; //Marco com -1 pra dizer que está sendo usada

	if(atual->prox != NULL){ //Nao Estou no fim da lista
		atual->prox->ant = atual; //faz o proximo apontar pra tras
	}
	printf("\nAlocado %d", tam);
	ptrNextFit = aux; //Ponteiro que diz onde foi a ultima alocacao, para usar NextFit
	return aux+tamHole;
}

void *nextfit(int tam){
	struct hole *atual, *anterior;
	void *aux;
	atual = ptrNextFit; //Ponteiro que diz onde foi a ultima alocacao
	do { //Percorro os buracos
		if(atual->tam >= (tam + tamHole)){ //Posso meu_aloca aqui
			int tamanhoOriginal = atual->tam;
			aux = atual + atual->tam - tam;
			anterior = atual;
			atual = aux;

			atual->ant = anterior;
			atual->prox = anterior->prox;
			anterior->prox = atual;
			anterior->tam = tamanhoOriginal-tam-tamHole;
			atual->tam = (-1)*tam; //Marco com -1 pra dizer que está sendo usada

			if(atual->prox != NULL){ //Nao Estou no fim da lista
				atual->prox->ant = atual; //faz o proximo apontar pra tras
			}
			printf("\nAlocado %d", tam);
			ptrNextFit = aux;
			return aux+tamHole;
		}
		anterior = atual;
		atual = atual->prox;
		if(atual == NULL)//Se cheguei no fim da lista, preciso apontar pro inicio novamente
			atual = listaMemoria;
	}while (atual != ptrNextFit); //Enquanto nao chego onde comecei
	printf("\nImpossivel Alocar\n");
	return NULL;
}

void *meu_aloca(int metodo, int tam){
	switch (metodo) {
		case 1:
			return firstfit(tam);
			break;
		case 2:
			return bestfit(tam);
			break;
		case 3:
			return worstfit(tam);
			break;
		case 4:
			return nextfit(tam);
			break;
		default:
			printf("\nErro!");
	}
}
void meu_libera(void *ponteiro){
	if(ponteiro == NULL){
		printf("\nPonteiro Nulo!\n");
		return;
	}

	struct hole *atual;
	void *aux;

	aux = ponteiro - tamHole; //Volto o ponteiro para o cabeça
	atual = aux;

	int tam = mod(atual->tam);//Marco com sinal positivo dizendo que esta disponivel
	atual->tam = tam;
	merge(); //Chamo merge para juntar os buracos vazios
	printf("\nDesalocado %d", tam);
}

int main(){
	inicializa_mem();
  /*void *a1  =  meu_aloca(1, 10);
  void *a2  =  meu_aloca(1, 5);
  void *a3  =  meu_aloca(1, 20);
  void *a4  =  meu_aloca(1, 30);
  void *a5  =  meu_aloca(1, 15);
  void *a6  =  meu_aloca(1, 40);
  void *a7  =  meu_aloca(1, 60);
  void *a8  =  meu_aloca(1, 80);
  void *a9  =  meu_aloca(1, 100);
  mostra_mem();
  meu_libera( a2 );
  meu_libera( a4 );
  meu_libera( a5 );
  meu_libera( a7 );
  meu_libera( a9 );
  mostra_mem();
  void *a10 =  meu_aloca(1, 3);
  mostra_mem();
  void *a11 =  meu_aloca(1, 20);
  mostra_mem();
  void *a12 =  meu_aloca(1, 12);
  mostra_mem();
  void *a13 =  meu_aloca(1, 40);
  mostra_mem();*/
	mostra_mem();
	void *teste = meu_aloca(1, 30);
	mostra_mem();
	void *teste2 = meu_aloca(1, 10);
	mostra_mem();
	meu_libera(teste);
	mostra_mem();
	meu_libera(teste2);
	mostra_mem();

	return 0;
}
