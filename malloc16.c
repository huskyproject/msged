/*
 *  MALLOC16.C
 *
 *  Written on 13-Apr-98 by Tobias Ernst and released to the public domain.
 *
 *  This file provides the malloc16() and free16() functions. These functions
 *  allocate a memory block that is guaranteed to NOT cross a 64K boundary
 *  and to reside below the magic 512 MB boundary on OS/2.
 *
 *  Note that a pointer that has been alloc'ed with malloc16() MUST be
 *  freed by free16(). Never pass it to free(), and never pass a pointer
 *  that has been alloc'ed with malloc() to free16()!!!
 *
 *  You need to allocate memory with these functions, for example, if you
 *  whish to pass a pointer on to a 16 bit OS/2 API function, like the
 *  Vio* functions that have not changed since OS/2 1.3.
 *
 *  If you pass a pointer to the Vio* functions that has been allocated
 *  with standard malloc, the code will run a thousand times without problems,
 *  but in the 1001st time (when the pointer malloc returned accidentally
 *  crosses 1 64k boundary), you will get heap corruption because the Vio
 *  function writes to the wrong memory area. It took me over a month
 *  to find out that this was the reason that my EMX compiled MsgEd crashed
 *  once a week, while it worked OK for the rest of the time ......
 *
 */

#ifdef OS2
#include "malloc16.h"
#define INCL_DOSMEMMGR
#define INCL_DOSERRORS
#include <os2.h>


#ifdef __EMX__

void *malloc16(size_t size)
{
   if (size >= 65530L)
   {
       return NULL;
   }
   return _tmalloc(size);
}

void free16(void *ptr)
{
   _tfree(ptr);
}

#else

/* Note: the non-EMX implementation is to the utmost inefficient. It is
   a quick hack. A better way would be to allocate a non-commited
   large block at program startup and then use DosSubAllocMem as needed.
   I'll implement that in the future. */


void *malloc16(size_t size)
{
    void *ptr;
    APIRET rc;

    if (size >= 65536L)       /* this is impossible */
    {
        return NULL;
    }

    rc = DosAllocMem(&ptr, size, PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);

    if (rc != NO_ERROR)
    {
        return NULL;
    }

    return ptr;
}

void free16(void *ptr)
{
    APIRET rc;

    rc = DosFreeMem (ptr);

    if (rc != NO_ERROR)
    {
        abort();
    }
}
#endif /* not EMX */

#else /* defined OS2 */
#error malloc16.c is only applicaple to the OS/2 version
#endif
