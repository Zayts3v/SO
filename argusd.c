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
    int in, out, st;

    int ppRTin[2], ppRTout[2];

    if (pipe(ppRTin) < 0 || pipe(ppRTout) < 0)
        return -1;

    if ((st = fork()) == 0)
    {
        close(ppRTin[1]);
        dup2(ppRTin[0], 0);
        close(ppRTin[0]);

        close(ppRTout[0]);
        dup2(ppRTout[1], 1);
        close(ppRTout[1]);

        _exit(argusRTE(0));
    }
    close(ppRTin[0]);
    close(ppRTout[1]);
    int RTEinput = ppRTin[1], RTEoutput = ppRTout[0];

    while (1)
    {
        if (mkfifo(InputFIFOName, 0666) < 0)
            break;
        if (mkfifo(OutputFIFOName, 0666) < 0)
            break;
        in = open(InputFIFOName, O_RDONLY, 0666);
        out = open(OutputFIFOName, O_WRONLY, 0666);

        if ((st = fork()) == 0)
        {
            close(RTEoutput);
            close(out);
            char *buffer[ReadBufferSize];
            while ((st = read(in, buffer, ReadBufferSize) > 0))
            {
                write(RTEinput, buffer, st);
            }
            close(RTEinput);
            close(in);
            _exit(0);
        }
        if (st < 0)
            break;
        if ((st = fork()) == 0)
        {
            close(RTEinput);
            close(in);
            char *buffer[ReadBufferSize];
            while ((st = read(RTEoutput, buffer, ReadBufferSize) > 0))
            {
                write(out, buffer, st);
            }
            close(RTEoutput);
            close(out);
            _exit(0);
        }

        if (st < 0)
            break;
        close(in);
        close(out);
        printf("Imma hewe");
        wait(&st);
        kill(st, SIGKILL);
    }
    terminateServer(0);
    return -1;
}