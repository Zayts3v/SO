#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Interface.h"
#include "argus.h"

/**
 * @brief Lê, processa e comunica ao servidor um comando, proveniente de uma shell, a ser executado
 * 
 * @param argc Nº de parametros do comando
 * @param argv Comando parametrizado
 * @return int 0 se conclui com sucesso, caso contrario devolve um outro numero
 */
int shell(int argc, char **argv)
{
    //Validar formato da flag
    if (argc < 1 || argv[0][0] != '-' || argv[0][1] == '\0' || (argv[0][2] != '\0' && argv[0][1] != '-'))
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
        argv[0] = "historico";
        break;
    case 'h':
        argv[0] = "ajuda";
        break;
    case 'o':
        argv[0] = "output";
        break;
    case '-':
    {
        if (strcmp(argv[0], "--serverstop") == 0)
        {
            execlp("pkill", "pkill", "argusd", NULL);
            return -1;
        }
        if (strcmp(argv[0], "--help") == 0)
        {
            char *out = "--help Flag List\n--serverstop Stop argusd Server \n-i Max Idle\n-m Max RunTime\n-e execute\n-l list tasks \n-t kill task\n-r history\n-h argus help\n-o task output\n";
            write(1, out, strlen(out));
            return 0;
        }
    }
    default:
        write(2, "Comando Invalido\n", 18);
        return -2;
        break;
    }

    int i,serverin, serverout; 

    serverin = open(InputFIFOName, O_WRONLY, 0666);
    serverout = open(OutputFIFOName, O_RDONLY, 0666);
    if (serverin<0 || serverout<0)
    {
        write(2, "Error: Server Offline\n", 23);
        return -1;
    }

    //Concatenar argumentos
    char res[MaxLineSize];
    res[0] = '\0';
    for (i = 0; i < argc ; i++)
    {
        int bruh = MaxLineSize - strlen(res);
        strncat(res, argv[i], bruh);
        strncat(res, " ", bruh);
    }
    strncat(res, "\n",MaxLineSize - strlen(res));

    //Enviar operação para o servidor

    write(serverin, res, strlen(res));

    //Ler operação do servidor
    
    alarm(10);
    char buffer[ReadBufferSize];
    if ((i = read(serverout, buffer, ReadBufferSize)) > 0)
    {
        write(1, buffer, i);
    }
    close(serverout);
    close(serverin);

    return 0;
}

/**
 * @brief Função de partida do progama 
 */
int main(int argc, char *argv[])
{
    if (argc == 1)
        return argusRTE();
    return shell(argc - 1, argv + 1);
}