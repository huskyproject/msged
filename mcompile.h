/*
 *  MCOMPILE.H
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Miscellaneous macro definitions.
 */

#ifndef __MCOMPILE_H__
#define __MCOMPILE_H__

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef PATHLEN
#undef PATHLEN
#endif

#endif
