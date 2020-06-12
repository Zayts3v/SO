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
            datafreefunc(l->data);
        free(t0);
    }
}

void ** List_lookupPointer(List l, int (*keyfinder)(void * data)){
    for(;l;l=l->next){
        if (l -> data && keyfinder(l->data)){
            return &(l->data);
        }
    }
    return NULL;
}