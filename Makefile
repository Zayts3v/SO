CC = gcc
GFLAGS = -Wall
OFLAGS = -c $(GFLAGS)


argus: argus.c Interface.o
	$(CC) $(GFLAGS) argus.c *.o -o argus

Interface.o: Interface.c Interface.h taskexec.o
	$(CC) $(OFLAGS) Interface.c Interface.h

#Not Permenant
taskexec.o: taskexec.c taskexec.h
	$(CC) $(OFLAGS) taskexec.c

	
clean:
	rm *.o argus