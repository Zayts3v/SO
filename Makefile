CC = gcc
GFLAGS = -Wall
OFLAGS = -c $(GFLAGS)


argus: argus.c argus_Server argus_Terminal argus_Shell
	make argus_Server
	make argus_Terminal
	make argus_Shell
	$(CC) $(GFLAGS) argus.c -o argus

argus_Server: argus_Server.c Interface.o
	$(CC) $(GFLAGS) argus_Server.c Interface.o -o argus_Server

argus_Terminal: argus_Terminal.c Interface.o
	$(CC) $(GFLAGS) argus_Terminal.c Interface.o -o argus_Terminal

argus_Shell: argus_Shell.c Interface.o
	$(CC) $(GFLAGS) argus_Shell.c Interface.o -o argus_Shell

Interface.o: Interface.c Interface.h 
	$(CC) $(OFLAGS) Interface.c Interface.h 

#Not Permenant
taskexec.o: taskexec.c
	$(CC) $(OFLAGS) taskexec.c

	
clean:
	rm *.o argus argus_Server argus_Terminal argus_Shell