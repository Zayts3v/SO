#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include "constants.h"
#include "Interface.h"
#include "taskexec.h"
#include "list.h"
#include "LogManager.h"
#include "unistd.h"

/**
 * @brief Define uma estrutura de informação de tarefa
 * 
 */
typedef struct _taskinfo
{
    char *command, *outputfilename;
    pid_t pid;
    int  index, output, RTM, ITM;
} * TaskInfo;

/**
 * @brief Cria uma estrutura de informação de uma tarefa, e o seu ficheiro temporio de output
 * 
 * @param comand Comando da tarefa
 * @param pid Process Identification da tarefa
 * @param index Indice da tarefa
 * @param RTM Tempo maximo de execução da tarefa, (-1 se nao for aplicavel)
 * @param ITM Tempo maximo de inatividade de cada operação da tarefa, (-1 se nao for aplicavel)
 * @return TaskInfo Estrutura de informação da tarefa
 */
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

/**
 * @brief Liberta uma estrutura de informação de uma tarefa de memoria/disco
 * 
 * @param info Eestrutura de informação a ser libertada
 */
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

/**
 * @brief Estrutura do ArgusStatus
 * 
 */
typedef struct _argST
{
    List tasklist;
    int taskcount;
    int RunTimeMax, IdleTimeMax;
    int logs, logsIDX;
} * ArgusStatus;

/**
 * @brief Inicializa um ArgusStatus
 * 
 * @return ArgusStatus Novo argusEstado vazio em termos de informação
 */
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

/**
 * @brief Liberta a estrutura ArgusStatus de memoria
 * 
 * @param in estrutura a ser libertada
 */
void freeArgusStatus(ArgusStatus in)
{
    List_free(in->tasklist, (void (*)(void *))TaskInfo_free);
    free(in);
}

/**
 * @brief Estado Principal do argus
 */
ArgusStatus msystem = NULL;

/**
 * @brief Limpa as tarefas que invalidas ou quje ja terminaram do registo do progama
 * 
 * @param signum Sinal que provocou a chamada desta função
 */
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

/**
 * @brief Define o tempo maximo de execuçao das proximas tarefas a serem executadas
 * 
 * @param RunTime Tempo Maximo de execução, se este for um numero negativo este timeout é desligado
 */
void setMaximumRunTime(int RunTime)
{
    msystem->RunTimeMax = RunTime;
}
/**
 * @brief Define o tempo maximo de inatividade das operações das proximas tarefas a serem executadas
 * 
 * @param IdleTime Tempo Maximo de execução, se este for um numero negativo este timeout é desligado
 */
void setMaximumIdleTime(int IdleTime)
{
    msystem->IdleTimeMax = IdleTime;
}

/**
 * @brief Executa uma nova tarefa
 * 
 * @param command Comando da tarefa a ser executado
 * @return int zero se a tarefa foi cria, -1 caso tenha ocorrido um erro
 */
int execute(char *command)
{
    TaskInfo res = mkTaskInfo(command, -1, msystem->taskcount + 1, msystem->RunTimeMax, msystem->RunTimeMax);
    pid_t pid;

    if ((pid = fork()) == 0)
    {
        //Cria uma entrada no IDX para a tarefa
        updateIDX(msystem->logsIDX, res->index, 0);
        //Separa a tarefa e a sua descendencia do grupo do progama principal
        //caso isto nao seja feito os timeouts irao afeter o argus em si
        pid = getpid();
        setpgid(pid, pid);
        //Devia o output da tarefa para o ficheiro temporario de output
        dup2(res->output, 1);
        //Executa a tarefa
        _exit(task(res->command, res->RTM, res->ITM));
    }
    //Se o fork falhar
    if (pid < 0)
    {
        char *bruh_moment = "Erro a iniciar a tarefa\n";
        write(1, bruh_moment, strlen(bruh_moment));
        TaskInfo_free(res);
        return -1;
    }

    res->pid = pid;
    msystem->taskcount++;
    msystem->tasklist = List_prepend(msystem->tasklist, res);
    char notefy[32];
    snprintf(notefy, 32, "Nova tarefa #%d \n", res->index);
    write(1, notefy, strlen(notefy));

    return 0;
}

/**
 * @brief Lista as tarefas em execução para o STDOUT
 */
void listTasks()
{
    char stringbuff[256];
    for (List l = msystem->tasklist; l; l = l->next)
    {
        TaskInfo info = l->data;
        snprintf(stringbuff, 256, "#%d: %s\n", info->index, info->command);
        write(1, stringbuff, strlen(stringbuff));
    }
}

/**
 * @brief Termina uma tarefa em execução
 * 
 * @param task Nº da tarefa a ser terminada
 * @return int 0 se terminou com sucesso, -1 caso tenha ocorrido um erro
 */
int terminate(int task)
{
    List l = msystem->tasklist;
    while (l)
    {
        TaskInfo info = l->data;
        if (info->index < task)
            break;
        if (info->index == task)
        {
            kill(-getpgid(info->pid), SIGKILL);
            return 0;
        }
        l = l->next;
    }

    char *out = "Tarefa Inexistente\n";
    write(1, out, strlen(out));
    return -1;
}

/**
 * @brief Imprime o historico do progama do STDOUT
 */
void history()
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
}

/**
 * @brief Imprime o output de uma dada tarefa no STDOUT
 * 
 * @param task tarefa a ser imprimida
 * @return int 0 se tiver sucesso, -1 se ocorrer um erro, -2 se a tarefa ainda estiver em execuçao
 */
int output(int task)
{
    for (List l = msystem->tasklist; l && ((TaskInfo)l->data)->index > task; l = l->next)
        if (((TaskInfo)l->data)->index == task)
        {
            char *bruh_moment = "A tarefa ainda está em execução\n";
            write(1, bruh_moment, strlen(bruh_moment));
            return -2;
        }

    return -!(writeOutputTo(msystem->logs, 1, readIndexIDX(msystem->logsIDX, task)) > 0);
}

/**
 * @brief Devolve uma string com informaçoes de como usar o argus
 * 
 * @return char* String com informaçoes de como usar o argus
 */
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
                if (strcmp(comand, "terminar") == 0 && objects)
                {
                    terminate(atoi(objects));
                    break;
                }
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
                break;
            case 'e':
                if (strcmp(comand, "executar") == 0 && objects)
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
