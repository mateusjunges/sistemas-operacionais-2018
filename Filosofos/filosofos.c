#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
 
#define N 5 // define que N será igual a 5 
#define LEFT (i+N-1)%N //esquerda
#define RIGHT (i+1)%N //direita
#define THINKING 0 // código usado para pensando
#define HUNGRY 1 // código que usei para filósofo com fome
#define EATING 2 // Código que usei para comendo

/* Funções utilizadas no código */
void mostrar();
void *acao_filosofi(void *j);
void pegar_garfo(int i);
void por_garfo(int i);
void test(int i);
void pensar(int i);
void comer(int i);


 
int state[N], i, int_rand;
float float_rand;
 
//semaforos
sem_t mutex;
sem_t sem_fil[N];
 
//funcao que mostra o estado dos N filosofos
void mostrar(){
   for(i=1; i<=N; i++){
       if(state[i-1] == THINKING){ // se status pensando
          printf("O filosofo %d esta pensando!\n", i);
       }
       if(state[i-1] == HUNGRY){ // se status com fome
          printf("O filosofo %d esta com fome!\n", i);
       }
       if(state[i-1] == EATING){ // se status comendo
          printf("O filosofo %d esta comendo!\n", i);
       }
   }
   printf("\n");
}
 
//acao tomada pelo filosofo
void *acao_filosofo(void *j){
   int i= *(int*) j;
   while(1){  
    pensar(i);
    pegar_garfo(i);
    comer(i); 
    por_garfo(i);
   }
 
}
 
void pegar_garfo(int i){
   sem_wait(&mutex);
   state[i]=HUNGRY; // status muda para com fome
   mostrar();
   test(i);
   sem_post(&mutex);
   sem_wait(&sem_fil[i]);
}
 
void por_garfo(int i){
   sem_wait(&mutex);
   state[i]=THINKING; // status muda para pensando
   mostrar();
   test(LEFT);
   test(RIGHT);
   sem_post(&mutex);
}
 
//funcao que testa se o filosofo pode comer
void test(int i){
   if(state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING){
       state[i]=EATING;
       mostrar();
       sem_post(&sem_fil[i]);
   } 
}
 
//a thread(filosofo) espera um tempo aleatoria pensando ou comendo
void pensar(int i){
 
   float_rand=0.001*rand();
   int_rand=float_rand;
   usleep(int_rand);
}
 
void comer(int i){
 
   float_rand=0.001*rand();
   int_rand=float_rand;
   usleep(int_rand);
}
 
int main(){
 
    for(i=0;i<N; i++){
       state[i]=0;
    }
        mostrar();
 
    int res;
 
    pthread_t thread[N];
 
    void *thread_result;
 
//inicia os semaforos
 
    res = sem_init(&mutex,0,1);
    if(res!=0){
       perror("Erro na inicialização do semaforo!");
       exit(EXIT_FAILURE);
    }
 
    for(i=0; i<N; i++){
       res = sem_init(&sem_fil[i],0,0);
       if(res!=0){
          perror("Erro na inicialização do semaforo!");
          exit(EXIT_FAILURE);
       }
    }
 
//cria as threads(filosofos)
 
    for(i=0; i<N; i++){
       res = pthread_create(&thread[i],NULL,acao_filosofo,&i);
       if(res!=0){
          perror("Erro na inicialização da thread!");
          exit(EXIT_FAILURE);
       }
    }
 
//join nas threads
 
        for(i=0; i<N; i++){
       res = pthread_join(thread[i],&thread_result);
       if(res!=0){
          perror("Ocorreu um erro ao fazer o join nas threads!");
          exit(EXIT_FAILURE);
       }
    }
	return 0;
}
