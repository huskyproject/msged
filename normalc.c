/*
 *  NORMALC.C
 *
 *  Extracted from READMAIL.C by Paul Edwards.
 *  Released to the public domain.
 */

#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "misc.h"
#include "normal.h"

void normalize(char *s)
{
    char *tmp = s;

    while (*s)
    {
        if (stripSoft && ((unsigned char)*s == 0x8d) && !softcrxlat)
        {
            s++;
        }
        else if (*s == 0x0a)
        {
            s++;
        }
        else if (*s == 0x0d)
        {
            s++, *tmp++ = '\n';
        }
        else
        {
            *tmp++ = (char)DOROT13((int)*s);
            s++;
        }
    }
    *tmp = '\0';
}
