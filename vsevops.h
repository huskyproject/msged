/*
 *  VSEVOPS.H
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  Prototypes for VSEVOPS.C.
 */

#ifndef __VSEVOPS_H__
#define __VSEVOPS_H__

char *v7lookupnode(ADDRESS * faddr, char *name);
ADDRESS v7lookup(char *name);
char *v7lookupsystem(ADDRESS * faddr, char *system);

#endif
