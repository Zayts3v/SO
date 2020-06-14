#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "constants.h"

/**
 * @brief magic number associado aos ficheiro idx compativel com o progama
 */
const int magicnumber = 0x00000C01;
const int magnumANDnullsize[2] = {magicnumber, 0};

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
    if (res == 0)
        return -1;
    for (int i = 0; i < res; i++)
        if (buffer[i] == '\n')
        {
            buffer[i] = '\0';
            break;
        }
    return res;
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
        index = i;
    lseek(idxfile, 4, SEEK_SET);
    write(idxfile, &i, sizeof(int));

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
 * @param filetocopy descritor do output
 * @return int 
 */
int updateLogs(int logfile, char *comando, int descriptortocopy)
{

    lseek(descriptortocopy, 0, SEEK_SET); //NOTA: Se descriptortocopy for um pipe isto da erro e nao faz nada

    int pos = lseek(logfile, 0, SEEK_END); //escreve no fim
    if (pos < 0)
        return -1;

    write(logfile, comando, strlen(comando)); //write comando
    write(logfile, "\n", 1);

    ssize_t n;
    char buffer[ReadBufferSize];
    while ((n = read(descriptortocopy, buffer, ReadBufferSize)) > 0)
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

    if (lseek(logfile, file_index, SEEK_SET) < 0)
        return -1;

    ssize_t n;
    char buffer[ReadBufferSize];
    buffer[ReadBufferSize - 1] = '\0';
    while ((n = read(logfile, buffer, ReadBufferSize - 1)) > 0)
    {
        int t0 = strlen(buffer);
        write(logfile, buffer, t0);
        if (t0 < n)
            break;
    }

    return 0;
}

/**
 * @brief Devolve a informaçao do output de uma tarefa a partir do ficheiro logs
 * 
 * @param logfile descritor do ficheiro de logs a ser lido
 * @param destination_file descritor onde sera escrito o output
 * @param file_index posiçao do output no ficheiro
 * @param output_comand array onde escrever o comando
 * @param output_comand_size tamanho do array que foi fornecido
 * @return int 0 se correu tudo bem
 */
int getOutputInfo(int logfile, int destination_file, off_t file_index, char output_comand[], int output_comand_size)
{
    char buffer[MaxLineSize];
    if (lseek(logfile, file_index, SEEK_SET) < 0)
        return -1;

    readln(logfile, buffer, MaxLineSize);

    int i;
    output_comand_size--;
    for (i = 0; buffer[i] && i < output_comand_size; i++)
    {
        output_comand[i] = buffer[i];
    }
    output_comand[i] = '\0';

    return 0;
}
