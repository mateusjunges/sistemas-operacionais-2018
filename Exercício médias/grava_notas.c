//
//  Programa utilizado para criar o arquivo com notas para processamento.
//
//  gcc grava_notas.c -o grava_notas -D N_NOTAS=1000

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#ifndef N_NOTAS
   #define N_NOTAS  200
#endif

void main( void )
{
  int i, fd;

  fd = open( "notas.lst", O_WRONLY | O_CREAT | O_TRUNC, 0666 );
  
  if( fd < 0 ) {
    printf( "Erro na criacao do arquivo\n" );
    exit( fd );
  }
  
  for( i = 1; i <= N_NOTAS; ++i ) {
    float n1, n2, n3;

    n1 = rand()%100/10.0;
    n2 = rand()%100/10.0;
    n3 = rand()%100/10.0;
    if( n1 + n2 >= 14.0 )
      dprintf( fd, "%d %.1f %.1f %.1f\n", i, n1, n2, 0.0 );
    else
      dprintf( fd, "%d %.1f %.1f %.1f\n", i, n1, n2, n3 );
  }
  close( fd );
}

