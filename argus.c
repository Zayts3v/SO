#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Interface.h"

const unsigned int MaxLineSize = 1024;
const unsigned int ReadBufferSize = 4096;
const char InputFIFOName[12] = "ArgusInput";
const char OutputFIFOName[12] = "ArgusOutput";
const char argusTag[8] = "argus$ ";

/**
 * @brief Le no maximo nbytes de um dado ficheiro imprime a primeira linha do mesmo num dado buffer
 * 
 * @param fd File descriptor a ser lido
 * @param buffer Buffer a ser escrito
 * @param nbytes Nº de bytes limite que se podem ler
 * @return int Nº de bytes lidos
 */
int readln(int fd, char *buffer, size_t nbytes)
{
    int res = read(fd, buffer, nbytes);
    if (res == 0)
        return -1;
    for (int i = 0; i < res; i++)
        if (buffer[i] == '\n')
        {
            buffer[i] = '\0';
            break;
        }
    return res;
}
/**
 * @brief Intrepeta comandos do STDIN e escreve os resultados dos mesmos no STDOUT
 * 
 * @param displayname Indica se pretende-se mostrar a identificação do progama em cada comando
 * @return int 0 se acabou com sucesso, qualquer outro numero se ocorreu um erro
 */
int terminal(int displayname)
{
    ArgusStatus sys = initArgusStatus();

    char buffer[ReadBufferSize];
    int rsize = -1;

    if (displayname)
        write(1, argusTag, strlen(argusTag));

    while ((rsize = readln(0, buffer, ReadBufferSize)) >= 0)
    {
        char *comand = strtok(buffer, " ");

        //O objetivo deste switch é tornar o codigo mais rapido
        if (comand != NULL)
        {
            switch (comand[0]) //TODO IMPRIMIR NO STDOUT
            {
            case 't':
                if (strcmp(comand, "tempo-execucao") == 0)
                {
                    setMaximumRunTime(sys, atoi(buffer+strlen(comand)));
                }
                if (strcmp(comand, "tempo-inatividade") == 0)
                {
                    setMaximumRunTime(sys, atoi(buffer+strlen(comand)));
                }
                if (strcmp(comand, "terminar"))
                {
                    terminate(sys, atoi(buffer+strlen(comand)));
                }
                break;
            case 'e':
                if (strcmp(comand, "executar") == 0)
                    execute(sys, buffer+strlen(comand));
                break;
            case 'l':
                if (strcmp(comand, "listar") == 0)
                    listTasks(sys);
                break;
            case 'h':
                if (strcmp(comand, "historico") == 0)
                    history(sys);
                break;
            case 'a':
                if (strcmp(comand, "ajuda") == 0)
                    help(sys);
                break;
            case 'o':
                if (strcmp(comand, "output") == 0)
                    output(sys, atoi(buffer+strlen(comand)));
                break;
            case 'q':
                if (strcmp(comand, "quit") == 0)
                    return 0;
            default:
                write(1,"Comando Invalido\n",18);
            break;
            }
        }
        if (displayname)
            write(1, argusTag, strlen(argusTag));
    }

    return rsize;
}

int server_stop()
{
    int fd = open(InputFIFOName, O_WRONLY, 0666);
    if(fd>0){
    write(fd, "quit\n", 6);
    close(fd);
    }
    return 0;
}

/**
 * @brief Inicia o progama argus num servidor em background
 * 
 * @return int Identidicação do proceso do servidor,se for um numero negativo indica que ocorreu um erro
 */
int server()
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

    st = terminal(0);

    unlink(InputFIFOName);
    unlink(OutputFIFOName);
    return st;
}

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

    char res[MaxLineSize];
    res[0] = '\0';
    int in,out, i, cursor;

    if ((out = open(InputFIFOName, O_WRONLY, 0666)) < 0)
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

    //Ler operação do servidor


    return 0;
}

/**
 * @brief Função de partida do progama 
 */
int main(int argc, char *argv[])
{
    if (argc == 1)
        return terminal(1);
    if (strcmp("server", argv[1]) == 0)
    {
        if (argc > 2)
            if (strcmp(argv[2], "stop") == 0)
                return server_stop();
        return server();
    }

    return shell(argc - 1, argv + 1);
}