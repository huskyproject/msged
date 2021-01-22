/*
 *  STREXTRA.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis,
 *  Paul Edwards and Andrew Clarke.  Released to the public domain.
 *
 *  A few string handling routines for Msged.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "strextra.h"

void strdel(char * l, int x)
{
    int i;

    i = strlen(l);

    if(x > i)
    {
        return;
    }

    x--;
    memmove(l + x, l + x + 1, i - x + 1);
    *(l + i) = 0;
}

/* strncmpi(...) -> strncmp(...), ignore case */
int strncmpi(const char * s, const char * t, size_t x)
{
    long n;

    n = (long)x;

    while(n-- && tolower(*s) == tolower(*t))
    {
        if(*s == '\0')
        {
            /* equal */
            return 0;
        }

        s++;
        t++;
    }

    if(n < 0)
    {
        /* maximum hit, equal */
        return 0;
    }

    /* fell through, not equal */
    if(tolower(*s) > tolower(*t))
    {
        return 1;
    }
    else
    {
        return -1;
    }
} /* strncmpi */

#if !defined (MSC) && !defined (UNIX) && !defined (_MSC_VER)

int stricmp(const char * s, const char * t)
{
    while(*s != '\0')
    {
        int rc;
        rc = tolower((unsigned char)*s) - tolower((unsigned char)*t);

        if(rc != 0)
        {
            return rc;
        }

        s++;
        t++;
    }

    if(*t != '\0')
    {
        return -tolower((unsigned char)*t);
    }

    return 0;
}

#endif /* if !defined (MSC) && !defined (UNIX) && !defined (_MSC_VER) */

#ifndef __IBMC__

#if !(defined (_MSC_VER) && (_MSC_VER >= 1200)) && !defined (UNIX)
char * strdup(const char * s)
{
    char * p;

    p = malloc(strlen(s) + 1);

    if(p != NULL)
    {
        strcpy(p, s);
    }

    return p;
}

#endif
#endif

#if !(defined (_MSC_VER) && (_MSC_VER >= 1200))

#ifndef __IBMC__
int memicmp(const void * s1, const void * s2, size_t n)
#else
int memicmp(void * s1, void * s2, size_t n)
#endif
{
    size_t x;
    int ret;

    for(x = 0; x < n; x++)
    {
        ret = (tolower((unsigned char)*(char *)s1) - tolower((unsigned char)*(char *)s2));

        if(ret != 0)
        {
            return ret;
        }
    }
    return 0;
}

char * strlwr(char * s)
{
    char * p;

    p = s;

    while(*p != '\0')
    {
        *p = (char)tolower((unsigned char)*p);
        p++;
    }
    return s;
}

char * strupr(char * s)
{
    char * p;

    p = s;

    while(*p != '\0')
    {
        *p = (char)toupper((unsigned char)*p);
        p++;
    }
    return s;
}

#endif /* if !(defined (_MSC_VER) && (_MSC_VER >= 1200)) */

#if !defined (PACIFIC) && !defined (__HIGHC__)

const char * stristr(const char * s1, const char * s2)
{
    const char * pptr, * sptr, * start;
    size_t slen, plen;

    start = s1;
    slen  = strlen(s1);
    plen  = strlen(s2);

    /* while string length not shorter than pattern length */
    while(slen >= plen)
    {
        /* find start of pattern in string */
        while(toupper(*start) != toupper(*s2))
        {
            start++;
            slen--;

            /* if pattern longer than string */
            if(slen < plen)
            {
                return NULL;
            }
        }
        sptr = start;
        pptr = s2;

        while(toupper(*sptr) == toupper(*pptr))
        {
            sptr++;
            pptr++;

            /* if end of pattern then pattern was found */
            if(*pptr == '\0')
            {
                return start;
            }
        }
        start++;
        slen--;
    }
    return NULL;
} /* stristr */

#endif /* if !defined (PACIFIC) && !defined (__HIGHC__) */
