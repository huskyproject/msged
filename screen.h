/*
 *  SCREEN.H
 *
 *  Released to the public domain.
 *
 *  Prototypes for SCREEN.C.
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

void cursor(char state);
unsigned int GetKey(void);
unsigned int KeyHit(void);
unsigned int ConvertKey(int ch);

#endif
