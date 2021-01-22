/*
 *  DOSASM.H
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  Miscellaneous MS-DOS function prototypes.
 */

#ifndef __DOSASM_H__
#define __DOSASM_H__

void dospause(void);
void dvpause(void);
int dvcheck(void);
void dpmipause(void);
int dpmicheck(void);
int kbdhit(void);
unsigned int obtkey(void);
unsigned int dosavmem(void);

#endif
