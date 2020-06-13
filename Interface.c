#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "Interface.h"
#include "taskexec.h"
#include "list.h"
#include "unistd.h"

typedef struct _taskinfo{
    char * command;
    int id,index,status;
} * TaskInfo;

typedef enum TaskStatus {Done,Running};

TaskInfo mkTaskInfo(char *comand, int pid, int index, int status)
{
    TaskInfo res = calloc(1, sizeof(struct _taskinfo));
    res->command = comand;
    res->id = pid;
    res->index = index;
    res->status = status;
    return res;
}

void TaskInfo_free(TaskInfo info)
{
    if (info)
        free(info);
}


struct _argST
{   int taskcount;
    int RunTimeMax, IdleTimeMax;
    int logs,logsIDX;
};

ArgusStatus initArgusStatus(){
    ArgusStatus res = calloc(1,sizeof(struct _argST));
    res->RunTimeMax = -1;
    res->IdleTimeMax = -1;
    //res -> logs =  
    //
}


int setMaximumRunTime(ArgusStatus sys, int RunTime)
{
    if (RunTime < 1)
        return -1;
    sys->RunTimeMax = RunTime;
    return 0;
}

int setMaximumIdleTime(ArgusStatus sys, int IdleTime)
{
    if (IdleTime < 1)
        return -1;
    sys->IdleTimeMax = IdleTime;
    return 0;
}

int execute(ArgusStatus sys, char *command)
{
    int ncomands = 1;
    for (int i = 0; command[i]; i++)
        if (command[i] == '|')
            ncomands++;

    char *comands[ncomands], *t0 = strtok(command, "|");
    for (int i = 0; t0; i++, t0 = strtok(NULL, "|"))
    {
        comands[i] = t0;
    }

    if (fork()==0) {task(ncomands, comands, sys->RunTimeMax, sys->IdleTimeMax);}
    return 0;
}

int listTasks(ArgusStatus sys)
{
}

int terminate(ArgusStatus sys, int task)
{
}

int history(ArgusStatus sys)
{
}

int output(ArgusStatus sys, int task)
{
}

char * help(ArgusStatus sys){

}

/**
 * @brief Le no maximo nbytes de um dado ficheiro imprime a primeira linha do mesmo num dado buffer
 * 
 * @param fd File descriptor a ser lido
 * @param buffer Buffer a ser escrito
 * @param nbytes Nº de bytes limite que se podem ler
 * @return int Nº de bytes lidos
 */
int readln(int fd, char *buffer, unsigned int nbytes)
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
int argus(int displayname)
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
