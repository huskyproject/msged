/*
 *  MEMEXTRA.C
 *
 *  Written 1995-1996 by Andrew Clarke and released to the public domain.
 *
 *  Memory allocation routines with core exhaust checking.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "memextra.h"

static char msg_alloc_fail[] =
  "*** Memory allocation failure (out of memory)\n"
  "*** Needed %u (%Xh) bytes.\n";

static char msg_realloc_fail[] =
  "*** Memory reallocation failure (out of memory)\n"
  "*** Needed %u (%Xh) bytes.\n";

static char msg_free_fail[] =
"*** Memory deallocation failure (attempted to free null pointer)\n";

void *xmalloc(size_t size)
{
    void *ptr;
    ptr = malloc(size);
    if (ptr == NULL)
    {
        cleanup(msg_alloc_fail, (unsigned)size, (unsigned)size);
        exit(0);
    }
    return ptr;
}

void *xcalloc(size_t nmemb, size_t size)
{
    void *ptr;
    ptr = calloc(nmemb, size);
    if (ptr == NULL)
    {
        cleanup(msg_alloc_fail, (unsigned)(nmemb * size), (unsigned)(nmemb * size));
        exit(0);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
    if (ptr == NULL)
    {
        return xmalloc(size);
    }
    if (size == (size_t) 0)
    {
        xfree(ptr);
        return NULL;
    }
    ptr = realloc(ptr, size);
    if (ptr == NULL)
    {
        cleanup(msg_realloc_fail, (unsigned)size, (unsigned)size);
        exit(0);
    }
    return ptr;
}

char *xstrdup(const char *str)
{
    return strcpy(xmalloc(strlen(str) + 1), str);
}

void xfree(void *ptr)
{
    if (ptr == NULL)
    {
        cleanup(msg_free_fail);
        exit(0);
    }
    else
    {
        free(ptr);
    }
}

#ifdef OS2
#include "malloc16.h"

void xfree16(void *ptr)
{
    if (ptr == NULL)
    {
        cleanup(msg_free_fail);
        exit(0);
    }
    else
    {
        free16(ptr);
    }
}

void *xmalloc16(size_t size)
{
    void *ptr;
    ptr = malloc16(size);
    if (ptr == NULL)
    {
        cleanup(msg_alloc_fail, (unsigned)size, (unsigned)size);
        exit(0);
    }
    return ptr;
}
#endif
