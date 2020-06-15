CC = gcc
GFLAGS = -Wall
OFLAGS = -c $(GFLAGS)

all:
	make argus
	make argusd

argus: argus.c Interface.o constants.o
	$(CC) $(GFLAGS) argus.c *.o -o argus 

argusd: argusd.c Interface.o 
	$(CC) $(GFLAGS) argusd.c *.o -o argusd

Interface.o: Interface.c Interface.h taskexec.o list.o LogManager.o 
	$(CC) $(OFLAGS) Interface.c Interface.h 

taskexec.o: taskexec.c taskexec.h 
	$(CC) $(OFLAGS) taskexec.c taskexec.h
	
list.o : list.h list.c
	$(CC) $(OFLAGS) list.c list.h
	
LogManager.o: LogManager.c LogManager.h 
	$(CC) $(OFLAGS) LogManager.c LogManager.h

constants.o : constants.c argus.h
	$(CC) $(OFLAGS) constants.c argus.h

clean:
	-killall -SIGKILL -g argus
	-killall -SIGKILL -g argusd
	-rm *.o argus argusd *.gch ArgusInput ArgusOutput logs logs.idx *.out