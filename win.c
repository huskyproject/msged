/*
 *  WINDOW.C
 *
 *  Written by John Dennis and released to the public domain on 10-Jul-94.
 *
 *  This module contains routines that maintain text windows on the
 *  screen.  Output routines are integrated in relation to the window
 *  coordinates on the screen.  All coordinates have a 0 based origin!
 *  Please note this!
 *
 *  This is the second level of abstraction from the physical device; it
 *  should not be necessary to modify this module when porting to other
 *  operating systems/devices.
 *
 *  History:
 *
 *  10-Nov-91 JD Started.
 *  25-Jul-92 JD Whole package (of which this is a module) was finished
 *               to a useable degree.
 *  13-Aug-92 JD Functions added during the Msged port to this system.
 *               Should increase the packages' useability.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "specch.h"
#include "memextra.h"
#include "keys.h"
#include "winsys.h"
#include "mcompile.h"

#define XMOD(w) ((w->flags & INSBDR) ? 3 : ((w->flags & NBDR) ? 0 : 1))
#define YMOD(w) ((w->flags & INSBDR) ? 2 : ((w->flags & NBDR) ? 0 : 1))

/* double and single border chars */
unsigned char Dbdr[6] = {SC1, SC2, SC3, SC4, SC5, SC6};
unsigned char Sbdr[6] = {SC7, SC8, SC9, SC10, SC11, SC12};

int wnd_bs_127 = 0;           /* Is ASCII 127 backspace on ANSI console ? */
int wnd_suppress_shadows = 0; /* do not suppress window shadows */
#ifdef UNIX
int wnd_force_monochrome = 1;
#else
int wnd_force_monochrome = 0; /* do not enforce monochrome output */
#endif

int FillChr = SC13;           /* fill char for fields */
unsigned long wndid = 20;     /* unique window ID */
WND *CW = NULL;               /* current window */

static void WDrwBox(int x1, int y1, int x2, int y2, unsigned char *Bdrc, int Battr, int ins);

static char line[255];

int CheckMousePos(int x1, int y1, int x2, int y2)
{
    int x, y;
    GetMouInfo(&x, &y);
    if (x >= x1 && x <= x2)
    {
        if (y >= y1 && y <= y2)
        {
            return 1;
        }
    }
    return 0;
}

/*
 *  WndOpen; Creates and opens a window returning the handle to
 *  the user, using 0 based coordinates on the screen.
 */

WND *WndOpen(int x1, int y1, int x2, int y2, int Bdr, int BAttr, int Attr)
{
    WND *w;
    unsigned short ch;
    int i, k = 0;
    static int first_win = 1;

    w = xmalloc(sizeof *w);

    if (w == NULL)
    {
        return NULL;
    }

    if (wnd_suppress_shadows)
    {
        Bdr = Bdr & (~SHADOW);
    }

    /* sanity checks*/
    if (x1 < 0)  x1 = 0;
    if (y1 < 0)  y1 = 0;
    if (x2 < x1) x2 = x1;
    if (y2 < y1) y2 = y1;

    w->wid = ++wndid;
    w->x1 = (unsigned char)x1;
    w->x2 = (unsigned char)x2;
    w->y1 = (unsigned char)y1;
    w->y2 = (unsigned char)y2;
    w->wattr = (unsigned char)Attr;
    w->battr = (unsigned char)BAttr;
    w->flags = (unsigned char)Bdr;
    w->title = NULL;



    if (Bdr & SHADOW)
    {
        x2 += 2;
        y2++;
    }

    MouseOFF();

    /* get a copy of the background */

    if ((!(Bdr & NOSAVE)) || (Bdr & SHADOW))
    {
        w->buffer = xmalloc(sizeof(unsigned short *) * ((y2 - y1) + 2));

        if (!w->buffer)
        {
            return NULL;
        }

        for (i = y1; i <= y2; i++)
        {
            w->buffer[k] = xmalloc(sizeof(unsigned short) * ((x2 - x1) + 2));
            if (!w->buffer[k])
            {
                return NULL;
            }
            TTReadStr(w->buffer[k], x2 - x1 + 1, i, x1);
            k++;
        }
    }
    else
    {
        w->buffer = NULL;
    }

    /* put out shadow */

    if (Bdr & SHADOW)
    {
        k = 1;
        for (i = y1 + 1; i <= y2; i++)
        {
            ch = (unsigned short)w->buffer[k][x2 - x1];
            ch &= 0x00FF;
            ch |= ((unsigned short)(DGREY | _BLACK) << 8);
            TTWriteStr(&ch, 1, i, x2);
            ch = (unsigned short)w->buffer[k][x2 - x1 - 1];
            ch &= 0x00FF;
            ch |= ((unsigned short)(DGREY | _BLACK) << 8);
            TTWriteStr(&ch, 1, i, x2 - 1);
            k++;
        }
        k = y2 - y1;
        for (i = 2; i <= (x2 - x1); i++)
        {
            ch = (unsigned short)w->buffer[k][i];
            ch &= 0x00FF;
            ch |= ((unsigned short)(DGREY | _BLACK) << 8);
            TTWriteStr(&ch, 1, y2, i + x1);
        }
    }
    TTScolor(Attr);
    if (!first_win)
    {
        TTClear(w->x1, w->y1, w->x2, w->y2);
    }
    else
    {
        first_win = 0;
    }

    if (!(Bdr & NBDR))
    {
        if (Bdr & SBDR)
        {
            WDrwBox(w->x1, w->y1, w->x2, w->y2, Sbdr, BAttr, Bdr & INSBDR ? 1 : 0);
        }
        else
        {
            WDrwBox(w->x1, w->y1, w->x2, w->y2, Dbdr, BAttr, Bdr & INSBDR ? 1 : 0);
        }
    }

    MouseON();

    CW = w;

    return CW;
}

/*
 *  WndPopUp; Opens up a window of the passed size in the middle of the
 *  screen.
 */

WND *WndPopUp(int wid, int dep, int Bdr, int BAttr, int NAttr)
{
    int x1, x2, y1, y2;
    x1 = max((term.NCol / 2) - (wid / 2) - 1, 0);
    y1 = max((term.NRow / 2) - (dep / 2) - 1, 0);
    x2 = min(x1 + wid, term.NCol - 1);
    y2 = min(y1 + dep, term.NRow - 1);
    return WndOpen(x1, y1, x2, y2, Bdr, BAttr, NAttr);
}

/*
 *  WndClose; Closes the passed window and restores the screen
 *  behind it.
 *
 *  Caveats: Since no track is kept of the windows on the screen, it
 *  is the caller's responsibility to ensure that windows are closed
 *  in the right order.
 */

void WndClose(WND * w)
{
    WND *wnd;
    int i, k = 0;
    int x2, y2;

    /* restore screen buffer */

    if (w == NULL)
    {
        wnd = CW;
    }
    else
    {
        wnd = w;
    }

    if (wnd == NULL)
    {
        return;
    }

    x2 = wnd->x2;
    y2 = wnd->y2;

    if (wnd->flags & SHADOW)
    {
        x2 += 2;
        y2++;
    }

    MouseOFF();

    if (!(wnd->flags & NOSAVE))
    {
        for (i = wnd->y1; i <= y2; i++)
        {
            TTWriteStr(wnd->buffer[k], x2 - wnd->x1 + 1, i, wnd->x1);
            xfree(wnd->buffer[k++]);
        }
        xfree(wnd->buffer);
    }
    xfree(wnd);
    MouseON();
}

/*
 *  WndDrwBox; Draws a box at the specified position. Internal function.
 */

static void WDrwBox(int x1, int y1, int x2, int y2, unsigned char *Bdrc, int Battr, int ins)
{
    unsigned int i, width;
    unsigned short cell, *pcell, *p;
    int xmod = ins ? 2 : 0;
    int ymod = ins ? 1 : 0;

    MouseOFF();

    /* write corner chars */

    cell = (unsigned short)Bdrc[2] | (unsigned short)(Battr << 8);
    TTWriteStr(&cell, 1, y1 + ymod, x1 + xmod);

    cell = (unsigned short)Bdrc[3] | (unsigned short)(Battr << 8);
    TTWriteStr(&cell, 1, y1 + ymod, x2 - xmod);

    cell = (unsigned short)Bdrc[4] | (unsigned short)(Battr << 8);
    TTWriteStr(&cell, 1, y2 - ymod, x1 + xmod);

    cell = (unsigned short)Bdrc[5] | (unsigned short)(Battr << 8);
    TTWriteStr(&cell, 1, y2 - ymod, x2 - xmod);

    /* write top & bottom horiziontal border chars */

    width = (x2 - xmod) - (x1 + 1 + xmod);
    pcell = xmalloc(width * sizeof *pcell);
    p = pcell;
    for (i = 0; i < width; i++)
    {
        *p = (unsigned short)Bdrc[1] | (unsigned short)(Battr << 8);
        p++;
    }
    TTWriteStr(pcell, width, y1 + ymod, x1 + 1 + xmod);
    TTWriteStr(pcell, width, y2 - ymod, x1 + 1 + xmod);
    xfree(pcell);

    /* write left & right vertical border chars */

    cell = (unsigned short)Bdrc[0] | (unsigned short)(Battr << 8);
    for (i = y1 + 1 + ymod; i < y2 - ymod; i++)
    {
        TTWriteStr(&cell, 1, i, x1 + xmod);
        TTWriteStr(&cell, 1, i, x2 - xmod);
    }

    MouseON();
}

/*
 *  WndBox; Draws a box in the window.
 */

void WndBox(int x1, int y1, int x2, int y2, int Attr, int type)
{
    int xmod, ymod;
    unsigned char *Bdrc = (type & DBDR) ? Dbdr : Sbdr;

    if (CW == NULL)
    {
        return;
    }

    ymod = YMOD(CW);
    xmod = XMOD(CW);

    if (y1 < 0 || x1 < 0 || CW->x1 + x2 + xmod > CW->x2 || CW->y1 + y2 + ymod > CW->y2)
    {
        return;
    }

    WDrwBox(x1 + CW->x1 + xmod, y1 + CW->y1 + ymod, x2 + CW->x1 + xmod, y2 + CW->y1 + ymod, Bdrc, Attr, 0);
}

/*
 *  WndGetRel; Converts absolute coordinates to coordinates relative
 *  to the current window.
 */

void WndGetRel(int x, int y, int *wx, int *wy)
{
    int xmod, ymod;

    if (CW == NULL)
    {
        return;
    }

    ymod = YMOD(CW);
    xmod = XMOD(CW);

    *wx = x - (CW->x1 + xmod);
    *wy = y - (CW->y1 + ymod);
}

int WndWidth(void)
{
    if (CW == NULL)
    {
        return 0;
    }
    return CW->x2 - CW->x1 - (XMOD(CW) * 2);
}

/*
 *  WndTitle; Writes a title to centre of the *current* window,
 *  clearing whatever was there initially.
 *
 *  Caveats: Doesn't check to see if there is a border, only for the
 *  type of border.
 */

void WndTitle(const char *title, int Attr)
{
    int cntr;
    int pos, i, len;
    unsigned short ch;
    int ymod;
    unsigned char *Bdrc = Sbdr;
    int m = 0;

    if (CW == NULL)
    {
        return;
    }

    if (title == NULL)
    {
        title = "<-- null pointer passed -->";
    }
    cntr = (CW->x2 - CW->x1 + 1) / 2;
    len = strlen(title);

    /* 'cause were writing on the border */
    if (CW->flags & INSBDR)
    {
        ymod = 1;
    }
    else
    {
        ymod = 0;
    }

    if (CheckMousePos(CW->x1, CW->y1, CW->x2, CW->y2))
    {
        m = 1;
        MouseOFF();
    }

    if (CW->title)
    {
        if (!(CW->flags & NBDR))
        {
            if (CW->flags & SBDR)
            {
                Bdrc = Sbdr;
            }
            else
            {
                Bdrc = Dbdr;
            }
        }

        ch = (unsigned short)Bdrc[1] | (unsigned short)(CW->battr << 8);
        for (i = CW->x1 + 1; i < CW->x2; i++)
        {
            TTWriteStr(&ch, 1, CW->y1 + ymod, i);
        }

        xfree(CW->title);
    }

    pos = (cntr - (len / 2)) + CW->x1;
    CW->title = xstrdup(title);

    TTScolor(Attr);
    TTStrWr((unsigned char *)title, CW->y1 + ymod, pos);

    if (m)
    {
        MouseON();
    }
}

/*
 *  WndWriteStr; Writes a string to the (current) window, using a
 *  zero-based origin.
 *
 *  Caveats: If string would write past end of line, it won't write
 *  the string.
 */

void WndWriteStr(int x, int y, int Attr, char *Str)
{
    int row, col, len;
    int m = 0;
    char *trunc, ch = 0;
    char *PrintStr = Str;

    if (Str == NULL)
    {
        return;
    }

    len = strlen(Str);


    if (CW == NULL)
    {
        return;
    }

    /* row and col must be zero-based */
    row = CW->y1 + y;
    col = CW->x1 + x;

    if (x < 0 || y < 0)
    {
        return;
    }

    if (!(CW->flags & NBDR))
    {
        /* check end-of-window overstep */
        row += YMOD(CW);
        col += XMOD(CW);
        if (row >= CW->y2 || col > CW->x2 - XMOD(CW))
        {
            return;
        }
    }
    else
    {
        if (row > CW->y2 || col > CW->x2)
        {
            return;
        }
    }

    if (CheckMousePos(col, row, col + len - 1, row))
    {
        m = 1;
        MouseOFF();
    }

    trunc = NULL;
    if ((col + len - 1) > CW->x2 - XMOD(CW))
    {
        PrintStr = xstrdup(Str);
        trunc = PrintStr + ((CW->x2 - XMOD(CW)) - col + 1);
        ch = *trunc;
        *trunc = '\0';
    }

    TTScolor(Attr);
    TTStrWr((unsigned char *)PrintStr, row, col);

    if (PrintStr != Str)
    {
        xfree(PrintStr);
    }

    if (m)
    {
        MouseON();
    }
}

/*
 *  WndPutsCen; Puts a string in the center of the window at the passed
 *  line.
 */

void WndPutsCen(int y, int attr, char *str)
{
    int cntr;
    int col, len;
    int mod;

    if (CW == NULL)
    {
        return;
    }

    if (str == NULL)
    {
        return;
    }

    if (y < 0)
    {
        return;
    }

    cntr = (CW->x2 - CW->x1 + 1) / 2;
    mod = XMOD(CW);
    len = strlen(str);


    if (len > WndWidth())
    {
        col = 0;
    }
    else
    {
        col = cntr - (len / 2) - mod;
    }
    WndWriteStr(col, y, attr, str);
}

/*
 *  WndPrintf; Operates the same as printf(), except it does it to the
 *  current window, using the passed parameters.
 */

int WndPrintf(int x, int y, int attr, char *str, ...)
{
    int rc;
    va_list params;
    va_start(params, str);
    rc = vsprintf(line, str, params);
    WndWriteStr(x, y, attr, line);
    return rc;
}

/*
 *  WndPutsn; Puts len of str onto the current window.
 */

void WndPutsn(int x, int y, int len, int attr, char *str)
{
    char *s = str, *c = line;
    int i = 0;

    if (x < 0 || y < 0 || len < 1)
    {
        return;
    }
    if (len > 254)
    {
        len = 254;
    }
    if (s != NULL)
    {
        i = strlen(s);
        if (i > len)
        {
            i = len;
        }
        memcpy(c, s, i);
    }
    if (i < len)
    {
        memset(c + i, ' ', len - i);
    }
    c[len] = '\0';
    WndWriteStr(x, y, attr, line);
}

/*
 *  WndScroll; Scrolls a window at the specified cooridiates using a
 *  zero-based origin.
 *
 */

void WndScroll(int x1, int y1, int x2, int y2, int dir)
{
    int xmod, ymod;
    int m = 0;

    if (CW == NULL)
    {
        return;
    }

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 < x1 || y2 < y1) return;

    xmod = XMOD(CW);
    ymod = YMOD(CW);

    if (CheckMousePos(CW->x1 + x1 + xmod, CW->y1 + y1 + ymod, CW->x2 + x2 + xmod, CW->y2 + y2 + ymod))
    {
        m = 1;
        MouseOFF();
    }
    TTScolor(CW->wattr);
    TTScroll(CW->x1 + x1 + xmod, CW->y1 + y1 + ymod, CW->x1 + x2 + xmod, CW->y1 + y2 + ymod, 1, dir);
    if (m)
    {
        MouseON();
    }
}

/*
 *  WndCurr; Makes the passed window the current window.
 *
 *  Caveats: The windowing system doesn't keep track of the windows
 *  that exist - the caller must do that.
 */

void WndCurr(WND * hWnd)
{
    if (CW == hWnd || hWnd == NULL)
    {
        return;
    }
    CW = hWnd;
}

/*
 *  WndTop; Returns the current window recognized by the windowing system.
 */

WND *WndTop(void)
{
    if (CW != NULL)
    {
        return CW;
    }
    else
    {
        return NULL;
    }
}

/*
 *  WndPutc; Puts a char anywhere on the window, ignoring borders, etc.
 *  Puts it at the current cursor position (virtual or real).
 *
 *  Caveats: The cursor may not even be on the window, but it doesn't
 *  care.  :-)
 */

void WndPutc(int Ch, int attr)
{
    if (CW == NULL)
    {
        return;
    }
    MouseOFF();
    TTScolor(attr);
    TTPutChr(Ch);
    MouseON();
}

/*
 *  WndGotoXY; Goes to the passed coordinates, zero-based, starting
 *  from any borders on the window.
 *
 *  Caveats: No checks for validity of arguments.
 */

void WndGotoXY(int x, int y)
{
    int xmod, ymod;

    if (CW == NULL)
    {
        return;
    }

    xmod = XMOD(CW);
    ymod = YMOD(CW);

    TTgotoxy(CW->y1 + y + ymod, CW->x1 + x + xmod);
}

/*
 *  WndClear; Clears the passed coordinates with the passed attribute
 *  in the current window.
 *
 *  Caveats: No checks for validity of arguments.
 */

void WndClear(int x1, int y1, int x2, int y2, int attr)
{
    int xmod, ymod, m = 0;

    if (CW == NULL)
    {
        return;
    }

    xmod = XMOD(CW);
    ymod = YMOD(CW);

    if (CheckMousePos(CW->x1 + x1 + xmod, CW->y1 + y1 + ymod, CW->x2 + x2 + xmod, CW->y2 + y2 + ymod))
    {
        m = 1;
        MouseOFF();
    }
    TTScolor(attr);
    TTClear(CW->x1 + x1 + xmod, CW->y1 + y1 + ymod, CW->x1 + x2 + xmod, CW->y1 + y2 + ymod);
    if (m)
    {
        MouseON();
    }
}

/*
 *  WndClearLine; Clears the passed line in the current window.
 */

void WndClearLine(int y, int Att)
{
    if (CW == NULL)
    {
        return;
    }
    WndClear(0, y, CW->x2 - CW->x1 - (XMOD(CW) * 2), y, Att);
}

/*
 *  WndFillField; Fills in an edit-field region with char.
 */

void WndFillField(int x, int y, int len, unsigned char ch, int Attr)
{
    if (len > sizeof(line) - 1)
    {
        len = sizeof(line) - 1;
    }
    memset(line, ch, len);
    *(line + len - 1) = '\0';
    WndWriteStr(x, y, Attr, line);
}

/*
 *  WndGetLine; Gets a string from the user on the current window at
 *  the passed coordinates.  Passes mouse events not on itself back to
 *  the caller via the extra parameters.
 */

int WndGetLine(int x, int y, int len, char *buf, int Attr, int *pos, int nokeys, int fil, int disp, EVT * ev)
{
    EVT event;
    int done = 0;
    int row = y, col = x;
    int xmod, ymod, i;
    unsigned int ch = 0;
    unsigned char fill = (fil) ? (unsigned char)FillChr : (unsigned char) ' ';

    if (CW == NULL)
    {
        return 0;
    }

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    if (disp)
    {
        WndFillField(col, row, len + 1, fill, Attr);
        WndPutsn(col, row, len, Attr, buf);
    }

    xmod = XMOD(CW);
    ymod = YMOD(CW);

    i = *pos;

    TTCurSet(1);

    while (!done)
    {
        WndGotoXY(i + col, row);
        ch = MnuGetMsg(&event, 0);
        switch (event.msgtype)
        {
        case WND_WM_MOUSE:
        case WND_WM_COMMAND:
            {
                int p1 = event.x, p2 = event.y;
                if (p1 < CW->x1 + xmod + x || p1 > CW->x1 + xmod + x + len - 1 || p2 != CW->y1 + ymod + y)
                {
                    done = TRUE;
                    break;
                }
            }
            break;

        case WND_WM_CHAR:
            switch (ch)
            {
            case Key_Up:
            case Key_Dwn:
                done = TRUE;
                break;

            case Key_Lft:
                if (i > 0)
                {
                    i--;
                }
                break;

            case Key_Rgt:
                if (i < strlen(buf))
                {
                    i++;
                }
                break;

            case Key_A_X:
                memset(buf, 0, len);
                i = 0;
                break;

            case Key_BS:
                if (i > 0)
                {
                    if (i < strlen(buf))
                    {
                        memmove(buf + i - 1, buf + i, strlen(buf + i) + 1);
                        i--;
                        WndWriteStr(col + i, row, Attr, buf + i);
                        WndPrintf(col + strlen(buf), row, Attr, "%c", fill);
                    }
                    else
                    {
                        i--;
                        *(buf + i) = '\0';
                        WndGotoXY(i + col, row);
                        WndPutc(fill, Attr);
                    }
                }
                break;

            case Key_Del:
                if (i < strlen(buf))
                {
                    memmove(buf + i, buf + i + 1, strlen(buf + i + 1) + 1);
                    WndWriteStr(col + i, row, Attr, buf + i);
                    WndPrintf(col + strlen(buf), row, Attr, "%c", fill);
                }
                break;

            case Key_End:
                i = strlen(buf);
                break;

            case Key_Home:
                i = 0;
                break;

            case Key_Ent:
                done = 1;
                break;

            case Key_Esc:
                done = 2;
                break;

            default:
                if (ch > 0 && i < len && strlen(buf) < len)
                {
                    if (nokeys)
                    {
                        strcpy(buf, "");
                        WndFillField(col, row, len + 1, fill, Attr);
                        i = 0;
                        WndGotoXY(i + col, row);
                    }
                    if (i < strlen(buf))
                    {
                        memmove(buf + i + 1, buf + i, strlen(buf + i) + 1);
                        *(buf + i) = (char)ch;
                        WndWriteStr(col + i, row, Attr, buf + i);
                        i++;
                    }
                    else
                    {
                        *(buf + i) = (char)ch;
                        *(buf + i + 1) = '\0';
                        WndPutc((unsigned char)ch, Attr);
                        i++;
                    }
                }
                else
                {
                    done = 1;
                }
                break;
            }
            break;
        }
        nokeys = 0;
    }
    TTCurSet(0);

    if (ev != NULL)
    {
        *ev = event;
    }

    *pos = i;
    return (int)ch;
}
