/*
 *  SCREEN.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  If you need to port this, remember, Msged uses a 1-based coordinate
 *  system (ie. top left is (1,1) NOT (0,0)).
 */

#include <stdio.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "winsys.h"
#include "screen.h"

static int autostart;
static unsigned int *macro;

unsigned int KeyHit(void)
{
    if (macro)
    {
        return 0;
    }

    if (TTPeekQue())
    {
        return 1;
    }

    return 0;
}

void cursor(char state)
{
    TTCurSet(state);
}

unsigned int GetKey(void)
{
    int ch;

    if (macros[0] != NULL && !autostart)
    {
        autostart = 1;
        macro = macros[0];
    }

    if (macro != NULL)
    {
        macro++;
        if (*macro)
        {
            return *macro;
        }
        macro = NULL;
    }

    ch = TTGetChr();

    if (ch >= 0x3b && ch <= 0x44)
    {
        macro = macros[ch - 0x3a];
    }
    else
    {
        if (ch >= 0x54 && ch <= 0x71)
        {
            macro = macros[ch - 0x49];
        }
    }

    if (macro != NULL)
    {
        if (*macro)
        {
            return *macro;
        }

        macro = NULL;
    }
    return (unsigned int)ch;
}

unsigned int ConvertKey(int ch)
{
    int idx = ch >> 8;

    if (ch == 0)
    {
        if (macros[0] != NULL && !autostart)
        {
            autostart = 1;
            macro = macros[0];
        }

        if (macro != NULL)
        {
            macro++;
            if (*macro)
            {
                return *macro;
            }
            macro = NULL;
        }
        return 0;
    }

    if (idx >= 0x3b && idx <= 0x44)
    {
        macro = macros[idx - 0x3a];
    }
    else
    {
        if (idx >= 0x54 && idx <= 0x71)
        {
            macro = macros[idx - 0x49];
        }
    }

    if (macro != NULL)
    {
        if (*macro)
        {
            return *macro;
        }
        macro = NULL;
    }
    return (unsigned int)ch;
}

