/*
 *  DLIST.C - Doubly linked-list management functions.
 *  Adapted from 1995 public domain C code by Scott Pitcher.
 *  Modified 1995-1996 by Andrew Clarke and released to the public domain.
 */

#include <stdlib.h>
#include "dlist.h"
#include "unused.h"

DLIST * dlistInit(void)
{
    DLIST * p_list;

    p_list = malloc(sizeof *p_list);

    if(p_list != NULL)
    {
        p_list->L_first    = NULL;
        p_list->L_last     = NULL;
        p_list->L_elements = 0L;
    }

    return p_list;
}

void dlistTerm(DLIST * p_list)
{
    DLISTNODE * p_node;

    if(p_list != NULL)
    {
        if(p_list->L_elements != 0)
        {
            p_node = p_list->L_first;

            while(p_node != NULL)
            {
                p_list->L_first = p_node->L_next;
                free(p_node);
                p_node = p_list->L_first;
            }
        }

        free(p_list);
    }
}

void * dlistGetElement(DLISTNODE * p_node)
{
    if(p_node != NULL)
    {
        return p_node->L_element;
    }
    else
    {
        return NULL;
    }
}

void dlistSetElement(DLISTNODE * p_node, void * p_element)
{
    if(p_node != NULL)
    {
        p_node->L_element = p_element;
    }
}

DLISTNODE * dlistCreateNode(void * p_element)
{
    DLISTNODE * p_node;

    p_node = malloc(sizeof *p_node);

    if(p_node != NULL)
    {
        p_node->L_next    = NULL;
        p_node->L_prev    = NULL;
        p_node->L_element = p_element;
    }

    return p_node;
}

void dlistDeleteNode(DLISTNODE * p_node)
{
    if(p_node != NULL)
    {
        free(p_node);
    }
}

void dlistAddNode(DLIST * p_list, DLISTNODE * p_node)
{
    if(p_list != NULL)
    {
        p_node->L_prev = p_list->L_last;

        if(p_node->L_prev != NULL)
        {
            p_list->L_last->L_next = p_node;
        }

        p_node->L_next = NULL;

        if(p_list->L_first == NULL)
        {
            p_list->L_first = p_node;
        }

        p_list->L_last = p_node;
        p_list->L_elements++;
    }
}

void dlistDropNode(DLIST * p_list, DLISTNODE * p_node)
{
    DLISTNODE * poldnext;

    poldnext = p_node->L_next;

    if(p_list != NULL)
    {
        if(p_list->L_first == p_node)
        {
            p_list->L_first = p_node->L_next;
        }

        if(p_list->L_last == p_node)
        {
            p_list->L_last = p_node->L_prev;
        }

        if(p_node->L_next != NULL)
        {
            (p_node->L_next)->L_prev = p_node->L_prev;
            p_node->L_next           = NULL;
        }

        if(p_node->L_prev != NULL)
        {
            p_node->L_prev->L_next = poldnext;
            p_node->L_prev         = NULL;
        }

        p_list->L_elements--;
    }
} /* dlistDropNode */

DLISTNODE * dlistTravFirst(DLIST * p_list)
{
    if(p_list != NULL)
    {
        return p_list->L_first;
    }
    else
    {
        return NULL;
    }
}

DLISTNODE * dlistTravLast(DLIST * p_list)
{
    if(p_list != NULL)
    {
        return p_list->L_last;
    }
    else
    {
        return NULL;
    }
}

DLISTNODE * dlistTravPrevious(DLISTNODE * p_node)
{
    if(p_node != NULL)
    {
        return p_node->L_prev;
    }
    else
    {
        return NULL;
    }
}

DLISTNODE * dlistTravNext(DLISTNODE * p_node)
{
    if(p_node != NULL)
    {
        return p_node->L_next;
    }
    else
    {
        return NULL;
    }
}

int dlistCompareNodes(DLISTNODE * p_node1, DLISTNODE * p_node2, int (* fcmp)(const void *,
                                                                             const void *))
{
    return fcmp(p_node1->L_element, p_node2->L_element);
}

void dlistSwapNodes(DLISTNODE * p_node1, DLISTNODE * p_node2)
{
    DLISTNODE * p_temp;

    if(p_node1 != NULL && p_node2 != NULL)
    {
        p_temp          = p_node1->L_next;
        p_node1->L_next = p_node2->L_next;
        p_node2->L_next = p_temp;
        p_temp          = p_node1->L_prev;
        p_node1->L_prev = p_node2->L_prev;
        p_node2->L_prev = p_temp;
    }
}

DLISTNODE * dlistSearch(DLIST * p_list, void * p_element, int (* fcmp)(const void *, const void *))
{
    DLISTNODE * p_node;

    if(p_list != NULL && p_list->L_elements != 0)
    {
        p_node = p_list->L_first;

        while(p_node != NULL)
        {
            if(fcmp(p_node->L_element, p_element) == 0)
            {
                return p_node;
            }
            else
            {
                p_node = p_node->L_next;
            }
        }
    }

    return NULL;
}

unsigned long dlistTotalNodes(DLIST * p_list)
{
    if(p_list != NULL)
    {
        return p_list->L_elements;
    }
    else
    {
        return 0;
    }
}

int dlistIsEmpty(DLIST * p_list)
{
    return p_list->L_elements == 0;
}

#ifdef TEST

#include <stdio.h>

int main(void)
{
    DLIST * p_list;
    DLISTNODE * p_node;

    p_list = dlistInit();
    p_node = dlistCreateNode("One banana");
    dlistAddNode(p_list, p_node);
    p_node = dlistCreateNode("Two banana");
    dlistAddNode(p_list, p_node);
    p_node = dlistCreateNode("Three banana");
    dlistAddNode(p_list, p_node);
    p_node = dlistCreateNode("Four banana");
    dlistAddNode(p_list, p_node);
    p_node = dlistCreateNode("Five banana");
    dlistAddNode(p_list, p_node);
    p_node = dlistTravFirst(p_list);

    while(p_node != NULL)
    {
        printf("%s\n", (char *)dlistGetElement(p_node));
        p_node = dlistTravNext(p_node);
    }
    dlistTerm(p_list);
    return 0;
}

#endif /* ifdef TEST */
