/*
 *  DLIST.H - Doubly linked-list management functions.
 *  Adapted from 1995 public domain C code by Scott Pitcher.
 *  Modified 1995-1996 by Andrew Clarke and released to the public domain.
 */

#ifndef __DLIST_H__
#define __DLIST_H__

typedef struct dlistnode
{
    struct dlistnode *L_prev;
    struct dlistnode *L_next;
    void *L_element;
}
DLISTNODE;

typedef struct
{
    DLISTNODE *L_first;
    DLISTNODE *L_last;
    unsigned long L_elements;
}
DLIST;

DLIST *dlistInit(void);
void dlistTerm(DLIST * p_list);
void *dlistGetElement(DLISTNODE * p_node);
void dlistSetElement(DLISTNODE * p_node, void *p_element);
DLISTNODE *dlistCreateNode(void *p_element);
void dlistDeleteNode(DLISTNODE * p_node);
void dlistAddNode(DLIST * p_list, DLISTNODE * p_node);
void dlistDropNode(DLIST * p_list, DLISTNODE * p_node);

DLISTNODE *dlistTravFirst(DLIST * p_list);
DLISTNODE *dlistTravLast(DLIST * p_list);
DLISTNODE *dlistTravPrevious(DLISTNODE * p_node);
DLISTNODE *dlistTravNext(DLISTNODE * p_node);

int dlistCompareNodes(DLISTNODE * p_node1, DLISTNODE * p_node2, int (*fcmp) (const void *, const void *));
void dlistSwapNodes(DLISTNODE * p_node1, DLISTNODE * p_node2);
DLISTNODE *dlistSearch(DLIST * p_list, void *p_element, int (*fcmp) (const void *, const void *));
unsigned long dlistTotalNodes(DLIST * p_list);
int dlistIsEmpty(DLIST * p_list);

#endif
