#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "constants.h"
#include "Interface.h"
#include "taskexec.h"
#include "list.h"
#include "unistd.h"

typedef struct _taskinfo
{
    char *command, *outputfilename;
    int pid, index, output;
} * TaskInfo;

TaskInfo mkTaskInfo(char *comand, int pid, int index)
{
    TaskInfo res = calloc(1, sizeof(struct _taskinfo));
    res->command = comand;
    res->pid = pid;
    res->index = index;

    char outname[20];
    snprintf(outname, 20, "task%d.out", index);
    res->outputfilename = strdup(outname);
    res->output = open(res->outputfilename, O_CREAT | O_TRUNC | O_RDWR);
    if (res->output < 0)
        res->output = 1;

    return res;
}

void TaskInfo_free(TaskInfo info)
{
    if (info)
    {
        if (info->outputfilename)
        {
            unlink(info->outputfilename);
            free(info->outputfilename);
        }
        free(info);
    }
}

typedef struct _argST
{
    List tasklist;
    int taskcount;
    int RunTimeMax, IdleTimeMax;
    int logs, logsIDX;
} * ArgusStatus;

ArgusStatus initArgusStatus()
{
    ArgusStatus res = calloc(1, sizeof(struct _argST));
    res->tasklist = NULL;
    res->taskcount = 0;
    res->RunTimeMax = -1;
    res->IdleTimeMax = -1;
    //res -> logs = API NEEDED
    //res -> logsIDX = API NEEDED
}

ArgusStatus msystem = NULL;

void taskMaid(int signum)
{
    for (List *l = &(msystem->tasklist); *l;)
    {
        TaskInfo task = (*l)->data;
        int status;
        if (task->pid && waitpid(task->pid, &status, WNOHANG) == task->pid)
        {
            //MOVE OUTPUT TO LOG -->API NEEDED
            //UPDATE IDX WITH LOG POSITION --> API NEEDED
            *l = (*l)->next;
            TaskInfo_free(task);
        }
        else
            l = &(*l)->next;
    }
}

int setMaximumRunTime(int RunTime)
{
    if (RunTime < 1)
        return -1;
    msystem->RunTimeMax = RunTime;
    return 0;
}

int setMaximumIdleTime(int IdleTime)
{
    if (IdleTime < 1)
        return -1;
    msystem->IdleTimeMax = IdleTime;
    return 0;
}

int execute(char *command)
{
    TaskInfo res = mkTaskInfo(command, -1, msystem->taskcount + 1);
    int pid;

    if ((pid = fork()) == 0)
    {
        //REGISTER TASK ENTRY IN IDX -> API NEEDDED
        dup2(res->output, 1);
        pid = task(res->command, msystem->RunTimeMax, msystem->IdleTimeMax);
        _exit(pid);
    }
    else
    {
        char *bruh_moment = "Erro a iniciar a tarefa";
        write(1, bruh_moment, sizeof(bruh_moment));
        return -1;
    }

    res->pid = pid;
    msystem->taskcount++;
    msystem->tasklist = List_prepend(msystem->tasklist, res);
    char outputstring[32];
    snprintf(outputstring, 32, "Nova tarefa #%d \n");

    return 0;
}

int listTasks() //MIGHT BE AFFECT WITH HISTORICO PROBLEM ,_,
{
    char stringbuff[256];
    for (List l = msystem->taskcount; l; l = l->next)
    {
        TaskInfo info = l->data;
        snprintf(stringbuff, 256, "#%d: %s\n", info->index, info->command);
        write(1, stringbuff, strlen(stringbuff));
    }
}

int terminate(int task)
{
    for (List l = msystem->taskcount; l; l = l->next)
    {
        TaskInfo info = l->data;
        if (info->index <= task)
        {
            if (info->index == task)
                kill(SIGINT, info->pid);
            break;
        }
    }
}

int history()
{
    char boolarray[msystem->taskcount];
    for (int i = 0; i < msystem->taskcount; i++)
        boolarray[i] = 0;

    for (List l = msystem->tasklist; l ; l=l->next)
        boolarray[ ((TaskInfo) l)->index] = 1;

    for (int i = 0; i < msystem->taskcount; i++)
        if(boolarray[i]){
            //UNEXPECTED PROBLEMS IN HERE - NOT STORING ALL INFO REQUIRED TO CHECK LATER
        }
}

int output(int task)
{
    //get idx line
    //if (output (1,idxpos)<0) write(1,"\n",1);
}

char *help()
{
    return "bruh_1\nbruh2\nbruh3\nbruh4\n";
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
    signal(SIGCHLD, taskMaid);
    msystem = initArgusStatus();

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
                    setMaximumRunTime(atoi(buffer + strlen(comand)));
                }
                if (strcmp(comand, "tempo-inatividade") == 0)
                {
                    setMaximumRunTime(atoi(buffer + strlen(comand)));
                }
                if (strcmp(comand, "terminar"))
                {
                    terminate(atoi(buffer + strlen(comand)));
                }
                break;
            case 'e':
                if (strcmp(comand, "executar") == 0)
                    execute(buffer + strlen(comand));
                break;
            case 'l':
                if (strcmp(comand, "listar") == 0)
                    listTasks();
                break;
            case 'h':
                if (strcmp(comand, "historico") == 0)
                    history();
                break;
            case 'a':
                if (strcmp(comand, "ajuda") == 0)
                    help();
                break;
            case 'o':
                if (strcmp(comand, "output") == 0)
                    output(atoi(buffer + strlen(comand)));
                break;
            case 'q':
                if (strcmp(comand, "quit") == 0)
                    return 0;
            default:
                write(1, "Comando Invalido\n", 18);
                break;
            }
        }
        if (displayname)
            write(1, argusTag, strlen(argusTag));
    }

    return rsize;
}
