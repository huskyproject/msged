/* Special characters */

#ifndef __SPECCH_H__
#define __SPECCH_H__

#if defined(MSDOS) || defined(OS2) || defined(WINNT)

#define SC1 0xBA
#define SC2 0xCD
#define SC3 0xC9
#define SC4 0xBB
#define SC5 0xC8
#define SC6 0xBC
#define SC7 0xB3
#define SC8 0xC4
#define SC9 0xDA
#define SC10 0xBF
#define SC11 0xC0
#define SC12 0xD9
#define SC13 0xB1
#define SC14 0x10
#define SC15 0x11
#define SC16 0xdc
#define SC17 0xdf
#define SC18 0x18
#define SC19 0x19
#define SC20 0x14
#define SC21 0x1d

#else

#define SC1 '|'
#define SC2 '-'
#define SC3 '*'
#define SC4 '*'
#define SC5 '*'
#define SC6 '*'
#define SC7 '|'
#define SC8 '-'
#define SC9 '*'
#define SC10 '*'
#define SC11 '*'
#define SC12 '*'
#define SC13 '*'
#define SC14 '>'
#define SC15 '<'
#define SC16 '!'
#define SC17 '*'
#define SC18 '^'
#define SC19 'v'
#define SC20 '&'
#define SC21 '*'

#endif

#endif
