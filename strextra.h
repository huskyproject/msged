/*
 *  STREXTRA.H
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis,
 *  Paul Edwards and Andrew Clarke.  Released to the public domain.
 *
 *  A few string handling routines for Msged.
 */

#ifndef __STREXTRA_H__
#define __STREXTRA_H__

#ifndef __DIRUTE_C
#include <huskylib/compiler.h>
#endif

#ifdef __MINGW32
#define strncmpi _strncmpi
#endif

int strncmpi(const char * s, const char * t, size_t n);
void strdel(char * l, int x);

#if !(defined (_MSC_VER) && (_MSC_VER >= 1200))

#ifndef UNIX
int stricmp(const char * s, const char * t);

#endif
#ifndef UNIX
char * strdup(const char * s);

#endif


#ifndef __IBMC__
int memicmp(const void * s1, const void * s2, size_t n);

#else
int memicmp(void * s1, void * s2, size_t n);

#endif


char * strlwr(char * s);
char * strupr(char * s);

#endif

#ifndef PACIFIC
const char * stristr(const char * s1, const char * s2);

#endif

#define strend(str) ((str) + strlen(str) - 1)

#ifdef UNIX
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#endif // ifndef __STREXTRA_H__
