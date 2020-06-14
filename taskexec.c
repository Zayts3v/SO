#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "constants.h"
#include "list.h"

List pids = NULL;

#include <stdio.h>

/**
 * @brief Dado um Sinal termina a tarefa a ser executada
 * 
 * @param signum Sinal a ser processado
 */
void killTask(int signum)
{
    for (List l; l; l = l->next)
        kill(SIGKILL, *((int *)l->data));
    switch (signum)
    {
    case 0:
        return;
    case SIGTERM:
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
void TerminateFather(int signum)
{
    kill(getppid(), SIGTERM);
    _exit(1);
}

/**
 * @brief Adiciona um ProcessID ao registo dos mesmos
 * 
 * @param pid ProcessID a ser registado
 */
void addPID(int pid)
{
    int *t = calloc(1, sizeof(int));
    *t = pid;
    pids = List_prepend(pids, t);
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

int task(char *comand, int RunTimeMax, int IdleTimeMax)
{
    if (comand == NULL)
        return -1;

    signal(SIGALRM, killTask);
    signal(SIGTERM, killTask);

    List comandList = NULL;
    char *p = strtok(comand, "|");
    while (p)
    {
        comandList = List_append(comandList, p);
        p = strtok(NULL, "|");
    }

    char tITM = (IdleTimeMax > 0 ? 1 : 0);
    int st, pp[2], stdout = dup(1);
    List t0 = NULL;

    for (t0 = comandList; t0 && t0->next; t0 = t0->next)
    {

        for (int cycle = 0; cycle < tITM + 1; cycle++) //SE tITM for 1 o ciclo itera duas vezes senao itera apenas uma vez
        {
            if (pipe(pp) < 0)
                _exit(-1); //Se houver um erro ao abrir um pipe a tarefa é cancelada
            if ((st = fork()) == 0)
            {
                close(pp[0]);
                dup2(pp[1], 1);
                close(pp[1]);

                if (cycle < 1)
                    execSystem((char *)t0->data);
                else //se nao estivermos na primeira iteração do ciclo em vez de se executar um comando o processo fica a medir
                {    //a variaçao do fluxo do pipe, se esta variaçao nao satisfaz o tempo limite um SIGTERM é enviado ao seu pai
                    signal(SIGALRM, TerminateFather);
                    idlelimit(IdleTimeMax);
                }

                _exit(-1);
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
        execSystem((char *)t0->data);
    }
    if (st < 0)
        _exit(-1); //Se houver um erro ao criar um filho a tarefa é cancelada


    if (RunTimeMax > 0)
        alarm(RunTimeMax);

    waitpid(st, &st, WUNTRACED);

    List_lfree(comandList);
    List_free(pids,free);

    killTask(0);

    return st;
}