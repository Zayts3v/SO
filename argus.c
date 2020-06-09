#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Interface.h"

const unsigned int MaxLineSize = 1024;
const char name_ServerFIFO[12] = "ServerInput";

int terminal()
{
    /**
    char * comand;
    while ((comand = read(...) )> 0){ //TODO READLINE



        
        }
        
        
        */
    return 0;
}

int server_stop()
{
    int fd = open(name_ServerFIFO, O_WRONLY, 0777);
    write(fd, "quit\n", 6);
    close(fd);
    return 0;
}

int server()
{
    int st = 0;
    if ((st = fork()) != 0)
        return st;

    mkfifo(name_ServerFIFO, 0666);
    st = open(name_ServerFIFO, O_RDONLY, 0777);
    dup2(st, 0);
    close(st);
    st = open(name_ServerFIFO, O_WRONLY, 0777); //Impede que exista um EOF no pipe
    close(1);
    close(2);

    pause();
    terminal();

    close(st);
    unlink(name_ServerFIFO);
    return 0;
}

int shell(int argc, char **argv)
{
    //Validar formato da flag
    if (argc < 1 || argv[0][0] != '-' || argv[0][1] == '\0' || argv[0][2] != '\0')
    {
        write(2, "Comando Invalido\n", 18);
        return -2;
    }

    //Interpretar flag
    switch (argv[0][1])
    {
    case 'i':
        argv[0] = "tempo-inatividade";
        break;
    case 'm':
        argv[0] = "tempo-execucao";
        break;
    case 'e':
        argv[0] = "executar";
        break;
    case 'l':
        argv[0] = "listar";
        break;
    case 't':
        argv[0] = "terminar";
        break;
    case 'r':
        argv[0] = "tempo-inatividade";
        break;
    case 'h':
        argv[0] = "tempo-inatividade";
        break;
    case 'o':
        argv[0] = "tempo-inatividade";
        break;
    default:
        write(2, "Comando Invalido\n", 18);
        return -2;
        break;
    }

    char res[MaxLineSize];
    res[0] = '\0';
    int out, i, cursor;

    if ((out = open(name_ServerFIFO, O_WRONLY, 0666)) < 0)
    {
        write(2, "Error: Server Offline\n", 23);
        return -1;
    }
    //Concatenar argumentos
    for (i = 0, cursor = 0; i < argc && cursor > MaxLineSize; i++)
    {
        strncat(res, argv[i], MaxLineSize - cursor);
        cursor += strlen(argv[i]);
        strncat(res, " ", MaxLineSize - cursor);
        cursor++;
    }
    strncat(res, "\n", MaxLineSize - cursor);

    //Enviar operação para o servidor

    write(out, res, strlen(res));
    close(out);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        return terminal();
    if (strcmp("server", argv[1]) == 0)
    {
        if (argc > 2)
            if (strcmp(argv[2], "stop") == 0)
                return server_stop();
        return server();
    }

    return shell(argc - 1, argv + 1);
}