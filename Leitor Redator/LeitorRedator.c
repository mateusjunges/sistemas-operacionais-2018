#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

pthread_mutex_t no_wait, no_acc, counter;
int no_of_readers=0; //  número de leitores

void reader(void *arg)
{
   int id=*((int*)arg);
   printf("Leitor %d iniciou\n", id);
   while(1)
   {
      sleep(rand()%4); //dorme por um tempo
      check_and_wait(id);
      read(id); //lê id
   }
}

void writer(void* arg) // escritor
{
   int id=*((int*)arg);
   printf("Escritor %d iniciou\n", id);
   while(1)
   {
      sleep(rand()%5);
      check_and_wait_if_busy(id); // verifica e espera se ocupado
      write(id); //escreve
   }
}

/*sub functions for reader and writer threads*/
void check_and_wait_if_busy(int id)
{
   if(pthread_mutex_trylock(&no_wait)!=0){
      printf("scritor %d esperando\n", id);
      pthread_mutex_lock(&no_wait);
   }
}

void check_and_wait(int id)
{
   if(pthread_mutex_trylock(&no_wait)!=0){/*	tenta bloquear, e se ainda n 							estiver bloqueado, faz o 							bloqueio*/
      printf("Leitor %d esperando\n", id);
      pthread_mutex_lock(&no_wait);
   }
}

void read(int id)
{
   pthread_mutex_lock(&counter);
   no_of_readers++;//incrementa o número de leitores
   pthread_mutex_unlock(&counter);
   if(no_of_readers==1)
      pthread_mutex_lock(&no_acc);
   pthread_mutex_unlock(&no_wait);
   printf("Leitor %d lendo...\n", id);
   sleep(rand()%5); //dormir por um tempo
   printf("Leitor %d terminou de ler\n", id);
   pthread_mutex_lock(&counter);
   no_of_readers--; //decrementa o número de leitores
   pthread_mutex_unlock(&counter);
   if(no_of_readers==0)//se o número de leitores igual a 0
      pthread_mutex_unlock(&no_acc);
}

void write(int id)
{
   pthread_mutex_lock(&no_acc);
   pthread_mutex_unlock(&no_wait);
   printf("Esritor %d escrevendo...\n", id);
   sleep(rand()%4+2);
   printf("Escritor %d escrevendo\n", id);
   pthread_mutex_unlock(&no_acc);
}

/**************************************************/
//MAIN
int main(int argc, char* argv[])
{
   pthread_t R[5],W[5];
   int ids[5];
   for(int i=0; i<5; i++)
   {
      ids[i]=i+1;
      pthread_create(&R[i], NULL, (void*)&reader, (void*)&ids[i]); //cria leitor
      pthread_create(&W[i], NULL, (void*)&writer, (void*)&ids[i]); /* cria 									escritor */
   }
   pthread_join(R[0], NULL);
   exit(0);
}
