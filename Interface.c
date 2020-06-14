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
#include "LogManager.h"
#include "unistd.h"

typedef struct _taskinfo
{
    char *command, *outputfilename;
    int pid, index, output, RTM, ITM;
} * TaskInfo;

TaskInfo mkTaskInfo(char *comand, int pid, int index, int RTM, int ITM)
{
    TaskInfo res = calloc(1, sizeof(struct _taskinfo));
    res->command = comand;
    res->pid = pid;
    res->index = index;
    res->ITM = ITM;
    res->RTM = RTM;

    char outname[20];
    snprintf(outname, 20, "task%d.out", index);
    res->outputfilename = strdup(outname);
    res->output = open(res->outputfilename, O_CREAT | O_TRUNC | O_RDWR, 0666);
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
    res->RunTimeMax = -1;
    res->IdleTimeMax = -1;
    res->logsIDX = openIDX(&(res->taskcount));
    res->logs = openLogs();
    return res;
}

void freeArgusStatus(ArgusStatus in)
{
    List_free(in->tasklist, (void (*)(void *))TaskInfo_free);
    free(in);
}

ArgusStatus msystem = NULL;

void TaskCleaner(int signum)
{
    List *l = &(msystem->tasklist);
    while (*l)
    {
        TaskInfo task = (*l)->data;
        int status;
        if (waitpid(task->pid, &status, WNOHANG) != 0)
        {
            int pos = updateLogs(msystem->logs, task->command, task->ITM, task->RTM, task->output);
            updateIDX(msystem->logsIDX, task->index, pos);
            TaskInfo_free(task);
            (*l) = List_freeBlock(*l);
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
    TaskInfo res = mkTaskInfo(command, -1, msystem->taskcount + 1, msystem->RunTimeMax, msystem->RunTimeMax);
    int pid;

    if ((pid = fork()) == 0)
    {
        updateIDX(msystem->logsIDX, -1, 0);
        dup2(res->output, 1);
        pid = task(res->command, res->RTM, res->ITM);
        _exit(pid);
    }
    if (pid < 0)
    {
        char *bruh_moment = "Erro a iniciar a tarefa\n";
        write(1, bruh_moment, strlen(bruh_moment));
        return -1;
    }

    res->pid = pid;
    msystem->taskcount++;
    msystem->tasklist = List_prepend(msystem->tasklist, res);
    char comand[32];
    snprintf(comand, 32, "Nova tarefa #%d \n", res->index);
    write(1, comand, strlen(comand));

    return 0;
}

int listTasks()
{
    char stringbuff[256];
    for (List l = msystem->tasklist; l; l = l->next)
    {
        TaskInfo info = l->data;
        snprintf(stringbuff, 256, "#%d: %s\n", info->index, info->command);
        write(1, stringbuff, strlen(stringbuff));
    }
    return 0;
}

int terminate(int task)
{
    List l = msystem->tasklist;
    while (l)
    {
        perror("IFWOEFE");
        TaskInfo info = l->data;
        if(info->index < task)break;
        if(info->index == task){
            kill(SIGTERM,info->pid);
            return 0;      
        }
    l = l->next;
    }
    
    char * out = "Tarefa Inexistente";
    write (1,out,strlen(out));
    return -1;
}

int history()
{
    char statusarray[msystem->taskcount];
    for (int i = 0; i < msystem->taskcount; i++)
        statusarray[i] = 0;

    for (List l = msystem->tasklist; l; l = l->next)
        statusarray[((TaskInfo)l)->index] = 0x1;

    char out[MaxLineSize];
    out[0] = '\0';
    for (int i = 0; i < msystem->taskcount; i++)
        if (!statusarray[i])
        {
            int ITM, RTM;
            snprintf(out, 32, "#%d: ", i);
            getCommandInfo(msystem->logs, readIndexIDX(msystem->logsIDX, i), out, MaxLineSize - strlen(out) - 64, &ITM, &RTM);
            if (RTM > 0)
                snprintf(out + strlen(out), 32, "Texec Maximo: %d ", RTM);
            if (ITM > 0)
                snprintf(out + strlen(out), 32, "Tinatividade Maximo: %d ", ITM);
            write(1, out, strlen(out));
        }

    return 0;
}

int output(int task)
{
    for (List l = msystem->tasklist; l && ((TaskInfo)l->data)->index > task; l = l->next)
        if (((TaskInfo)l->data)->index == task)
        {
            char *bruh_moment = "A tarefa ainda está em execução\n";
            write(1, bruh_moment, strlen(bruh_moment));
        }

    return !(writeOutputTo(msystem->logs, 1, readIndexIDX(msystem->logsIDX, task)) > 0);
}

char *help()
{
    return "\n\ttempo-inatividade <tempo em segundos>\n\ttempo-execucao <tempo em segundos>\n\texecutar <tarefa>\n\tlistar\n\tterminar <nº da tarefa>\n\thistorico\n\toutput <nº da tarefa>\n\n";
}

/**
 * @brief Runt Time Envirement do progama argus
 * 
 * @param displayname Indica se pretende-se mostrar a identificação do progama em cada comando
 * @return int 0 se acabou com sucesso, qualquer outro numero se ocorreu um erro
 */
int argusRTE(int displayname)
{
    signal(SIGCHLD, TaskCleaner);
    msystem = initArgusStatus();

    char buffer[ReadBufferSize];
    int rsize = -1;

    if (displayname)
        write(1, argusTag, strlen(argusTag));

    while ((rsize = readln(0, buffer, ReadBufferSize)) >= 0)
    {
        char *comand = strtok(buffer, " ");
        char *objects = strtok(NULL, "\0");

        if (comand != NULL)
        {
            switch (comand[0]) //TODO IMPRIMIR NO STDOUT
            {
            case 't':
                if (strcmp(comand, "tempo-execucao") == 0 && objects)
                {
                    setMaximumRunTime(atoi(objects));
                    break;
                }
                if (strcmp(comand, "tempo-inatividade") == 0 && objects)
                {
                    setMaximumRunTime(atoi(objects));
                    break;
                }
                if (strcmp(comand, "terminar") && objects)
                {
                    terminate(atoi(objects));
                    break;
                }
                break;
            case 'e':
                if (strcmp(comand, "executar") == 0)
                    execute(objects);
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
                {
                    char *res = help();
                    write(1, res, strlen(res));
                }
                break;
            case 'o':
                if (strcmp(comand, "output") == 0 && objects)
                    output(atoi(objects));
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

    freeArgusStatus(msystem);
    return rsize;
}
