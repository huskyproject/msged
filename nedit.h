/*
 *  NEDIT.H
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  This structure defines a LINE of text.
 */

#ifndef __NEDIT_H__
#define __NEDIT_H__

/*
 *  Probably possible to have an extra field called block_len, containing
 *  the length of the block - this would allow rectangular blocks.
 */

typedef struct _line
{
    char *text;                 /* pointer to actual line text */
    unsigned int block  : 1;    /* this is in a block */
    unsigned int hide   : 1;    /* this is a hidden line */
    unsigned int quote  : 1;    /* this is a quoted line */
    unsigned int templt : 1;    /* was this a template line? */
    int column;                 /* if a block, starting column */
    struct _line *prev;         /* previous line in BUFFER */
    struct _line *next;         /* next line in BUFFER */
}
LINE;

#endif
