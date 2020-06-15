#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Interface.h"
#include "argus.h"

/**
 * @brief Inicia o progama argus num servidor em background
 * 
 * @return int Identidicação do proceso do servidor,se for um numero negativo indica que ocorreu um erro
 */
int main()
{
    int st = 0;
    if ((st = fork()) != 0)
        return st;

    mkfifo(InputFIFOName, 0666);
    st = open(InputFIFOName, O_RDONLY, 0666);
    dup2(st, 0);
    close(st);

    mkfifo(OutputFIFOName, 0666);
    st = open(OutputFIFOName, O_WRONLY, 0666);
    dup2(st, 1);
    close(st);

    st = argusRTE(0);

    unlink(InputFIFOName);
    unlink(OutputFIFOName);
    return st;
}