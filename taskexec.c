#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include "constants.h"

int *pids = NULL, ind = 0, arrsize = 1;

/**
 * @brief Adiciona um ProcessID ao registo dos mesmos
 * 
 * @param pid ProcessID a ser registado
 */
void addPID(int pid)
{
    if (ind >= arrsize)
    {
        arrsize *= 2;
        pids = realloc(pids, arrsize * sizeof(int));
    }
    pids[ind++] = pid;
}

/**
 * @brief Dado um Sinal termina a tarefa a ser executada
 * 
 * @param signum Sinal a ser processado
 */
void killTask(int signum)
{
    for (int i = 0; i < ind; i++)
        kill(SIGKILL, pids[i]);
    switch (signum)
    {
    case SIGINT:
        _exit(1);
    case SIGALRM:
        _exit(2);
    }
}

/**
 * @brief Envia um sinal de interrupção ao processo pai
 * 
 * @param signum Sinal original
 */
void IterruptFather(int signum)
{
    kill(getppid(), SIGINT);
    _exit(1);
}

/**
 * @brief Despeja o conteudo de STDINT para STDOUT, se ao fim de um dado tempo nao ouver trasferencia de dados envia um Sinal de Alarme ao progama principal
 * 
 * @param limit Tempo limite de inatividade
 */
void idlelimit(unsigned int limit)
{
    char buffer[ReadBufferSize];
    int i = 0;
    alarm(limit);
    while ((i = read(0, buffer, ReadBufferSize)) > 0)
    {
        write(1, buffer, i);
        alarm(limit);
    }
}

/**
 * @brief Substitui a execução atual pelo comando dado
 * 
 * @param comando comando a ser executado
 * @return int (-1) se o comando for invalido
 */
int execSystem(char *comando)
{
    int i, tam = 1;
    for (i = 0; comando[i]; i++)
        if (comando[i] == ' ')
        {
            tam++;
            while (comando[i++] == ' ')
                ;
        }

    char **res = calloc(tam, sizeof(char *));
    char *p = strtok(comando, " ");
    for (i = 0; p; i++)
    {
        res[i] = p;
        p = strtok(NULL, " ");
    }
    execvp(res[0], res);
    return -1;
}

/**
 * @brief Executa uma Tarefa
 * 
 * @param cmdN Nº de comandos da Tarefa
 * @param cmdL Lista de comandos da Tarefa
 * @param RuntimeMax Tempo de Execução Maximo da tarefa (se for negativo significa que a tarefa nao tem limite de tempo)
 * @param IdleTimeMax Tempo Maximo de inatividade de cada subtarefa (se for negativo significa qnao existe limite)
 * @return int 
 */
int task(int cmdN, char **cmdL, int RuntimeMax, int IdleTimeMax)
{
    signal(SIGALRM, killTask);
    signal(SIGINT, killTask);
    int i, st, pp[2], stdout = dup(1);

    for (i = 0; i < cmdN - 1; i++)
    {
        if (pipe(pp) < 0)
            _exit(-1); //Se houver um erro ao abrir um pipe a tarefa é cancelada
        if ((st = fork()) == 0)
        {
            close(pp[0]);
            dup2(pp[1], 1);
            close(pp[1]);
            execSystem(cmdL[i]);
            _exit(-1);
        }
        if (st < 0)
            _exit(-1); //Se houver um erro ao criar um filho a tarefa é cancelada
        close(pp[1]);
        dup2(pp[0], 0);
        close(pp[0]);

        //Lim
        if (IdleTimeMax > 0)
        {
            if (pipe(pp) < 0)
                _exit(-1); //Se houver um erro ao abrir um pipe a tarefa é cancelada
            if ((st = fork()) == 0)
            {
                signal(SIGALRM, IterruptFather);
                close(pp[0]);
                dup2(pp[1], 1);
                close(pp[1]);
                idlelimit(IdleTimeMax);
                _exit(0);
            }
            if (st < 0)
                _exit(-1); //Se houver um erro ao criar um filho a tarefa é cancelada
            close(pp[1]);
            dup2(pp[0], 0);
            close(pp[0]);
        }
    }
    dup2(stdout, 1);
    if ((st = fork()) == 0)
    {
        execSystem(cmdL[cmdN - 1]);
    }
    if (st < 0)
        _exit(-1); //Se houver um erro ao criar um filho a tarefa é cancelada

    if (RuntimeMax > 0)
        alarm(RuntimeMax);

    waitpid(st, &st, WUNTRACED);

    _exit(0);
}