/*
 *  MEMEXTRA.H
 *
 *  Written 1995-1996 by Andrew Clarke and released to the public domain.
 *
 *  Memory allocation routines with core exhaust checking.
 */

#ifndef __MEMEXTRA_H__
#define __MEMEXTRA_H__

#include <stdlib.h>  /* size_t */

void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *str);
void xfree(void *ptr);

#ifdef OS2
void xfree16(void *ptr);
void* xmalloc16(size_t size);
#endif

#endif
