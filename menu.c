/*
 *  MENU.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Moving bar menu code for Msged.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "winsys.h"
#include "unused.h"
#include "menu.h"
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "main.h"
#include "keys.h"
#include "strextra.h"
#include "areas.h"
#include "memextra.h"

void SelShowItem(char *text, int y, int len, int Attr, int indent)
{
    char line[256];
    memset(line, ' ', 40);
    strcpy(line + indent, text);
    WndPutsn(indent, y, len, Attr, line);
}

void SelShowPage(char **text, int top, int bot, int len, int pos, int Attr, int indent)
{
    int i;
    int y = top;

    for (i = pos; text[i] != NULL; i++)
    {
        if (y > bot)
        {
            break;
        }

        SelShowItem(text[i], y++, len, Attr, indent);
    }
    if (y <= bot)
    {
        while (y <= bot)
        {
            SelShowItem(" ", y++, len, Attr, indent);
        }
    }
}

void CalcDef(int max, int cur, int *top, int miny, int maxy, int *y)
{
    int dif = maxy - miny;

    unused(cur);
    if (max - 1 - *top < dif && max > dif)
    {
        *y = maxy;
        *top = (max - 1) - dif;
    }
}

int SelBox(char **Itms, int y1, int y2, int len, int def, WND * hPrev, WND * hWnd, int Sel, int Norm, int selbox_id, char *topMsg)
{
    EVT e;
    char find[30];
    int itemCnt, Stuff, done, curY, Msg, currItem, Top, page, i;

    itemCnt = 0;
    Stuff = 0;
    for (i = 0; Itms[i] != NULL; i++)
    {
        itemCnt++;
    }

    if (itemCnt < y2)
    {
        y2 = itemCnt;
    }
    currItem = def;
    curY = y1;
    page = y2 - y1;
    Top = currItem;

    if (currItem + y1 < y1)
    {
        curY = y1 + currItem;
        Top = 0;
    }
    else
    {
        if (itemCnt - currItem <= y2 - y1)
        {
            Top -= ((y2 - y1 + 1) - (itemCnt - Top));
            curY = y1 + def - Top;
            if (Top < 0)
            {
                Top = 0;
                curY--;
            }
        }
    }
    done = 0;

    SelShowPage(Itms, y1, y2, len, Top, Norm, 1);
    SelShowItem(Itms[currItem], curY, len, Sel, 1);

    /* clear input queue */
    TTClearQue();

    memset(find, '\0', sizeof find);
    while (!done)
    {
        if (*topMsg && selbox_id == SELBOX_REPLYOTH && !*find)
        {
            WndCurr(hPrev);
            WndClear(strlen(topMsg) + 3, 0, maxx - 32, 0, cm[MN_NTXT]);
            WndPrintf(0, 0, cm[MN_NTXT], ">>%s:", topMsg);
            WndCurr(hWnd);
        }
        if (!Stuff)
        {
            Msg = MnuGetMsg(&e, hWnd->wid);
        }
        else
        {
            e.msgtype = WND_WM_CHAR;
            Msg = Stuff;
            Stuff = 0;
        }

        switch (e.msgtype)
        {
        case WND_WM_MOUSE:
            switch (Msg)
            {
            case RMOU_CLCK:
            case MOU_RBTUP:
                return -1;

            case LMOU_RPT:
            case MOU_LBTDN:
            case LMOU_CLCK:
                {
                    int x, y;

                    WndGetRel(e.x, e.y, &x, &y);
                    if (y >= y1 && y <= y2)  /* in window */
                    {
                        Stuff = 0;
                        if (x >= 0 && x < len)
                        {
                            if (y == curY)
                            {
                                if (Msg == LMOU_CLCK || Msg == MOU_LBTUP)
                                {
                                    return currItem;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                            SelShowItem(Itms[currItem], curY, len, Norm, 1);

                            if (y > curY)
                            {
                                currItem += y - curY;
                            }
                            else
                            {
                                currItem -= curY - y;
                            }

                            curY = y;

                            SelShowItem(Itms[currItem], curY, len, Sel, 1);

                            if (Msg == LMOU_CLCK || Msg == MOU_LBTUP)
                            {
                                return currItem;
                            }
                        }
                    }
                    else
                    {
                        if (Msg != LMOU_CLCK)
                        {
                            if (y < y1)
                            {
                                Stuff = Key_Up;
                            }
                            else
                            {
                                Stuff = Key_Dwn;
                            }
                        }
                    }
                }
                memset(find, '\0', sizeof find);
                break;

            default:
                break;
            }
            break;

        case WND_WM_CHAR:
            switch (Msg)
            {
            case Key_Home:
                if (!currItem)
                {
                    break;
                }
                SelShowItem(Itms[currItem], curY, len, Norm, 1);
                currItem = 0;
                Top = 0;
                curY = y1;
                SelShowPage(Itms, y1, y2, len, Top, Norm, 1);
                SelShowItem(Itms[currItem], curY, len, Sel, 1);
                memset(find, '\0', sizeof find);
                break;

            case Key_End:
                if (currItem == itemCnt - 1)
                {
                    break;
                }
                SelShowItem(Itms[currItem], curY, len, Norm, 1);
                currItem = itemCnt - 1;
                while (currItem && currItem >= itemCnt - page)
                {
                    currItem--;
                }
                Top = currItem;
                currItem = itemCnt - 1;
                curY = currItem - Top + y1;
                CalcDef(itemCnt, currItem, &Top, y1, y2, &curY);
                SelShowPage(Itms, y1, y2, len, Top, Norm, 1);
                SelShowItem(Itms[currItem], curY, len, Sel, 1);
                memset(find, '\0', sizeof find);
                break;

            case Key_Dwn:
                if (currItem == itemCnt - 1)
                {
                    break;
                }

                SelShowItem(Itms[currItem], curY, len, Norm, 1);
                currItem++;
                if (curY == y2)
                {
                    WndScroll(1, y1, len, y2, 1);
                    Top++;
                }
                else
                {
                    curY++;
                }
                SelShowItem(Itms[currItem], curY, len, Sel, 1);
                memset(find, '\0', sizeof find);
                break;

            case Key_Up:
                if (!currItem)
                {
                    break;
                }
                SelShowItem(Itms[currItem], curY, len, Norm, 1);
                currItem--;
                if (curY == y1)
                {
                    WndScroll(1, y1, len, y2, 0);
                    if (Top)
                    {
                        Top--;
                    }
                }
                else
                {
                    curY--;
                }
                SelShowItem(Itms[currItem], curY, len, Sel, 1);
                memset(find, '\0', sizeof find);
                break;

            case Key_PgUp:
                if (!currItem)
                {
                    break;
                }
                SelShowItem(Itms[currItem], curY, len, Norm, 1);
                currItem -= page;
                if (currItem < 0)
                {
                    currItem = 0;
                }
                Top = currItem;
                curY = y1;
                SelShowPage(Itms, y1, y2, len, Top, Norm, 1);
                SelShowItem(Itms[currItem], curY, len, Sel, 1);
                memset(find, '\0', sizeof find);
                break;

            case Key_PgDn:
                if (currItem == itemCnt - 1)
                {
                    break;
                }
                SelShowItem(Itms[currItem], curY, len, Norm, 1);
                Top = currItem;
                currItem += page;
                if (currItem > itemCnt - 1)
                {
                    currItem = itemCnt - 1;
                }
                curY = currItem - Top + y1;
                CalcDef(itemCnt, currItem, &Top, y1, y2, &curY);
                SelShowPage(Itms, y1, y2, len, Top, Norm, 1);
                SelShowItem(Itms[currItem], curY, len, Sel, 1);
                memset(find, '\0', sizeof find);
                break;

            case Key_Rgt:
            case Key_Ent:
                {
                    size_t i;
                    i = (size_t) strchr(Itms[currItem] + 1, ' ') - 1 -
                        (size_t) Itms[currItem];
                     
                    if (i > 28)
                    {
                        i = 28;
                    }
                    strncpy(find, Itms[currItem] + 1, i);
                    *(find + i) = '\0';
                    strupr(find);
                    /* I did figure out that this code produces garbage on
                       screen.  I did not figure out however, in
                       when situations it is necessary.  So I
                       decided to comment it out and see what
                       happens ...
                       WndCurr(hPrev);
                       WndWriteStr(strlen(topMsg) + 4, 0, cm[MN_NTXT], find);
                     */
                    WndCurr(hWnd);
                    return currItem;
                }

            case Key_A_X:
            case Key_Esc:
                return -1;

            case '*':
            case Key_A_S:
                if (selbox_id == SELBOX_REPLYOTH)
                {
                    arealist_area_scan(1);
                    for (i = 0; i < SW->areas; i++)
                    {
                        xfree(alist2[i]);
                    }
                    xfree(alist2);
                    BuildList(&alist2);
                    Itms = alist2;
                    SelShowPage(Itms, y1, y2, len, Top, Norm, 1);
                    SelShowItem(Itms[currItem], curY, len, Sel, 1);
                    memset(find, '\0', sizeof find);
                    CurArea.messages = MsgAreaOpen(&CurArea);
                }
                break;

            default:
                if (Msg > 32 && Msg < 127)
                {
                    if (*topMsg && selbox_id == SELBOX_REPLYOTH)
                    {
                        if (strlen(find) >= 28)
                        {
                            break;
                        }
                        *(find + strlen(find)) = (char)toupper((char)Msg);
                        WndCurr(hPrev);
                        WndWriteStr(strlen(topMsg) + 4, 0, cm[MN_NTXT], find);
                        WndCurr(hWnd);
                    }
                    else
                    {
                        *find = (char)toupper((char)Msg);
                    }

                    i = currItem;

                    while (i < itemCnt)
                    {
                        if (selbox_id == SELBOX_REPLYOTH)
                        {
                            if (stristr(Itms[i] + 1, find) == Itms[i] + 1)
                            {
                                break;
                            }
                        }
                        else
                        {
                            if (toupper(*Itms[i]) == *find)
                            {
                                if (selbox_id == SELBOX_MOVEMSG ||
                                  selbox_id == SELBOX_WRTMODE ||
                                  selbox_id == SELBOX_WRTOVER)
                                {
                                    SelShowItem(Itms[currItem], curY, len, Norm, 1);
                                    return i;
                                }
                            }
                        }
                        i++;
                    }
                    if (i == itemCnt)
                    {
                        for (i = 0; i < currItem; i++)
                        {
                            if (selbox_id == SELBOX_REPLYOTH)
                            {
                                if (stristr(Itms[i] + 1, find) == Itms[i] + 1)
                                {
                                    break;
                                }
                            }
                            else
                            {
                                if (toupper(*Itms[i]) == *find)
                                {
                                    if (selbox_id == SELBOX_MOVEMSG ||
                                      selbox_id == SELBOX_WRTMODE ||
                                      selbox_id == SELBOX_WRTOVER)
                                    {
                                        SelShowItem(Itms[currItem], curY, len, Norm, 1);
                                        return i;
                                    }
                                }
                            }
                        }
                    }
                    if (i != currItem)
                    {
                        SelShowItem(Itms[currItem], curY, len, Norm, 1);
                        currItem = i;
                        curY = y1;
                        Top = currItem;

                        /* Get the cursor position right... */

                        if (itemCnt - 1 - currItem < y2 - y1)
                        {
                            if (currItem > y2 - y1)
                            {
                                curY = y2;
                                Top = currItem - (y2 - y1);
                            }
                            else
                            {
                                curY = currItem + y1;
                                Top = 0;
                            }
                        }
                        SelShowPage(Itms, y1, y2, len, Top, Norm, 1);
                        SelShowItem(Itms[currItem], curY, len, Sel, 1);
                    }
                }
                else if (Msg == '\b' && *find)
                {
                    if (*topMsg && selbox_id == SELBOX_REPLYOTH)
                    {
                        *(find + strlen(find) - 1) = '\0';
                        WndCurr(hPrev);
                        WndClear(strlen(topMsg) + 3 + strlen(find), 0, maxx - 32, 0, cm[MN_NTXT]);
                        WndCurr(hWnd);
                    }
                }
                else
                {
                    memset(find, '\0', sizeof find);
                }
                break;

            }
            break;
        }
    }
    return -1;
}

#ifndef NODOMENU

int DoMenu(int x1, int y1, int x2, int y2, char **Itms, int def, int selbox_id, char *topMsg)
{
    WND *hCurr, *hWnd;
    int ret;

    hCurr = WndTop();
    hWnd = WndOpen(x1 - 1, y1 - 1, x2 + 3, y2 + 1, DBDR | SHADOW, cm[MN_BTXT], cm[MN_NTXT]);

    if (!hWnd)
    {
        return -1;
    }

    if (selbox_id >= SELBOX_CHARSET)
    {
        WndTitle(topMsg, cm[MN_TTXT]);
    }

    ret = SelBox(Itms, 0, y2 - y1, x2 - x1 + 2, def, hCurr, hWnd, cm[MN_STXT], cm[MN_NTXT], selbox_id, topMsg);

    WndClose(hWnd);
    WndCurr(hCurr);

    return ret;
}

#endif
