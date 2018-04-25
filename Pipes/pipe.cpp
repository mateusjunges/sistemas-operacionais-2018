#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>

using namespace std;

int estado;
int pipe1[2];  /* Array Utilizado para criar o pipe1
                    * pipe1[0] => permite ler dados do pipe
                    * pipe1[1] => permite escrever dados no pipe
                    */
void criapipe(){
    pipe( pipe1 );
}



int main(){
    pid_t id_processo; /* Cria uma variável para armazenar o
                        ID quando der o fork*/

    char message[200] = "Mensagem do pai para o filho.";

    criapipe(); /* Chama a função que cria o pipe antes
                 de criar o processo filho, para que o processo
                 pai e o processo filho tenham os mesmos descritores
                 de arquivo, o que permite a comunicação entre os
                 processos
                 */

    id_processo = fork();

    if( id_processo < 0){ /* Se o PID for negativo, deu erro*/
        //cout << perror( id_processo );/* mostra a mensagem de erro */
    cout << "Erro" << endl;
    exit(-1);
    }
    else if( id_processo ){ /* Se deu tudo certo*/
        close(pipe1[0]);   /*  Somente o pai vai escrever,
                            * então fecha o pipe para leitura
                            */
        write(pipe1[1], message, (sizeof(message)+1)); /* Escreve a mensagem
                                                      * no pipe
                                                      */
        close(pipe1[1]);
        wait(&estado);

    }
    else{
        close(pipe1[1]); /* Como só o processo filho vai ler,
                         * fecha o pipe para escrita de dados
                         */
        read(pipe1[0], message, sizeof(message)+1); /* Lê a mensagem do pipe */
        cout << "\nO filho leu a string: " << message << endl;
        close(pipe1[0]); /* Fecha o pipe para leitura de dados */

    }
    return 0;
}
