#ifndef Interface_h
#define Interface_h


/**
 * @brief 
 * 
 */
void argusRTE_init();

void argusRTE_kill();

int argusRTE_readcomand(int filetoread, char **comand, char **objects);

void argusRTE_readcomand_free(char *comand, char *objects);

/**
 * @brief Executa um comando
 * 
 * @param comand comando ser executado
 * @param objects objetos do comando (null se nao for aplicavel)
 * @return int 
 */
int argusRTE_run(char *comand, char *objects);

/**
 * @brief Runt Time Envirement do progama argus
 * 
 * @param displayname Indica se pretende-se mostrar a identificação do progama em cada comando
 * @return int 0 se acabou com sucesso, qualquer outro numero se ocorreu um erro
 */
int argusRTE();

#endif 