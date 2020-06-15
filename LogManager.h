
#ifndef LogManager_h
#define LogManager_h

#include <stdlib.h>


/**
 * @brief Le no maximo nbytes de um dado ficheiro imprime a primeira linha do mesmo num dado buffer
 * 
 * @param fd File descriptor a ser lido
 * @param buffer Buffer a ser escrito
 * @param nbytes Nº de bytes limite que se podem ler
 * @return int Nº de bytes lidos
 */
int readln(int fd, char *buffer, unsigned int nbytes);

/**
 * @brief Le uma linha do dado descritor caracter a caracter WARNING: INEFICIENTE, usar so quando fd possa ser um pipe com pouco fluxo de dados
 * 
 * @param fd File descriptor a ser lido
 * @param buffer Buffer a ser escrito
 * @param nbytes Nº de bytes limite que se podem ler
 * @return int Nº de bytes lidos
 */
int readlncc(int fd, char *buffer, unsigned int nbytes);

/**
 * @brief Abre um ficheiro idx, ou cria um caso este seja invalido ou nao existir
 * 
 * @param nLinhas nº de linhas do ficheiro aberto
 * @return int descritor do ficheiro idx ou -1 caso tenha ocorrido um erro
 */
int openIDX(int *nLinhas);

/**
 * @brief Atualiza um entrada no ficheiro idx, se esta ja existir
 * 
 * @param index Indice da entrada a ser atualizada, se for negativa adiciona a entrada no fim do ficheiro
 * @param valor Valor a colocar nessa entrada
 */
void updateIDX(int idxfile, int index, int valor);

/**
 * @brief Le um dado indice do ficheiro;
 * 
 * @param index 
 * @return int 
 */
int readIndexIDX(int idxfile, int index);

/**
 * @brief Abre o ficheiro de logs, ou cria um novo caso este nao exista
 * 
 * @return int descritor do ficheiro de logs ou -1 caso tenha ocorrido um erro
 */
int openLogs();

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
int updateLogs(int logfile, char* comando, int inatividade, int execucao, int filetocopy);

/**
 * @brief Escreve o registo a partir de uma dada posiçao do ficheiro de logs
 * 
 * @param logfile descritor do ficheiro de logs a ser lido
 * @param destination_file descritor onde sera escrito o output
 * @param file_index posiçao do output no ficheiro
 * @return int 
 */
int writeOutputTo(int logfile, int destination_file, off_t file_index);

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
int getCommandInfo(int logfile, off_t file_index, char output_comand[], int output_comand_size, int *inatividade, int *execucao);

#endif