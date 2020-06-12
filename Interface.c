#include "Interface.h"
#include "taskexec.h"
#include "list.h"
#include "unistd.h"
#include <stdlib.h>
#include <string.h>

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
