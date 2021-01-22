/*
 *  ADDR.H
 *
 *  Released to the public domain.
 *
 *  Prototypes for ADDR.C.
 */

#ifndef __ADDR_H__
#define __ADDR_H__
/*
 *  Structure defining a "five-dimensional" FidoNet address, or an
 *  Internet address (stored in the domain).
 */
typedef struct _address
{
    unsigned int zone;
    unsigned int net;
    unsigned int node;
    unsigned int point;
    char *       domain;
    unsigned int notfound  : 1;
    unsigned int fidonet   : 1;
    unsigned int internet  : 1;
    unsigned int bangpath  : 1;
    unsigned int dontmatch : 1;  /* don't apply aka matching to this address */
} FIDO_ADDRESS;
/* please call copy_addr for assigning addresses because the domain string
   must be strdup'ed
 */
char * show_address(FIDO_ADDRESS * a);
char * show_4d(FIDO_ADDRESS * a);
FIDO_ADDRESS parsenode(char * t);
void parse_internet_address(const char *, char **, char **);
char * compose_internet_address(const char *, const char *);
int akamatch(FIDO_ADDRESS * pfrom, FIDO_ADDRESS * pto);
void copy_addr(FIDO_ADDRESS * pdest, FIDO_ADDRESS * psource);

#endif
