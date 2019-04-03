#ifndef __ES_LIST_H
#define __ES_LIST_H


typedef struct list_node {
  struct list_node  *next;          //point to next node
  struct list_node  *prev;          //point to prev node
} list_node;
typedef struct list_node list_t;                  /**< Type for lists. */

inline void list_init(list_t *l)
{
    l->next = l->prev = l;
}

/**
 * @brief insert a node after a list
 *
 * @param l list to insert it
 * @param n new node to be inserted
 */
inline void list_insert_after(list_t *l, list_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

         /**
 * @brief insert a node before a list
 *
 * @param n new node to be inserted
 * @param l list to insert it
 */
inline void list_insert_before(list_t *l, list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * @brief remove node from list.
 * @param n the node to remove from the list.
 */
inline void list_remove(list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief get the list length
 * @param l the list to get.
 */
inline unsigned int list_len(const list_t *l)
{
    unsigned int len = 0;
    const list_t *p = l;
    while (p->next != l)
    {
        p = p->next;
        len ++;
    }

    return len;
}


/**
 * @brief tests whether a list is empty
 * @param l the list to test.
 */
inline int list_isempty(const list_t *l)
{
    return l->next == l;
}

/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define list_entry(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))

/*@}*/










#endif

