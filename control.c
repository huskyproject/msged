/*
 *  CONTROL.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  The display routines for controls of various different types.
 */

#include <stdio.h>
#include <string.h>
#include "winsys.h"
#include "menu.h"
#include "specch.h"

static char text[255];

/*
 *  Makes up a button.
 */

char *MakeButton(button * b, int sel)
{
    if (sel)
    {
        sprintf(text, "%c %s %c", SC14, b->btext, SC15);
    }
    else
    {
        sprintf(text, "  %s  ", b->btext);
    }
    return text;
}

/*
 *  Puts a button onto the screen, handling all the states of the button.
 */

void ShowButton(button * b)
{
    char *s;
    static unsigned char ublock[2] = {'\0', '\0'};
    int len;
    unsigned char attr;

    s = MakeButton(b, b->select);
    len = strlen(s);
    if (b->select)
    {
        attr = (unsigned char)b->sattr;
    }
    else
    {
        attr = (unsigned char)b->fattr;
    }

    if (!b->down)
    {
        WndPutsn(b->x, b->y, 1, attr | F_ALTERNATE, s);
        WndPutsn(b->x + 1, b->y, len - 2, attr, s + 1);
        WndPutsn(b->x + len - 1, b->y, 1, attr | F_ALTERNATE, s + len - 1);
        ublock[0] = SC16;
        WndWriteStr(b->x + len, b->y, b->battr | F_ALTERNATE, (char *)ublock);

        memset(text, SC17, sizeof(text));
        *(text + len) = '\0';
        WndPutsn(b->x + 1, b->y + 1, len, b->battr | F_ALTERNATE, text);

    }
    else
    {
        WndPutsn(b->x, b->y, 1, attr | F_ALTERNATE, s);
        WndPutsn(b->x + 1, b->y, len - 2, attr, s + 1);
        WndPutsn(b->x + len - 1, b->y, 1, attr | F_ALTERNATE, s + len - 1);
        WndWriteStr(b->x, b->y, b->battr, " ");

        memset(text, ' ', sizeof(text));
        *(text + len) = '\0';

        WndPutsn(b->x + 1, b->y + 1, len, b->battr, text);
    }
    return;
}

/*
 *  Puts a checkbox button onto the screen, handling all the states of
 *  the check box.
 */

void ShowCkbutton(ckbutton * i)
{
    int fa = (i->select) ? i->sattr : i->fattr;

    if (i->down)
    {
        strcpy(text, " [x] ");
    }
    else
    {
        strcpy(text, " [ ] ");
    }
    WndWriteStr(i->x, i->y, fa, text);
    WndWriteStr(i->px, i->y, fa, i->prtext);
}

/*
 *  Displays an editf control.
 */

void ShowEditField(editf * i)
{
    if (!i->select)
    {
        WndPutsn(i->x, i->y, i->len, i->fattr, i->buf);
    }
    else
    {
        WndPutsn(i->x, i->y, i->len, i->sattr, i->buf);
    }
}

void D_ShowTxt(textl * i)
{
    if (i->text)
    {
        WndWriteStr(i->x, i->y, i->fattr, i->text);
    }
}

void D_ShowWBox(wbox * i)
{
    WndBox(i->x1, i->y1, i->x2, i->y2, i->fattr, i->type);
    if (i->title)
    {
        int x;
        x = ((i->x2 - i->x1) / 2) - (strlen(i->title) / 2) + i->x1;
        WndWriteStr(x, i->y1, i->tattr, i->title);
    }
}
