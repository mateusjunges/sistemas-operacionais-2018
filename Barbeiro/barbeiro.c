#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define CADEIRAS 5                /* número de cadeiras para os clientes à espera */
#define TRUE 1

sem_t cliente;                  /* número de cliente que estao esperando atendimento */
sem_t barbeiro;                  /* número de barbeiros à espera de clientes */
sem_t mutex;                    /* para exclusão mútua */
int esperando = 0;                /* clientes que estão esperando */



void cortando();
void cliente_chegou();
void sendoatendido();
void desistiu();

int main() {
sem_init(&cliente, TRUE, 0);
sem_init(&barbeiro, TRUE, 0);
sem_init(&mutex, TRUE, 1);

pthread_t b, c;

 /* criando barbeiro */
 pthread_create(&b, NULL, (void *) barbeiro, NULL);


 /* criação de clientes */
 while(TRUE) {
pthread_create(&c, NULL, (void *) cliente, NULL);
sleep(1);
}

return 0;
}

void* barbeiro(void *arg) {
while(TRUE) {
sem_wait(&cliente);   /* vai dormir se o número de clientes for 0 */
sem_wait(&mutex);       /* obtém acesso a 'esperando' */
esperando = esperando - 1;  /*decrementa o contador de clientes à espera */
sem_post(&barbeiro);     /* um barbeiro está agora pronto para cortar cabelo */
sem_post(&mutex);       /* libera 'esperando' */
cortando();             /* corta o cabelo (fora da região crítica) */
}

pthread_exit(NULL);
}

void* cliente(void *arg) {
sem_wait(&mutex);           /* entra na região crítica */

if(esperando < CADEIRAS) {      /* se não houver cadeiras vazias, vá embora */
cliente_chegou();
esperando = esperando + 1;  /* incrementa o contador de clientes que estao esperando  */
sem_post(&cliente);   /* Se precisar, acorda o barbeiro */
sem_post(&mutex);       /* libera o acesso a 'esperando' */
sem_wait(&barbeiro);     /* vai dormir se o número de barbeiros livres for 0 */
sendoatendido();          /* cliente está sentado e sendo atendido pelo barbeiro */
} else {
sem_post(&mutex);       /* a barbearia está cheia, o cliente não espera e vai embora */
desistiu();

}

pthread_exit(NULL);
}

void cortando() {
printf("Barbeiro cortando cabelo de um cliente!\n");
sleep(3);
}

void cliente_chegou() {
printf("Um cliente chegou\n");
}
void sendoatendido() {
printf("Cliente sendo atendido\n");
}

void desistiu() {
printf("O cliente foi embora porque a barbearia esta cheia\n");
}
