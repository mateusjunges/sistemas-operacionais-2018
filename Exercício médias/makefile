all: Sequencial

#ifndef NUM_NOTAS  
	#define NUM_NOTAS 200
#endif


Sequencial:	media_sequencial.cpp	grava_notas
	g++ media_sequencial.cpp -o med_seq

grava_notas:	grava_notas.c
	gcc grava_notas.c -o grava_notas -D N_NOTAS=NUM_NOTAS
