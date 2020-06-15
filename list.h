
#ifndef list_h
#define list_h

typedef struct _list{ 
    void * data;
    struct _list *next;
} *List;

/**
 * @brief 
 * 
 * @return List 
 */
List List_alloc();

/**
 * @brief 
 * 
 * @param l 
 * @param data 
 * @return List 
 */
List List_prepend(List l, void *data);

/**
 * @brief 
 * 
 * @param l 
 * @return int 
 */
int List_length(List l);

/**
 * @brief 
 * 
 * @param l 
 * @param datafreefunc 
 */
void List_free(List l, void (*datafreefunc)(void *data));

/**
 * @brief 
 * 
 * @param l 
 * @param keyfinder 
 * @return void** 
 */
void ** List_lookupPointer(List l, int (*keyfinder)(void * data));

/**
 * @brief 
 * 
 * @param l 
 * @param data 
 * @return List 
 */
List List_append(List l,void * data);

/**
 * @brief 
 * 
 * @param l 
 */
void List_lfree(List l);

/**
 * @brief 
 * 
 * @param l 
 * @param datafreefunc 
 */
List List_tail (List l);

#endif