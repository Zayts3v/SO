CC = gcc
GFLAGS = -Wall
OFLAGS = -c $(GFLAGS)


argus: argus.c taskexec.o
	$(CC) $(GFLAGS) argus.c *.o

taskexec.o: taskexec.c
	$(CC) $(OFLAGS) taskexec.c
	
clean:
	rm *.o argus 