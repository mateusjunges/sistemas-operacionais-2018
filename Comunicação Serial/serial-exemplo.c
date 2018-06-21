#include <stdio.h>
#include <stdlib.h>
#include "rs232.h" 

#ifdef __linux__                   // Verifica se tah no linux
#define limpar(x)  __fpurge(x)     // Se tiver, utiliza o __fpurge
#else                              // Se nao tiver no linux
#define limpar(x)  fflush(x)       // Utiliza fflush
#endif    

#define TAM 50                         //

char string[TAM];                   // Variavel q recebe a palavra a ser enviada

int main (){
    int op;
           
    OpenComport(0,9600);            // Abrir porta COM 0 com velocidade de 9600

    while(1){
        limpar(stdin);              // Limpar o buffer 
        printf("\n1 - Enviar string");
        printf("\n2 - Receber string");
        printf("\n3 - Sair\n\n");
        
        scanf("%d",&op);
        
        switch(op){
            case (1): limpar(stdin);
                      memset(string, 0, sizeof(string));  // limpa a string
                      printf("\n> ");
                      fgets(string,TAM,stdin);             // Pegar a string digitada e armazenar na variavel "string[30]"
                      cprintf(0,string);                  // Função da biblioteca - Enviar a string via serial
                      printf("Enviado com sucesso.");          
                      break;

            case (2): while(1){
                         limpar(stdin);
                         memset(string, 0, sizeof(string));
                         if(!PollComport(0, (unsigned char*)(string), 30)) // Poll Comport - Função da biblioteca q recebe a string
                            break;
                         printf("%s",string);
                      } 
                      break;

            case (3): CloseComport(0);         // Fechar a porta COM 0
                      return 0;
                      break;
        }
    }
}
