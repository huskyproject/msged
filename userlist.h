/*
 *  USERLIST.H
 *
 *  Written by Paul Edwards and modified by Bill Bond.
 *  Released to the public domain.
 *
 *  Prototypes for USERLIST.C.
 */

#ifndef __USERLIST_H__
#define __USERLIST_H__
 
ADDRESS lookup(char *name, char *fn);
void makeReverse(char *revName, char *name);

#endif
