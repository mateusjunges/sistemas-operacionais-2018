#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <csignal>

using namespace std;

void sig_handler( int signal ){
    if( signal == SIGINT )
        cout << "Recebi um SIGINT" << endl;
}

bool condicao = true;

int main(){

        while( condicao ){
            signal( SIGINT, sig_handler);
                    /*
                            A função signal especifica a
                            "handler routine" a ser executada
                            quando certo sinal ocorre.
                    */
            sleep(1);
        }
        return 0;


}
