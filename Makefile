CC = gcc
GFLAGS = -Wall
OFLAGS = -c $(GFLAGS)

all:
	make argus
	make argusd

argus: argus.c Interface.o constants.h constants.h
	$(CC) $(GFLAGS) constants.h argus.c *.o -o argus 

argusd : argusd.c Interface.o constants.h
		$(CC) $(GFLAGS) constants.h argusd.c *.o -o argusd

Interface.o: Interface.c Interface.h taskexec.o list.o constants.h
	$(CC) $(OFLAGS) Interface.c Interface.h constants.h

taskexec.o: taskexec.c taskexec.h
	$(CC) $(OFLAGS) taskexec.c taskexec.h
	
list.o : list.h list.c
	$(CC) $(OFLAGS) list.c list.h
	
clean:
	rm *.o argus *.gch ArgusInput ArgusOutput logs logs.idx