#include <unistd.h>
#include <fcntl.h>
#include "Interface.h"

int main(int args, char **argv)
{
    if (args < 2 || argv[1][0] != '-' || argv[1][1] == '\0' || argv[1][2] != '\0')
        write(1, "Comando Invalido\n", 17);
    switch (argv[1][1])
    {
    case 'i':

        break;
    case 'm':

        break;
    case 'e':

        break;
    case 't':

        break;
    case 'r':

        break;
    case 'h':
        break;

    case 'o':

        break;
    default:
        write(1, "Comando Invalido\n", 17);
        break;
    }
    return 0;
}