/*
 *  KEYCODE.C
 *
 *  Written by either jim nutt or John Dennis.  Changes by Paul Edwards
 *  and Andrew Clarke.  Released to the public domain.
 *
 *  Displays keyboard scan codes in hexadecimal form for Msged users.
 */

#include <stdio.h>
#include "winsys.h"

void keycode(void)
{
    unsigned int ch;

    TTopen();
    MouseOFF();

    printf("Displaying keyboard scan codes in hexadecimal form.\n\n");
    printf("Press any key or key combination, or 'q' (lowercase 'Q') to exit.\n");
    fflush(stdout);

    do
    {
        ch = TTGetKey();
        printf("Key: ");
        if ((ch >> 8) == 0)
        {
            printf("0x%04x (%c)", ch, ch);
        }
        else
        {
            printf("0x%04x", ch);
        }
        printf("\n");
        fflush(stdout);
    }
    while (ch != 'q');
    TTclose();
}
