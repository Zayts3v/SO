#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Interface.h"
#include "constants.h"

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
        argv[0] = "historico";
        break;
    case 'h':
        argv[0] = "ajuda";
        break;
    case 'o':
        argv[0] = "output";
        break;
    default:
        write(2, "Comando Invalido\n", 18);
        return -2;
        break;
    }

    int serverin,serverout, cursor=0;

    if ((serverin = open(InputFIFOName, O_WRONLY, 0666)) < 0 || (serverout = open(OutputFIFOName,O_RDONLY,0666)<0))
    {
        write(2, "Error: Server Offline\n", 23);
        return -1;
    }
    //Concatenar argumentos
    char res[MaxLineSize];
    res[0] = '\0';
    for (int i = 0, cursor = 0; i < argc && cursor > MaxLineSize; i++)
    {
        strncat(res, argv[i], MaxLineSize - cursor);
        cursor += strlen(argv[i]);
        strncat(res, " ", MaxLineSize - cursor);
        cursor++;
    }
    strncat(res, "\n", MaxLineSize - cursor);

    //Enviar operação para o servidor

    write(serverin, res, strlen(res));
    close(serverin);

    //Ler operação do servidor
    char buffer[ReadBufferSize];
    while ((cursor = read(serverout,buffer,ReadBufferSize))>0){
        write(1,buffer,cursor);
    }
    close(serverout);

    return 0;
}

/**
 * @brief Função de partida do progama 
 */
int main(int argc, char *argv[])
{
    if (argc == 1) return argusRTE(1);
    return shell(argc - 1, argv + 1);
}