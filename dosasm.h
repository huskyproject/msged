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
void winpause(void);
int dvcheck(void);
int kbdhit(void);
unsigned int obtkey(void);
unsigned int dosavmem(void);

#endif
