//
// Programa LEDS.C
//

// Pesquisar sobre:
//	- ioperm()
	/*
	ioperm()  sets  the  port access permission bits for the calling thread
       for num bits starting from port address from.  If turn_on  is  nonzero,
       then permission for the specified bits is enabled; otherwise it is dis‐
       abled.  If turn_on is nonzero, the calling thread  must  be  privileged
       (CAP_SYS_RAWIO).
	*/
//	- setuid()
	/*setuid()  sets  the  effective  user ID of the calling process.  If the
       calling process is privileged (more precisely: if the process  has  the
       CAP_SETUID  capability  in  its user namespace), the real UID and saved
       set-user-ID are also set.

       Under Linux, setuid() is implemented like the POSIX  version  with  the
       _POSIX_SAVED_IDS  feature.  This allows a set-user-ID (other than root)
       program to drop all of its user privileges, do some un-privileged work,
       and then reengage the original effective user ID in a secure manner.

	*/

//	- outb()
	/*
	 This family of functions is used to do low-level port input and output.
       The out* functions do port output, the in* functions do port input; the
       b-suffix functions are byte-width  and  the  w-suffix  functions  word-
       width; the _p-suffix functions pause until the I/O completes.

       They  are  primarily  designed for internal kernel use, but can be used
       from user space.
	*/

//	- inb()
	/* This family of functions is used to do low-level port input and output.
       The out* functions do port output, the in* functions do port input; the
       b-suffix functions are byte-width  and  the  w-suffix  functions  word-
       width; the _p-suffix functions pause until the I/O completes.

       They  are  primarily  designed for internal kernel use, but can be used
       from user space.
	*/
//	- controlador do teclado:
//		- http://www.brokenthorn.com/Resources/OSDev19.html

#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <errno.h>
#include <string.h>

#define R 1	// scroll lock
#define L 2	// numlock
#define M 4	// caps lock

#define PORTA_BASE  0x60
#define QTD_PORTAS    5
#define ON	1
#define OFF	0

#define VEZES	3

void setLeds( unsigned char );
void erro( char * );

int main( int argc, char **argv )
{
	unsigned char led, i;

	fprintf( stdout, "Testando os leds do teclado!\n\nOLHE!!!\n\n" );

	setuid( 0 ); 	//  acertar prioridade -- TALVEZ PRECISE ROOT

	if( ioperm( PORTA_BASE, QTD_PORTAS, ON ) )
		erro( "Falha no ioperm de acesso!" );
	for( i = 0; i < VEZES; ++i) {
		setLeds( L );
		sleep( 1 );
		setLeds( M );
		sleep( 1 );
		setLeds( R );
		sleep( 1 );
	}
	if( ioperm( PORTA_BASE, QTD_PORTAS, OFF ) )
		erro( "Falha no ioperm de fechamento!" );
	fprintf( stdout, "... Gostou?\n\n" );
	exit( 0 );
}

void setLeds( unsigned char led )
{
	outb( 0xED, PORTA_BASE );
	while( (inb( PORTA_BASE + 4 ) & 2)  ) //Faz máscara .E. com 2 para ver so bit está ligado
		;
	outb( led, PORTA_BASE );
	while( (inb( PORTA_BASE + 4 ) & 2)  )// Faz máscara .E. com 2 para ver se o bit está ligado
		;
	usleep( 1500 );
}

void erro( char *msg )
{
	fprintf( stderr, "\nERRO: %s (%d)\n%s\n\n", msg, errno, strerror( errno ) );
	exit( 1 );
}
