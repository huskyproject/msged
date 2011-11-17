/*
 *  AREAS.C
 *
 *  Written by jim nutt, John Dennis and released into the public domain
 *  by John Dennis in 1994.
 *
 *  This file contains the routines to select one of the areas specified
 *  in the config files.  It pops up a list with all the areas and lets
 *  the user choose.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "winsys.h"
#include "menu.h"
#include "main.h"
#include "misc.h"
#include "memextra.h"
#include "specch.h"
#include "keys.h"
#include "unused.h"
#include "help.h"
#include "version.h"
#include "strextra.h"
#include "areas.h"
#include "group.h"

char **alist = NULL;
char **alist2 = NULL;

static int AreaBox(char **Itms, int y1, int real_y2, int len, int def, WND * hPrev, WND * hWnd, int Sel, int Norm, int indent);

void BuildList(char ***lst)
{
    int i;
    AREA *a;
    char line[256];
    unsigned long unread, last;
    int areano;

    *lst = xcalloc(SW->groupareas + 2 ,sizeof(char *));

    for (i = 0; i < SW->groupareas; i++)
    {
        areano = group_getareano(i);

        if (areano>=0)
        {
            a = arealist + areano;
            memset(line, ' ', sizeof line);

            if (a->scanned)
            {
                last = a->lastread;
                if (last > a->messages)
                {
                    last = a->messages;
                }
                unread = a->messages - a->lastread;
                if (unread > a->messages)
                {
                    unread = a->messages;
                }
                
                /* F_ALTERNATE for SC14 is set in SelShowItem */
                sprintf(line, "%c%-*.*s", unread ? (SC14) : ' ',
                        maxx - 25, maxx - 25, a->description);
                line[strlen(line)] = ' ';
                sprintf(line + maxx - 23, "%6lu%6lu%6lu",
                        a->messages, unread, last);
            }
            else
            {
                sprintf(line, " %-*.*s", maxx - 25, maxx - 25, a->description);
                line[strlen(line)] = ' ';
                sprintf(line + maxx - 19, " -     -     -");
            }
        }
        else
        {
            memset(line, '=', sizeof line);
            sprintf(line + 2, " Group: %s ", group_getname(-areano));
            line[strlen(line)]='=';
            line[sizeof(line) - 1] = '\0';
        }

        (*lst)[i] = xstrdup(line);
    }
    (*lst)[i] = NULL;
}

static void SelShowItem(char *text, int y, int len, int Attr, int indent)
{
    char line[256];

    memset(line, ' ', sizeof(line));

    if (strlen(text) + indent > sizeof(line) - 1)
    {
        strcpy(line,"<--- internal buffer overflow --->");
    }

    strcpy(line + indent, text);
    line[sizeof(line)-1] = 0;
    if (indent)
    {
        WndPutsn(1, y, indent, Attr, line);
    }
    WndPutsn(1 + indent, y, 1, Attr | F_ALTERNATE, line + indent);
    if (len > (indent + 1))
    {
        WndPutsn(2 + indent, y, len - (indent + 1), Attr, line+indent+1);
    }
}

static void SelShowPage(char **text, int top, int bot, int len, int pos, int Attr, int indent)
{
    int i, y;

    y = top;
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

static void CalcDef(int max, int cur, int *top, int miny, int maxy, int *y)
{
    int dif;
    unused(cur);
    dif = (maxy - 1) - miny;
    if ((max - 1) - *top < dif && max > dif)
    {
        *y = maxy;
        *top = (max - 1) - dif;
    }
}

static void setup_areabox_coordinates(int *currItem, int *curY,
                                      int *Top, int *y2, int *page,
                                      int y1, int real_y2, int itemCnt)
{

    *y2=min(real_y2, itemCnt);
    *page = *y2 - y1;

    *curY = y1;

    *Top = *currItem;

    if (*currItem + y1 < y1)
    {
        *curY = y1 + *currItem;
        *Top = 0;
    }
    else
    {
        if ((itemCnt - *currItem) <= (*y2 - y1))
        {
            (*Top) -= ((*y2 - y1 + 1) - (itemCnt - *Top));
            *curY = y1 + (*currItem - *Top);
            if (*Top < 0)
            {
                *Top = 0;
                (*curY)--;
            }
        }
    }
}


static int AreaBoxCurItem;

static int AreaBox(char **Itms, int y1, int real_y2, int len, int def,
                   WND * hPrev, WND * hWnd, int Sel, int Norm, int indent)
{
    EVT e;
    char find[30];
    int itemCnt, Stuff, done, curY, Msg, currItem, Top, page, i, y2;
    int real_areano;

    itemCnt = 0;
    Stuff = 0;
    for (i = 0; Itms[i] != NULL; i++)
    {
        itemCnt++;
    }

    currItem = def;

    setup_areabox_coordinates(&currItem, &curY, &Top, &y2, &page,
                              y1, real_y2, itemCnt);
    
    done = 0;

    TTBeginOutput();
    SelShowPage(Itms, y1, y2, len, Top, Norm, indent);
    SelShowItem(Itms[currItem], curY, len, Sel, indent);
    TTEndOutput();

    TTClearQue();               /* clear input queue */

    memset(find, '\0', sizeof find);
    while (!done)
    {
        if (!*find)
        {
            WndCurr(hPrev);
            WndClear(16, 0, maxx - 36, 0, cm[MN_NTXT]);
            WndWriteStr(0, 0, cm[MN_NTXT], ">>Pick New Area:");
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
        case WND_WM_RESIZE:
            maxx = term.NCol;
            maxy = term.NRow;
            AreaBoxCurItem = currItem;
            return -2;  /* leave the list so it can be rebuilt with
                           the proper window dimensions */

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
                                    if (group_getareano(currItem) >= 0)
                                        return currItem;
                                }
                                else
                                {
                                    continue;
                                }
                            }

                            SelShowItem(Itms[currItem], curY, len, Norm, indent);

                            if (y > curY)
                            {
                                currItem += y - curY;
                            }
                            else
                            {
                                currItem -= curY - y;
                            }

                            curY = y;
                            SelShowItem(Itms[currItem], curY, len, Sel, indent);

                            if (Msg == LMOU_CLCK || Msg == MOU_LBTUP)
                            {
                                if (group_getareano(currItem) >= 0)
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
                TTBeginOutput();
                SelShowItem(Itms[currItem], curY, len, Norm, indent);
                currItem = 0;
                Top = 0;
                curY = y1;
                SelShowPage(Itms, y1, y2, len, Top, Norm, indent);
                SelShowItem(Itms[currItem], curY, len, Sel, indent);
                TTEndOutput();
                memset(find, '\0', sizeof find);
                break;

            case Key_A_H:
            case Key_F1:
                if (ST->helpfile != NULL)
                {
                    DoHelp(3);
                }
                break;

            case Key_End:
                if (currItem == itemCnt - 1)
                {
                    break;
                }
                TTBeginOutput();
                SelShowItem(Itms[currItem], curY, len, Norm, indent);
                currItem = itemCnt - 1;
                while (currItem && currItem >= (itemCnt - page))
                {
                    currItem--;
                }
                Top = currItem;
                currItem = itemCnt - 1;
                curY = currItem - Top + y1;
                CalcDef(itemCnt, currItem, &Top, y1, y2, &curY);
                SelShowPage(Itms, y1, y2, len, Top, Norm, indent);
                SelShowItem(Itms[currItem], curY, len, Sel, indent);
                TTEndOutput();
                memset(find, '\0', sizeof find);
                break;

            case Key_Dwn:
                if (currItem == itemCnt - 1)
                {
                    break;
                }
                SelShowItem(Itms[currItem], curY, len, Norm, indent);
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
                SelShowItem(Itms[currItem], curY, len, Sel, indent);
                memset(find, '\0', sizeof find);
                break;

            case Key_Up:
                if (!currItem)
                {
                    break;
                }
                SelShowItem(Itms[currItem], curY, len, Norm, indent);
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
                SelShowItem(Itms[currItem], curY, len, Sel, indent);
                memset(find, '\0', sizeof find);
                break;

            case Key_PgUp:
                if (!currItem)
                {
                    break;
                }
                TTBeginOutput();
                SelShowItem(Itms[currItem], curY, len, Norm, indent);
                if ((currItem -= page) < 0)
                {
                    currItem = 0;
                }
                Top = currItem;
                curY = y1;
                SelShowPage(Itms, y1, y2, len, Top, Norm, indent);
                SelShowItem(Itms[currItem], curY, len, Sel, indent);
                TTEndOutput();
                memset(find, '\0', sizeof find);
                break;

            case Key_PgDn:
                if (currItem == itemCnt - 1)
                {
                    break;
                }
                TTBeginOutput();
                SelShowItem(Itms[currItem], curY, len, Norm, indent);
                Top = currItem;

                if ((currItem += page) > itemCnt - 1)
                {
                    currItem = itemCnt - 1;
                    if (currItem > page)
                    {
                        Top = currItem - page;
                    }
                    else
                    {
                        Top = 0;
                    }
                }

                curY = currItem - Top + y1;
                CalcDef(itemCnt, currItem, &Top, y1, y2, &curY);
                SelShowPage(Itms, y1, y2, len, Top, Norm, indent);
                SelShowItem(Itms[currItem], curY, len, Sel, indent);
                TTEndOutput();
                memset(find, '\0', sizeof find);
                break;

            case Key_Ent:
            case Key_Rgt:
                if (group_getareano(currItem) >= 0)
                {
                    size_t i;

                    i = (size_t) (((char *)(strchr(Itms[currItem] + 1, ' '))) - Itms[currItem]);
                    i--;
                    if (i > 28)
                    {
                        i = 28;
                    }
                    strncpy(find, Itms[currItem] + 1, i);
                    *(find + i) = '\0';
                    strupr(find);
                    WndCurr(hPrev);
                    WndWriteStr(17, 0, cm[MN_NTXT], find);
                    WndCurr(hWnd);
                    return currItem;
                }
                break;

            case Key_Esc:
                if (SW->confirmations)
                {
                    if (confirm("Exit " PROG "?"))
                    {
                        return -1;
                    }
                }
                else
                {
                    return -1;
                }
                memset(find, '\0', sizeof find);
                break;

            case Key_A_X:
                return -1;

            case '*':
            case '#':
            case Key_A_S:
            case Key_A_T:
                arealist_area_scan(Msg == '*' || Msg == Key_A_T);

                for (i = 0; i < SW->groupareas; i++)
                {
                    xfree(alist[i]);
                }
                xfree(alist);
                BuildList(&alist);
                Itms = alist;
                TTBeginOutput();
                SelShowPage(Itms, y1, y2, len, Top, Norm, indent);
                SelShowItem(Itms[currItem], curY, len, Sel, indent);
                TTEndOutput();
                memset(find, '\0', sizeof find);
                break;

            case Key_A_1:
            case Key_A_2:
            case Key_A_3:
            case Key_A_4:
            case Key_A_5:
            case Key_A_6:
            case Key_A_7:
            case Key_A_8:
            case Key_A_9:
            case Key_A_0:
            case Key_A_G:
            case Key_C_G:

                real_areano = group_getareano(currItem);
                i = 1;

                switch(Msg)
                {
                case Key_C_G:
                    i = sel_group();
                    break;
                case Key_A_1:
                    group_set_group(1);
                    break;
                case Key_A_2:
                    group_set_group(2);
                    break;
                case Key_A_3:
                    group_set_group(3);
                    break;
                case Key_A_4:
                    group_set_group(4);
                    break;
                case Key_A_5:
                    group_set_group(5);
                    break;
                case Key_A_6:
                    group_set_group(6);
                    break;
                case Key_A_7:
                    group_set_group(7);
                    break;
                case Key_A_8:
                    group_set_group(8);
                    break;
                case Key_A_9:
                    group_set_group(9);
                    break;
                case Key_A_0:
                    group_set_group(0);
                    break;
                }
                    
                if (i)
                {
                    /* group has changed ... we have quite some work! */

                    /* find the new position of the currently selected group */
                    currItem = group_getareano(0) < 0 ? 1 : 0;
                        
                    for (i = 0; i < SW->groupareas; i++)
                    {
                        if (group_getareano(i) == real_areano)
                        {
                            currItem = i;
                            break;
                        }
                    }

                    /* delete the old area list */

                    for (i = 0; alist[i] != NULL; i++)
                    {
                        xfree(alist[i]);
                    }
                    xfree(alist);
                    
                    /* Build a new area list */
                    BuildList(&alist);
                    Itms = alist;
                    itemCnt = 0;
                    for (i = 0; Itms[i] != NULL; i++)
                    {
                        itemCnt++;
                    }

                    /* Do coordinate arithmetics */
                    y2 = min(maxy - 4, SW->groupareas);
                    setup_areabox_coordinates(&currItem, &curY, &Top,
                                              &y2, &page,
                                              y1, real_y2, itemCnt);

                    /* Redraw everything */
                    TTBeginOutput();
                    SelShowPage(Itms, y1, y2, len, Top, Norm, indent);
                    SelShowItem(Itms[currItem], curY, len, Sel, indent);
                    if (y2 < real_y2)
                    {
                        WndClear(1, y2 + 1, len, real_y2, Norm);
                    }
                    TTEndOutput();
                    memset(find, '\0', sizeof find);
                }
                break;
            default:
                if (Msg > 32 && Msg < 127)
                {
                    if (strlen(find) >= 28)
                    {
                        break;
                    }
                    *(find + strlen(find)) = (char)toupper((char)Msg);
                    WndCurr(hPrev);
                    WndWriteStr(17, 0, cm[MN_NTXT], find);
                    WndCurr(hWnd);
                    i = currItem;

                    while (i < itemCnt)
                    {
                        if (SW->arealistexactmatch)
                        {
                            if (stristr(Itms[i] + 1, find) == Itms[i] + 1)
                            {
                                break;
                            }
                        }
                        else
                        {
                            if (stristr(Itms[i] + 1, find) != NULL)
                            {
                                break;
                            }
                        }
                        i++;
                    }
                    if (i == itemCnt)
                    {
                        for (i = 0; i < currItem; i++)
                        {
                            if (SW->arealistexactmatch)
                            {
                                if (stristr(Itms[i] + 1, find) == Itms[i] + 1)
                                {
                                    break;
                                }
                            }
                            else
                            {
                                if (stristr(Itms[i] + 1, find) != NULL)
                                {
                                    break;
                                }
                            }
                        }
                        if (i == currItem)
                        {
                            i = itemCnt;
                        }
                    }
                    if ((i != currItem) && (i != itemCnt))
                    {
                        SelShowItem(Itms[currItem], curY, len, Norm, indent);
                        currItem = i;
                        curY = y1;
                        Top = currItem;

                        if ((itemCnt - 1) - currItem < y2 - y1)
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
                        SelShowPage(Itms, y1, y2, len, Top, Norm, indent);
                        SelShowItem(Itms[currItem], curY, len, Sel, indent);
                    }
                }
                else if (Msg == '\b' && *find)
                {
                    *(find + strlen(find) - 1) = '\0';
                    WndCurr(hPrev);
                    WndClear(17 + strlen(find), 0, maxx - 36, 0, cm[CM_NINF]);
                    WndCurr(hWnd);
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

int mainArea(void)
{
    WND *hCurr, *hWnd;
    int ret = -1, wid, dep, i;

    do
    {
        msgederr = 0;
        wid = maxx - 1;
        dep = maxy - 2;

        TTBeginOutput();
        WndClearLine(0, cm[MN_NTXT]);
        WndClearLine(maxy - 1, cm[MN_NTXT]);
        hCurr = WndTop();
        hWnd = WndOpen(0, 1, wid, dep, NBDR | NOSAVE, 0, cm[MN_BTXT]);
        WndBox(0, 0, maxx - 1, maxy - 3, cm[MN_BTXT], SBDR);

        WndWriteStr(3, 0, cm[LS_TTXT], "EchoID");
        WndWriteStr(maxx - 19, 0, cm[LS_TTXT], "Msgs");
        WndWriteStr(maxx - 12, 0, cm[LS_TTXT], "New");
        WndWriteStr(maxx - 7, 0, cm[LS_TTXT], "Last");
        TTEndOutput();


        BuildList(&alist);

        ret = AreaBox(alist, 1, dep - 2, wid - 1, 
                      (ret == -2) ? AreaBoxCurItem : SW->grouparea,
                      hCurr, hWnd, cm[MN_STXT], cm[MN_NTXT], 1);

        for (i = 0; alist[i] != NULL; i++)
        {
                xfree(alist[i]);
        }
        
        xfree(alist);
        
        if (ret == -1)
        {
                msgederr = 1;
        }
        if (ret == -2)
        {
            /*  returncode -2 means that the list should be restarted;
                it did only exit because it had to be rebuild because
                of a terminal window resize operation */
            window_resized = 1;
        }
        
        WndClose(hWnd);
        WndCurr(hCurr);

        /* returncode -3 means rebuild because group has changed */

    } while (ret == -2 || ret == -3);

    return ret;
}

int selectarea(char *topMsg, int def)
{
    WND *hCurr, *hWnd;
    int ret, wid, dep, i;

    msgederr = 0;
    wid = maxx - 1;
    dep = maxy - 2;

    WndClearLine(0, cm[MN_NTXT]);
    WndClearLine(maxy - 1, cm[MN_NTXT]);
    hCurr = WndTop();
    hWnd = WndOpen(0, 1, wid, dep, NBDR, 0, cm[MN_BTXT]);
    WndBox(0, 0, maxx - 1, maxy - 3, cm[MN_BTXT], SBDR);
    WndCurr(hCurr);
    WndClear(0, 0, maxx - 36, 0, cm[CM_NINF]);
    WndCurr(hWnd);
    WndWriteStr(3, 0, cm[LS_TTXT], "EchoID");
    WndWriteStr(maxx - 21, 0, cm[LS_TTXT], "Msgs");
    WndWriteStr(maxx - 13, 0, cm[LS_TTXT], "New");
    WndWriteStr(maxx - 7, 0, cm[LS_TTXT], "Last");

    BuildList(&alist2);

    do
    {
        ret = SelBox(alist2, 1, dep - 2, wid - 1, def, hCurr, hWnd,
                     cm[MN_STXT], cm[MN_NTXT], SELBOX_REPLYOTH, topMsg);
    } while (ret != -1 && group_getareano(ret) < 0);

                                /* don't allow group separators
                                   as selection */

    for (i = 0; i < SW->groupareas; i++)
    {
        xfree(alist2[i]);
    }

    xfree(alist2);

    if (ret < 0)
    {
        msgederr = 1;
    }

    WndClose(hWnd);
    WndCurr(hCurr);

    if (ret < 0)
    {
        return def;
    }

    return ret;
}
