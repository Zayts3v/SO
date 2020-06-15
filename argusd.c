#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "Interface.h"
#include "argus.h"
#include <stdio.h>

/**
 * @brief Termina o Servidor de uma maneira controlada e limpa
 * 
 * @param signum sinal original
 */
void terminateServer(int signum)
{
    unlink(InputFIFOName);
    unlink(OutputFIFOName);
    kill(-getpgrp(), SIGKILL);
    _exit(-1);
}

/**
 * @brief Inicia o progama argus num servidor em background
 * 
 * @return int Identidicação do proceso do servidor,se for um numero negativo indica que ocorreu um erro
 */
int main()
{
    signal(SIGTERM, terminateServer);
    int in, out,stout=dup(1);

    argusRTE_init();

    while (1)
    {
        mkfifo(InputFIFOName, 0666);
        mkfifo(OutputFIFOName, 0666);
        in = open(InputFIFOName, O_RDONLY, 0666);
        out = open(OutputFIFOName, O_WRONLY, 0666);
        dup2(out, 1);
        close(out);
        char *comand, *objects;
        while (argusRTE_readcomand(in, &comand, &objects) > 0)
        {
            argusRTE_run(comand, objects);
            argusRTE_readcomand_free(comand, objects);
        }
        close(in);
        dup2(stout,1);
    }

    argusRTE_kill();
    terminateServer(0);
    return -1;
}