/*
 *  DOSMISC.C
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  Miscellaneous DOS stuff.
 */

#ifdef HASCORE1

#include <alloc.h>

long corerem(void)
{
    return coreleft();
}

#else

#include "dosasm.h"

long corerem(void)
{
    return (long)dosavmem() * 16L;
}

#endif
