#include "list.h"
#include <stdlib.h>

/**
 * @brief Aloca espaço para uma bloco da lista
 * 
 * @return res Retorna o bloco criado
 */
List List_alloc()
{
    return calloc(1, sizeof(struct _list));
}

/**
 * @brief Adiciona um elemento ao ínicio da lista
 * 
 * @param l Lista ligada
 * @param data Data que irá ser adicionada
 * @return res Lista
 */
List List_prepend(List l, void *data)
{
    List res = calloc(1, sizeof(struct _list));
    res->data = data;
    res->next = l;
    return res;
}

/**
 * @brief Adiciona um elemento ao fim da lista
 * 
 * @param l Lista ligada
 * @param data Data que irá ser adicionada
 * @return l Lista
 */
List List_append(List l, void *data)
{

    List *ap = &l;
    while (*ap)
        ap = &(*ap)->next;

    *ap = List_alloc();
    (*ap)->data = data;

    return l;
}


/**
 * @brief Determina o tamanho da lista
 * 
 * @param l Lista ligada
 * @return i Tamanho da lista
 */
int List_length(List l)
{
    int i;
    for (i = 0; l; l = l->next, i++)
        ;
    return i;
}

/**
 * @brief Liberta uma lista ligada, através uma função
 * 
 * @param l Lista ligada
 * @param 
 */
void List_free(List l, void (*datafreefunc)(void *data))
{
    while (l)
    {
        List t0 = l;
        l = l->next;
        if (t0->data)
            (*datafreefunc)(l->data);
        free(t0);
    }
}

/**
 * @brief Liberta uma lista ligada
 * 
 * @param l Lista ligada
 */
void List_lfree(List l)
{
    while (l)
    {
        List t0 = l;
        l = l->next;
        free(t0);
    }
}

/**
 * @brief Retira a cabeça a uma lista ligada
 * 
 * @param l Lista ligada
 * @return bruhmium Tail da lista ligada
 */
List List_tail(List l)
{
    if (!l)
        return NULL;
    List bruhmium = l->next;
    free(l);
    return bruhmium;
}