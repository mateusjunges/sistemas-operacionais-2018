CC	= gcc
STD	= -D_GNU_SOURCE
WARN	= -Wall

OBJS    = main.o cleanup.o attch.o semop.o game.o

.c.o:
	$(CC) $(CFLAGS) -c $(STD) $(WARN) $<

all:	bttlship

bttlship: $(OBJS)
	$(CC) $(OBJS) -o bttlship

clean:  
	rm -f *.o core 

clobber: clean
	rm -f bttlship
