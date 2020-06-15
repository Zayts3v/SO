#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "argus.h"

/**
 * @brief magic number associado aos ficheiro idx compativel com o progama
 */
const int magicnumber = 0x00000C01;
const int magnumANDnullsize[2] = {0x00000C01, 0};

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
    if (res <= 0)
        return res;
    for (int i = 0; i < res; i++)
        if (buffer[i] == '\n')
        {
            buffer[i] = '\0';
            break;
        }
    return res;
}

/**
 * @brief Le uma linha do dado descritor caracter a caracter WARNING: INEFICIENTE, usar so quando fd possa ser um pipe com pouco fluxo de dados
 * 
 * @param fd File descriptor a ser lido
 * @param buffer Buffer a ser escrito
 * @param nbytes Nº de bytes limite que se podem ler
 * @return int Nº de bytes lidos
 */
int readlncc(int fd, char *buffer, unsigned int nbytes)
{
    int i;
    char c;
    for (i = 0; i < nbytes; i++)
    {   
        int res;
        if((res=read(fd, &c, 1))<=0) return res;
        if (c == '\n')
        {
            buffer[i] = '\0';
            break;
        }
        buffer[i] = c;
    }
    return i;
}

//---------------LOGS IDX---------------//

/**
 * @brief Abre um ficheiro idx, ou cria um caso este seja invalido ou nao existir
 * 
 * @param nLinhas nº de linhas do ficheiro aberto
 * @return int descritor do ficheiro idx ou -1 caso tenha ocorrido um erro
 */
int openIDX(int *nLinhas)
{

    int fi = open(idxname, O_RDWR, 0666);

    if (fi > -1)
    {
        int i;
        if (read(fi, &i, 4) > 0)
            if (magicnumber == i)
                if (read(fi, &i, 4) > 0)
                {
                    *nLinhas = i;
                    return fi;
                }
    }
    fi = open(idxname, O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (write(fi, magnumANDnullsize, sizeof(magnumANDnullsize)) > 0)
    {
        *nLinhas = 0;
        return fi;
    }
    return -1;
}

/**
 * @brief Atualiza um entrada no ficheiro idx, se esta ja existir
 * 
 * @param index Indice da entrada a ser atualizada, se for negativa adiciona a entrada no fim do ficheiro
 * @param valor Valor a colocar nessa entrada
 */
void updateIDX(int idxfile, int index, int valor)
{
    int i;

    lseek(idxfile, 4, SEEK_SET);
    read(idxfile, &i, sizeof(int));
    if (index < 0 || index > i)
    {
        index = i++;
        lseek(idxfile, 4, SEEK_SET);
        write(idxfile, &i, sizeof(int));
    }

    lseek(idxfile, index * sizeof(int), SEEK_CUR);
    write(idxfile, &valor, sizeof(valor));
}

/**
 * @brief Le um dado indice do ficheiro;
 * 
 * @param index 
 * @return int 
 */
int readIndexIDX(int idxfile, int index)
{
    int pos = 8 + (index * sizeof(int));

    if (lseek(idxfile, pos, SEEK_SET) != pos)
        return -1;

    int v;
    if (read(idxfile, &v, sizeof(int)) < 0)
        return -1;

    return v;
}

//-----------------LOGS-----------------//

/**
 * @brief Abre o ficheiro de logs, ou cria um novo caso este nao exista
 * 
 * @return int descritor do ficheiro de logs ou -1 caso tenha ocorrido um erro
 */
int openLogs()
{
    int fl = open(logfilename, O_RDWR, 0666);
    if (fl < 0)
    {
        fl = open(logfilename, O_CREAT | O_TRUNC | O_RDWR, 0666);
        if (fl < 0 || write(fl, "\0\0\0", sizeof(int)) < 0)
            return -1;
    }

    return fl;
}

/**
 * @brief Adiciona uma entrada nos logs a partir dos parametros dados
 * 
 * @param logfile descritor do ficheiro de logs a ser atualizado
 * @param comando comando que provocou este output;
 * @param inatividade int valor tempo inatividade
 * @param execucao int valor tempo execucao
 * @param filetocopy descritor do output
 * @return int 
 */
int updateLogs(int logfile, char *comando, int inatividade, int execucao, int filetocopy)
{

    lseek(filetocopy, 0, SEEK_SET); //NOTA: Se filetocopy for um pipe isto da erro e nao faz nada

    int pos = lseek(logfile, 0, SEEK_END); //escreve no fim
    if (pos < 0)
        return -1;

    //write int int comando
    write(logfile, &inatividade, sizeof(int));
    write(logfile, &execucao, sizeof(int));
    write(logfile, comando, strlen(comando));
    write(logfile, "\n", 1);

    //write time + output

    const time_t timer = time(NULL);
    char *time = ctime(&timer);
    write(logfile, time, strlen(time));
    write(logfile, "\n", 1);

    ssize_t n;
    char buffer[ReadBufferSize];
    while ((n = read(filetocopy, buffer, ReadBufferSize)) > 0)
        write(logfile, buffer, n);

    write(logfile, "\0", 1);

    return pos;
}

/**
 * @brief Escreve o registo a partir de uma dada posiçao do ficheiro de logs
 * 
 * @param logfile descritor do ficheiro de logs a ser lido
 * @param destination_file descritor onde sera escrito o output
 * @param file_index posiçao do output no ficheiro
 * @return int 
 */
int writeOutputTo(int logfile, int destination_file, off_t file_index)
{
    if (lseek(logfile, file_index + 8, SEEK_SET) < 0)
        return -1;

    ssize_t n;
    char buffer[ReadBufferSize];
    buffer[ReadBufferSize - 1] = '\0';
    while ((n = read(logfile, buffer, ReadBufferSize - 1)) > 0)
    {
        int t0 = strlen(buffer);
        write(destination_file, buffer, t0);
        if (t0 < n)
            break;
    }

    write(destination_file, "\n", 1);

    return 0;
}

/**
 * @brief Devolve a informaçao do output de uma tarefa a partir do ficheiro logs
 * 
 * @param logfile descritor do ficheiro de logs a ser lido
 * @param file_index posiçao do output no ficheiro
 * @param output_comand array onde escrever o comando
 * @param output_comand_size tamanho do array que foi fornecido
 * @param inatividade int valor tempo inatividade
 * @param execucao int valor tempo execucao
 * @return int 0 se correu tudo bem
 */

int getCommandInfo(int logfile, off_t file_index, char output_comand[], int output_comand_size, int *inatividade, int *execucao)
{
    if (lseek(logfile, file_index + sizeof(int), SEEK_SET) < 0)
        return -1;

    int i;
    read(logfile, &i, 4);
    *inatividade = i;

    read(logfile, &i, 4);
    *execucao = i;

    char buffer[MaxLineSize];
    readln(logfile, buffer, MaxLineSize);

    output_comand_size--;
    for (i = 0; buffer[i] && i < output_comand_size; i++)
    {
        output_comand[i] = buffer[i];
    }
    output_comand[i] = '\0';

    return 0;
}
