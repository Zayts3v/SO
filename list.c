#include "list.h"
#include <stdlib.h>



List List_alloc()
{
    return calloc(1, sizeof(struct _list));
}

List List_prepend(List l, void *data)
{
    List res = calloc(1, sizeof(struct _list));
    res->data = data;
    res->next = l;
    return res;
}

List List_append(List l,void * data){

    List *ap = &l;
    while(*ap) ap = &(*ap)->next;
    
    *ap = List_alloc();
    (*ap)->data = data;

    return l;
}

int List_length(List l)
{
    int i;
    for (i = 0; l; l = l->next, i++)
        ;
    return i;
}

void List_free(List l, void (*datafreefunc)(void *data))
{
    while (l)
    {
        List t0 = l->next;
        if (l->data)
            (*datafreefunc)(l->data);
        free(t0);
    }
}

