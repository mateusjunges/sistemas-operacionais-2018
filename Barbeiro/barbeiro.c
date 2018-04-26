#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define CADEIRAS 5                /* número de cadeiras para os clientes esperarem */
#define TRUE 1

sem_t sem_clientes;                /* número de cliente à espera de atendimento */
sem_t sem_barbeiros;                  /* número de barbeiros à espera de clientes */
sem_t mutex;                    /* exclusão mútua */
int esperando = 0;                /* clientes que estão esperando */

/* protótipos */
void* barbeiro(void *arg);
void* cliente(void *arg);
void cortarCabelo();
void clienteChegou();
void cortandoCabelo();
void clienteDesistiu();

int main() {
sem_init(&sem_clientes, TRUE, 0);
sem_init(&sem_barbeiros, TRUE, 0);
sem_init(&mutex, TRUE, 1);

pthread_t b, c;

 /* criando único barbeiro */
 pthread_create(&b, NULL, (void *) barbeiro, NULL);


 /* criação indefinida de clientes */
 while(TRUE) {
pthread_create(&c, NULL, (void *) cliente, NULL);
sleep(1);
}

return 0;
}

void* barbeiro(void *arg) {
while(TRUE) {
sem_wait(&sem_clientes);   /* vai dormir se o número de clientes for 0 */
sem_wait(&mutex);       /* obtém acesso a 'waiting' */
esperando = esperando - 1;  /*descresce de um o contador de clientes à espera */
sem_post(&sem_barbeiros);     /* um barbeiro está agora pronto para cortar cabelo */
sem_post(&mutex);       /* libera 'waiting' */
cortarCabelo();             /* corta o cabelo (fora da região crítica) */
}

pthread_exit(NULL);
}

void* cliente(void *arg) {
sem_wait(&mutex);           /* entra na região crítica */

if(esperando < CADEIRAS) {      /* se não houver cadeiras vazias, saia */
clienteChegou();
esperando = esperando + 1;  /* incrementa o contador de clientes à espera */
sem_post(&sem_clientes);   /* acorda o barbeiro se necessário */
sem_post(&mutex);       /* libera o acesso a 'waiting' */
sem_wait(&sem_barbeiros);     /* vai dormir se o número de barbeiros livres for 0 */
cortandoCabelo();          /* sentado e sendo servido */
} else {
sem_post(&mutex);       /* a barbearia está cheia; não espera */
clienteDesistiu();

}

pthread_exit(NULL);
}

void cortarCabelo() {
printf("Barbeiro esta cortando o cabelo de alguem!\n");
sleep(3);
}

void clienteChegou() {
printf("Cliente chegou para cortar cabelo!\n");
}
void cortandoCabelo() {
printf("Cliente esta sendo atendido!\n");
}

void clienteDesistiu() {
printf("Cliente desistiu!\n");
}
