#ifndef Interface_h
#define Interface_h

typedef struct _argST *ArgusStatus;

int setMaximumRunTime(ArgusStatus sys, int RunTime);

int setMaximumIdleTime(ArgusStatus sys, int IdleTime);

int execute(ArgusStatus sys,char *command);

int listTasks(ArgusStatus sys);

int terminate(ArgusStatus sys,int task);

int history(ArgusStatus sys);

int lookup(ArgusStatus sys,int task);
#endif 