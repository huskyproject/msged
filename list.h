/*
 *  LIST.H
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 */

#ifndef __LIST_H__
#define __LIST_H__

typedef struct _mlhead
{
    int sel;
    unsigned long msgnum;
    unsigned long umsgid;
    char to_name[37];
    int to_net;
    int to_node;
    char fr_name[37];
    int fr_net;
    int fr_node;
    char subj[73];
    int times_read;
}
MLHEAD;

void do_list(void);

#endif
