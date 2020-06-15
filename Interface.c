#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include "argus.h"
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
    int index, output, RTM, ITM;
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
        if (info->command)
            free(info->command);
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
 * @brief Força a terminação de todas as tarefas do argus
 * 
 * @param signum Sinal que provocou a chamada desta função
 */
void argusKillAllTasks(int signum)
{
    if (!msystem)
        return;
    for (List l = msystem->tasklist; l; l = l->next)
    {
        kill(-getpgid(((TaskInfo)l->data)->pid), SIGKILL);
    }
    if (signum != 0)
    {
        char *done = "\nTodas as tarefas foram terminadas\n";
        write(1, done, strlen(done));
    }
}

/**
 * @brief Força a terminação de todas as tarefas do argus e do argus em si
 * 
 * @param signum 
 */
void argusINT(int signum)
{
    argusKillAllTasks(0);
    _exit(-1);
}

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
            (*l) = List_tail(*l);
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
    char *out = (RunTime < 0 ? "Temporizador desativado\n" : "Temporizador defenido\n");
    write(1, out, strlen(out));
}
/**
 * @brief Define o tempo maximo de inatividade das operações das proximas tarefas a serem executadas
 * 
 * @param IdleTime Tempo Maximo de execução, se este for um numero negativo este timeout é desligado
 */
void setMaximumIdleTime(int IdleTime)
{
    msystem->IdleTimeMax = IdleTime;
    char *out = (IdleTime < 0 ? "Temporizador desativado\n" : "Temporizador defenido\n");
    write(1, out, strlen(out));
}

/**
 * @brief Executa uma nova tarefa
 * 
 * @param command Comando da tarefa a ser executado
 * @return int zero se a tarefa foi cria, -1 caso tenha ocorrido um erro
 */
int execute(char *command)
{
    TaskInfo res = mkTaskInfo(strdup(command), -1, msystem->taskcount + 1, msystem->RunTimeMax, msystem->IdleTimeMax);
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
        char *bruh_moment = "Erro ao iniciar a tarefa\n";
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
    for (List l = msystem->tasklist; l; l = l->next)
    {
        TaskInfo info = l->data;
        char stringbuff[MaxLineSize];
        snprintf(stringbuff, MaxLineSize, "#%d: %s\n", info->index, info->command);
        write(1, stringbuff, strlen(stringbuff));
    }
    if (msystem->tasklist == NULL)
    {
        char *out = "Nenhuma tarefa em execução\n";
        write(1, out, strlen(out));
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
    char *out;

    List l = msystem->tasklist;
    while (l)
    {
        TaskInfo info = l->data;
        if (info->index < task)
            break;
        if (info->index == task)
        {
            kill(-getpgid(info->pid), SIGKILL);
            out = "Sucesso\n";
            write(1, out, strlen(out));
            return 0;
        }
        l = l->next;
    }

    out = "Tarefa inexistente\n";
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

    int count = 0;
    for (int i = 0; i < msystem->taskcount; i++)
        if (!statusarray[i])
        {
            count++;
            int ITM, RTM;
            char out[MaxLineSize];
            getCommandInfo(msystem->logs, readIndexIDX(msystem->logsIDX, i), out, MaxLineSize, &ITM, &RTM);
            char S_ITM[48] = {0};
            if (ITM > 0)
                snprintf(S_ITM, 48, "Tinatividade Maximo: %d ", ITM);
            char S_RTM[48] = {0};
            if (RTM > 0)
                snprintf(S_RTM, 48, "Texecucao Maximo: %d ", RTM);
            printf("#%d: %s%s%s\n", i + 1, S_RTM, S_ITM, out);
        }
    if (count == 0)
    {
        char *out = "Nenhuma tarefa em historico\n";
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

    return writeOutputTo(msystem->logs, 1, readIndexIDX(msystem->logsIDX, task));
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
 * @brief 
 * 
 */
void argusRTE_init()
{
    signal(SIGCHLD, TaskCleaner);
    signal(SIGQUIT, argusKillAllTasks);
    signal(SIGINT, argusINT);
    msystem = initArgusStatus();
}

void argusRTE_kill()
{
    argusKillAllTasks(0);
    freeArgusStatus(msystem);
    _exit(0);
}

int argusRTE_readcomand(int filetoread, char **comand, char **objects)
{
    char buffer[ReadBufferSize];
    buffer[ReadBufferSize - 1] = 0;
    int res = readlncc(filetoread, buffer, ReadBufferSize);
    if (res < 1)
        return res;
    *comand = strtok(buffer, " ");
    if (*comand)
    {
        *comand = strdup(*comand);
        *objects = strtok(NULL, "\0");
        if (*objects != NULL)
            *objects = strdup(*objects);
    }

    return res;
}

void argusRTE_readcomand_free(char *comand, char *objects)
{
    if (comand != NULL)
        free(comand);
    if (objects != NULL)
        free(objects);
}

/**
 * @brief Executa um comando
 * 
 * @param comand comando ser executado
 * @param objects objetos do comando (null se nao for aplicavel)
 * @return int 
 */
int argusRTE_run(char *comand, char *objects)
{
    if (comand != NULL)
    {
        if (comand[0] == 'a' && strcmp(comand, "ajuda") == 0)
        {
            char *res = help();
            write(1, res, strlen(res));
        }
        else if (comand[0] == 'e' && strcmp(comand, "executar") == 0 && objects)
            return execute(objects);
        else if (comand[0] == 'h' && strcmp(comand, "historico") == 0)
            history();
        else if (comand[0] == 'l' && strcmp(comand, "listar") == 0)
            listTasks();
        else if (comand[0] == 'o' && strcmp(comand, "output") == 0 && objects)
            return output(atoi(objects));
        else if (comand[0] == 's' && strcmp(comand, "sair") == 0)
            argusRTE_kill();
        else if (comand[0] == 't' && strcmp(comand, "terminar") == 0 && objects)
            return terminate(atoi(objects));
        else if (comand[0] == 't' && strcmp(comand, "tempo-execucao") == 0 && objects)
            setMaximumRunTime(atoi(objects));
        else if (comand[0] == 't' && strcmp(comand, "tempo-inatividade") == 0 && objects)
            setMaximumIdleTime(atoi(objects));
        else
        {
            char *out = "Parametro ou comando Invalido\n";
            write(1, out, strlen(out));
        }
    }
    else
        return -1;
    return 0;
}

/**
 * @brief Runt Time Envirement do progama argus
 * 
 * @param displayname Indica se pretende-se mostrar a identificação do progama em cada comando
 * @return int 0 se acabou com sucesso, qualquer outro numero se ocorreu um erro
 */
int argusRTE()
{
    argusRTE_init();
    char *comand, *objects;
    write(1, argusTag, strlen(argusTag));
    while (argusRTE_readcomand(0, &comand, &objects) > 0)
    {
        argusRTE_run(comand, objects);
        argusRTE_readcomand_free(comand, objects);
        write(1, argusTag, strlen(argusTag));
    }
    argusRTE_kill();
    return 0;
}