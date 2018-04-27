################################# Makefile #################################

all:	produtor

produtor:	ProdutorConsumidor.o mysem.o
	gcc -o prodcons ProdutorConsumidor.c mysem.o -l pthread -std=c99

ProdutorConsumidor.o:	ProdutorConsumidor.c
	gcc -o ProdutorConsumidor.o -c ProdutorConsumidor.c

mysem.o:	mysem.c
	gcc -o mysem.o -c mysem.c

clean:	
	rm -rf *.o

mrproper:
	rm -rf produtor