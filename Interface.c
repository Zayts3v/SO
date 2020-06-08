#include "Interface.h"

//EITHER (this) (STRUCT)
int RunTimeMax = -1;
int IdleTimeMax = -1;

//-->Estrutura com as Tarefas e detalhes das mesmas (probably listas com varios parametros)

int setMaximumRunTime(int RunTime)
{
    if (RunTime < 1)
        return -1;
    RunTime = RunTime;
    return 0;
}

int setMaximumIdleTime(int IdleTime)
{
    if (IdleTimeMax < 1)
        return -1;
    IdleTimeMax = IdleTimeMax;
    return 0;
}

int execute(char *command)
{
    //TODO implementar ou migrar taskexec.c
}

int listTasks()
{
    //TODO
}

int terminate()
{
    //TODO
}

int history()
{
    //TODO
}

int lookup(int task)
{
    //TODO
}