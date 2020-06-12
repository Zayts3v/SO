CC = gcc
GFLAGS = -Wall
OFLAGS = -c $(GFLAGS)


argus: argus.c Interface.o
	$(CC) $(GFLAGS) argus.c *.o -o argus

Interface.o: Interface.c Interface.h taskexec.o list.o
	$(CC) $(OFLAGS) Interface.c Interface.h

taskexec.o: taskexec.c taskexec.h
	$(CC) $(OFLAGS) taskexec.c taskexec.h
	
list.o : list.h list.c
	$(CC) $(OFLAGS) list.c list.h
	
clean:
	rm *.o argus