#ifndef Interface_h
#define Interface_h

typedef struct _argST *ArgusStatus;

ArgusStatus initArgusStatus();

int setMaximumRunTime(ArgusStatus sys, int RunTime);

int setMaximumIdleTime(ArgusStatus sys, int IdleTime);

int execute(ArgusStatus sys,char *command);

int listTasks(ArgusStatus sys);

int terminate(ArgusStatus sys,int task);

int history(ArgusStatus sys);

int output(ArgusStatus sys,int task);

char * help(ArgusStatus sys);
#endif 