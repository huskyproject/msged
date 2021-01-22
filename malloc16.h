/*
 *  MALLOC16.H
 *
 *  Written on 13-Apr-98 by Tobias Ernst and released to the public domain.
 *
 *  This file provides the malloc16() and free16() functions. These functions
 *  allocate a memory block that is guaranteed to NOT cross a 64K boundary.
 *
 *  Note that a pointer that has been alloc'ed with malloc16() MUST be
 *  freed by free16(). Never pass it to free(), and never pass a pointer
 *  that has been alloc'ed with malloc() to free16()!!!
 *
 *  See the commentary in malloc16.c on a instruction when and why you
 *  need these functions.
 *
 */

#ifndef __MALLOC16_H
#define __MALLOC16_h

#include <stdlib.h>

void * malloc16(size_t);
void free16(void *);

#endif
