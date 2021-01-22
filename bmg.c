/*
 *  BMG.C
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  Emulate Boyer-More-Gosper routines, without actually doing it, because
 *  Msged doesn't use the proper functionality, for unknown reasons.
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "bmg.h"
#include "strextra.h"

static char search_string[256];
char * bmg_find(char * text, char * search)
{
    char * endText, * p;
    int lent, lens, searchStart;

    lent = strlen(text);
    lens = strlen(search);

    if(lens > lent)
    {
        return NULL;
    }

    searchStart = toupper(*search);
    p           = text;
    endText     = p + (lent - lens) + 1;

    while(p != endText)
    {
        if(toupper(*p) == searchStart)
        {
            if(strncmpi(p, search, lens) == 0)
            {
                return p;
            }
        }

        p++;
    }
    return NULL;
} /* bmg_find */

void bmg_setsearch(char * search)
{
    strcpy(search_string, search);
}

char * bmg_search(char * text)
{
    return bmg_find(text, search_string);
}
