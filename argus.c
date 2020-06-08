#include <unistd.h>
#include <string.h>

int main(int args, char **argv)
{
    if (args == 1)
        execl("./argus_Terminal", "argus_Terminal", NULL);
    if (strcmp("server", argv[1]) == 0)
        execl("./argus_Server", "argus_Server", NULL);
    execv("./argus_Shell", argv);
    return -1;
}