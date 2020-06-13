CC = gcc
GFLAGS = -Wall
OFLAGS = -c $(GFLAGS)

all:
	make argus
	make argusd

argus: argus.c Interface.o constants.o
	$(CC) $(GFLAGS) argus.c *.o -o argus 

argusd: argusd.c Interface.o constants.o
	$(CC) $(GFLAGS) argusd.c *.o -o argusd

Interface.o: Interface.c Interface.h taskexec.o list.o constants.o
	$(CC) $(OFLAGS) Interface.c Interface.h 

taskexec.o: taskexec.c taskexec.h constants.o
	$(CC) $(OFLAGS) taskexec.c taskexec.h
	
list.o : list.h list.c
	$(CC) $(OFLAGS) list.c list.h
	
constants.o : constants.c constants.h
	$(CC) $(OFLAGS) constants.c constants.h

clean:
	rm *.o argus argusd *.gch ArgusInput ArgusOutput logs logs.idx