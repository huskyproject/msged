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

int normalize(char * s) /* int: returns new string length */
{
    char * tmp = s;
    char * org = s;

    while(*s)
    {
        switch((unsigned char)(*s))
        {
            case 0x0a:
                s++;
                break;

            case 0x0d:
                s++;
                *tmp++ = '\n';
                break;

            case 0x8d:

                if(stripSoft && !softcrxlat)
                {
                    s++;
                    break;
                }

            default:
                *tmp++ = (char)DOROT13((int)*s);
                s++;
        }
    }
    *tmp = '\0';
    return tmp - org;
} /* normalize */
