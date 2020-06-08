#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Interface.h"

int main()
{
    mkfifo("ServerInput", 0777);
    int fd = open("ServerInput", O_RDONLY, 0777);

    //Client and Comand Interpreter

    close(fd);
    unlink("ServerInput");
}