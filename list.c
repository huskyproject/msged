/*
 *  LIST.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Lists the messages in the messagebase.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "memextra.h"
#include "specch.h"
#include "winsys.h"
#include "menu.h"
#include "main.h"
#include "strextra.h"
#include "keys.h"
#include "dosmisc.h"
#include "help.h"
#include "maintmsg.h"
#include "nshow.h"
#include "dlist.h"
#include "list.h"
#include "screen.h"

static int long_subj;
static int display_address = 1;

static void getheader(unsigned long n, MLHEAD * h, int check_sel);
static void showit(MLHEAD * h, int y, int sel);

static DLIST *ulist;

static char *forgroupmenu[] =
{
    "Move Message(s)",
    "Copy Message(s)",
    "Redirect Message(s)",
    "Forward Message(s)",
    NULL
};

/*
 *  Checks to see if the message has been selected.
 */

static int ulistFindUid(unsigned long uid)
{
    DLISTNODE *p_node;
    if (ulist == NULL)
    {
        return 0;
    }
    p_node = dlistTravFirst(ulist);
    while (p_node != NULL)
    {
        unsigned long *puid;
        puid = dlistGetElement(p_node);
        if (*puid == uid)
        {
            return 1;
        }
        p_node = dlistTravNext(p_node);
    }
    return 0;
}

/*
 *  Removes a selection from the list.
 */

static void ulistDropUid(unsigned long uid)
{
    DLISTNODE *p_node;
    if (ulist == NULL)
    {
        return;
    }
    p_node = dlistTravFirst(ulist);
    while (p_node != NULL)
    {
        unsigned long *puid;
        puid = dlistGetElement(p_node);
        if (*puid == uid)
        {
            dlistDropNode(ulist, p_node);
        }
        p_node = dlistTravNext(p_node);
    }
}

/*
 *  Adds a new message to the list of selected messages.
 */

static int ulistAddUid(unsigned long uid)
{
    unsigned long *puid;
    DLISTNODE *p_node;

    if (ulist == NULL)
    {
        ulist = dlistInit();
        if (ulist == NULL)
        {
            return 0;
        }
    }
    puid = xmalloc(sizeof *puid);
    if (puid == NULL)
    {
        return 0;
    }
    p_node = dlistCreateNode(puid);
    if (p_node == NULL)
    {
        xfree(puid);
        return 0;
    }
    *puid = uid;
    dlistAddNode(ulist, p_node);
    return 1;
}

/*
 *  Frees the list of msgnumbers.
 */

static void ulistTerm(void)
{
    DLISTNODE *p_node;
    if (ulist == NULL)
    {
        return;
    }
    p_node = dlistTravFirst(ulist);
    while (p_node != NULL)
    {
        unsigned long *puid;
        puid = dlistGetElement(p_node);
        xfree(puid);
        p_node = dlistTravNext(p_node);
    }
    dlistTerm(ulist);
    ulist = NULL;
}

/*
 *  Re-displays the entire screen.
 */

static void update(MLHEAD * headers, unsigned long i, int y)
{
    while (i <= CurArea.messages && y <= maxy - 4)
    {
        getheader(i, &headers[y - 1], 1);
        showit(&headers[y - 1], y, 0);
        i++;
        y++;
    }
    if (y <= (maxy - 4))
    {
        WndClear(1, y, maxx - 2, maxy - 4, cm[LS_NTXT]);
    }
}

/*
 *  Shows a header on the screen.
 */

static void showit(MLHEAD * h, int y, int sel)
{
    unsigned long msgn;
    char line[384];
    char msgnbuf[9];

    msgn = SW->showrealmsgn ? h->umsgid : h->msgnum;
    sprintf(msgnbuf, "%5ld %c", msgn, h->sel ? SC14 : ' ');

    if (long_subj)
    {
        sprintf(line, "%c%s%-15.15s %-70s", h->times_read > 0 ? '*' : ' ',
          msgnbuf, h->fr_name, h->subj);
    }
    else
    {
        sprintf(line, "%c%s%-15.15s %-15.15s %-70s",
          h->times_read > 0 ? '*' : ' ', msgnbuf, h->fr_name,
          h->to_name, h->subj);
    }

    if (sel)
    {
        WndPutsn(1, y, maxx - 2, cm[LS_STXT], line);
    }
    else if (stricmp(h->to_name, ST->username) == 0 ||
      stricmp(h->fr_name, ST->username) == 0)
    {
        WndPutsn(1, y, maxx - 2, cm[LS_ITXT], line);
    }
    else
    {
        WndPutsn(1, y, maxx - 2, cm[LS_NTXT], line);
    }
}

/*
 *  Gets a header from the msgbase.
 */

static void getheader(unsigned long n, MLHEAD * h, int check_sel)
{
    msg *x;

    memset(h, 0, sizeof *h);
    x = MsgReadHeader((unsigned int)n, RD_HEADER_BRIEF);
    if (x == NULL)
    {
        return;
    }

    h->msgnum = n;
    h->umsgid = x->msgnum;
    h->to_net = x->to.net;
    h->to_node = x->to.node;
    h->fr_net = x->from.net;
    h->fr_node = x->from.node;
    h->times_read = x->times_read;
    if (check_sel)
    {
        h->sel = ulistFindUid(x->msgnum);
    }
    else
    {
        h->sel = 0;
    }

    strncpy(h->subj, x->subj, 72);
    h->subj[72] = '\0';
    strncpy(h->to_name, x->isto, 36);
    h->to_name[36] = '\0';
    strncpy(h->fr_name, x->isfrom, 36);
    h->fr_name[36] = '\0';

    dispose(x);
}

/*
 *  Deletes the selected messages, or the current one if none.
 */

static void DeleteMsgs(unsigned long *CurrMsgn)
{
    if (ulist == NULL || dlistIsEmpty(ulist))
    {
/*
        if (!confirm("Erase message?"))
        {
            return;
        }
*/
        CurArea.current = *CurrMsgn;
        deletemsg();
    }
    else
    {
        int confirm_temp;
        DLISTNODE *p_node;
        unsigned long msgn, oldmsgn;
        char messagetxt[80];

        if (!confirm("Erase all selected messages?"))
        {
            return;
        }
        if (!OpenMsgWnd(50, 6, "Deleting Messages", NULL, 0, 0))
        {
            return;
        }
        SendMsgWnd("Press Esc to interrupt", 2);

        confirm_temp = SW->confirmations;
        SW->confirmations = 0;

        oldmsgn = 0L;

        p_node = dlistTravFirst(ulist);
        while (p_node != NULL)
        {
            unsigned long *puid;

            if (KeyHit() && GetKey() == Key_Esc)
            {
                p_node = NULL;
                break;
            }

            puid = dlistGetElement(p_node);
            msgn = UidToMsgn(*puid);

            sprintf(messagetxt, "Working on message #%lu", 
                    SW->showrealmsgn ? *puid: msgn);
            SendMsgWnd(messagetxt, 1);

            if (oldmsgn == 0L)
            {
                oldmsgn = msgn - 1;
            }
            if (msgn != 0L)
            {
                CurArea.current = msgn;
                deletemsg();
            }
            p_node = dlistTravNext(p_node);
        }

        if (oldmsgn == 0L)
        {
            oldmsgn = *CurrMsgn;
        }
        *CurrMsgn = oldmsgn;
        CurArea.current = oldmsgn;
        SW->confirmations = confirm_temp;

        CloseMsgWnd();
    }
}

/*
 *  Forwards, redirects, moves or copies a group of messages.
 */

static int movemsgs(int rc, int to_area)
{
    int clear = (to_area == -1);
    
    if (rc == 2 || rc == 3)
    {
        DrawHeader();
    }
    switch (rc)
    {
    case 0:                    /* Move */
        to_area = move_msg(to_area);
        break;

    case 1:                    /* Copy */
        to_area = copy_msg(to_area);
        break;

    case 2:                    /* Redirect */
        to_area = redirect_msg(to_area);
        break;

    case 3:                    /* Forward */
        to_area = forward_msg(to_area);
        break;

    case -1:                   /* Escape */
        return -1;
    }

    if (clear)
    {
        WndClearLine(0, cm[MN_NTXT]);
        WndWriteStr(2, 0, cm[LS_TTXT], CurArea.description);

        if (rc == 2 || rc == 3)
        {
            int i;
            for (i = 1; i <= 5; i++)
            {
                WndClearLine(i, cm[MN_NTXT]);
            }
            WndBox(0, 1, maxx - 1, maxy - 2, cm[LS_BTXT], SBDR);
        }
    }
    return to_area;
}

/*
 *  Moves, copies, redirects or forwards the selected messages, or the
 *  current one if none.
 */

static void MoveMsgs(unsigned long *CurrMsgn)
{
    int rc;
    WND *hCurr;

    rc = DoMenu((maxx / 2) - 10, (maxy / 2) - 1, (maxx / 2) + 9, (maxy / 2) + 2,
      forgroupmenu, 0, SELBOX_MOVEMSG, "");

    if (ulist == NULL || dlistIsEmpty(ulist))
    {
        CurArea.current = *CurrMsgn;
        hCurr = WndTop();
        WndCurr(hMnScr);
        groupmove = 1;
        movemsgs(rc, -1);
        groupmove = 0;
        WndCurr(hCurr);
    }
    else
    {
        DLISTNODE *p_node, *p_old_node;
        unsigned long msgn, oldmsgn;
        int to_area = -1;
        char messagetxt[80];

        oldmsgn = 0L;

        switch(rc)
        {
        case 0:
            strcpy(messagetxt," Moving");
            break;

        case 1:
            strcpy(messagetxt," Copying");
            break;

        case 2:
            strcpy(messagetxt," Redirecting");
            break;

        case 3:
            strcpy(messagetxt," Forwarding");
            break;

        default:
            strcpy(messagetxt," <internal error>");
            break;
        }
        strcat(messagetxt, " Messages ");

        if (!OpenMsgWnd(50, 6, messagetxt, NULL, 0, 0))
        {
            return;
        }
        SendMsgWnd("Press Esc to stop", 2);

        p_node = dlistTravFirst(ulist);
        while (p_node != NULL)
        {
            unsigned long *puid;

            if (KeyHit() && GetKey() == Key_Esc)
            {
                p_node = NULL;
                break;
            }

            puid = dlistGetElement(p_node);
            msgn = UidToMsgn(*puid);

            sprintf(messagetxt, "Working on message #%lu", 
                    SW->showrealmsgn ? *puid: msgn);
            SendMsgWnd(messagetxt, 1);
            
            if (oldmsgn == 0L)
            {
                oldmsgn = msgn - 1L;
            }
            p_old_node = p_node;
            if (msgn != 0L)
            {
                CurArea.current = msgn;
                hCurr = WndTop();
                WndCurr(hMnScr);
                groupmove = 1;
                to_area = movemsgs(rc, to_area);
                groupmove = 0;
                WndCurr(hCurr);
            }
            p_node = dlistTravNext(p_node);
            dlistDropNode(ulist, p_old_node);

            if (to_area == -1) /* an escape or an error occured */
            {
                p_node = NULL;
            }
        }
        if (oldmsgn == 0L)
        {
            oldmsgn = *CurrMsgn;
        }
        *CurrMsgn = oldmsgn;
        CurArea.current = oldmsgn;

        CloseMsgWnd();
    }
}

/*
 *  Puts a list of messages up in a window on the screen.  Allows for
 *  some basic management of those messages.
 */

void do_list(void)
{
    EVT event;
    WND *hWnd, *hCurr;
    static int in_list = 0;     /* stop recursion */
    MLHEAD *headers;            /* headers */
    int done = 0;               /* finished ? */
    int down = 0;
    int ForceEvt = 0;           /* forced/piped keypress */
    int Msg;                    /* message */
    int lbutton = 0;
    unsigned long i, a, j;
    int y;

    if (in_list || !CurArea.status)  /* stop recursion */
    {
        return;
    }
    else
    {
        in_list = 1;
    }

    /* Open the window and draw the screen and allocate the memory. */

    WndClearLine(0, cm[MN_NTXT]);
    WndClearLine(maxy - 1, cm[MN_NTXT]);
    WndWriteStr(2, 0, cm[LS_TTXT], CurArea.description);
    hCurr = WndTop();
    hWnd = WndOpen(0, 1, maxx - 1, maxy - 2, NBDR | NOSAVE, 0, cm[LS_NTXT]);
    headers = xcalloc(maxy, sizeof(MLHEAD));

    WndBox(0, 0, maxx - 1, maxy - 3, cm[LS_BTXT], SBDR);

    message = KillMsg(message);
    a = CurArea.current;
    y = 1;
    update(headers, a, y);

    while (!done)
    {
#if defined(MSDOS) && !defined(__FLAT__)
        /* shows memory if compiled under dos */

        if (SW->statbar)
        {
            char line[255];
            sprintf(line, "%c %3ldK ", SC7, (long)(corerem() / 1024));
            WndCurr(WndTop());
            WndPutsn(maxx - 7, maxy - 1, 7, cm[CM_ITXT], line);
            WndCurr(hWnd);
        }
#endif

        WndWriteStr(3, 0, cm[LS_TTXT], "Msg");
        WndWriteStr(9, 0, cm[LS_TTXT], "From");
        if (long_subj)
        {
            char tmp[8];
            memset(tmp, SC8, 7);
            *(tmp + 7) = '\0';
            WndWriteStr(25, 0, cm[LS_TTXT], "Subject");
            WndWriteStr(41, 0, cm[LS_BTXT], tmp);
        }
        else
        {
            char tmp[8];
            memset(tmp, SC8, 7);
            *(tmp + 7) = '\0';
            WndWriteStr(25, 0, cm[LS_BTXT], tmp);
            WndWriteStr(25, 0, cm[LS_TTXT], "To");
            WndWriteStr(41, 0, cm[LS_TTXT], "Subject");
        }
        showit(&headers[y - 1], y, 1);

        if (down)
        {
            /* If a selection has occured, then force cursor down. */

            Msg = Key_Dwn;
            event.msg = Msg;
            event.msgtype = WND_WM_CHAR;
            down = 0;
        }
        else if (ForceEvt)
        {
            /*
             * These events are from the mouse (no point in
             * duplicating code).
             */

            Msg = ForceEvt;
            event.msg = Msg;
            event.msgtype = WND_WM_CHAR;
            ForceEvt = 0;
        }
        else
        {
            Msg = MnuGetMsg(&event, hWnd->wid);
        }

        switch (event.msgtype)
        {
        case WND_WM_MOUSE:
            switch (Msg)
            {
            case LMOU_RPT:
                {
                    int y1 = event.y;

                    if (y1 > maxy - 4 && lbutton)
                    {
                        ForceEvt = Key_Dwn;
                    }
                    else if (y1 < 1 && lbutton)
                    {
                        ForceEvt = Key_Up;
                    }
                }
                break;

            case LMOU_CLCK:
            case MOU_LBTDN:
            case MOU_LBTUP:
            case MOUSE_EVT:
                {
                    int y1 = event.y - 1, ok = 0;

                    if (Msg == MOU_LBTDN)
                    {
                        lbutton = 1;
                    }
                    else if (Msg == MOU_LBTUP)
                    {
                        lbutton = 0;
                    }

                    if (y1 > maxy - 4)
                    {
                        if (Msg == MOU_LBTDN && lbutton)
                        {
                            ForceEvt = Key_Dwn;
                        }
                    }
                    else
                    {
                        if (y1 < 1)
                        {
                            if (Msg == MOU_LBTDN && lbutton)
                            {
                                ForceEvt = Key_Up;
                            }
                        }
                        else
                        {
                            /* The event occured on the list. */

                            if (y == y1 && (Msg == MOU_LBTUP || Msg == LMOU_CLCK))
                            {
                                ForceEvt = Key_Ent;
                                continue;
                            }
                            showit(&headers[y - 1], y, 0);
                            if (y > y1)
                            {
                                if (a - y - y1 >= 1)
                                {
                                    a -= y - y1;
                                    ok = TRUE;
                                }
                            }
                            else
                            {
                                if (a + y1 - y <= CurArea.messages)
                                {
                                    a += y1 - y;
                                    ok = TRUE;
                                }
                            }

                            if (ok == TRUE)
                            {
                                y = y1;
                                if (Msg == MOU_LBTUP || Msg == LMOU_CLCK)
                                {
                                    ForceEvt = Key_Ent;
                                }
                            }
                        }
                    }
                }
                break;

            default:
                break;
            }
            break;

        case WND_WM_CHAR:
            switch (Msg)
            {
            case Key_PgDn:
                i = maxy - 4 - y;
                while (i > 0 && a < CurArea.messages)
                {
                    i--;
                    a++;
                }
                y = 1;
                update(headers, a, y);
                break;

            case Key_PgUp:
                if (y == 1)
                {
                    i = maxy - 5;
                }
                else
                {
                    i = y - 1;
                }
                while (i > 0 && a > 1)
                {
                    i--;
                    a--;
                }
                y = 1;
                update(headers, a, y);
                break;

            case Key_Up:
                if (a > 1)
                {
                    showit(&headers[y - 1], y, 0);
                    a--;
                    y--;

                    if (y < 1)
                    {
                        y = 1;
                        WndScroll(1, 1, maxx - 2, maxy - 4, 0);
                        if (SW->statbar)
                        {
                            memmove(headers + 1, headers,
                              sizeof(MLHEAD) * (maxy - 2));
                        }
                        else
                        {
                            memmove(headers + 1, headers,
                              sizeof(MLHEAD) * (maxy - 1));
                        }
                        getheader(a, &headers[0], 1);
                    }
                }
                break;

            case Key_Dwn:
                if (a < CurArea.messages)
                {
                    showit(&headers[y - 1], y, 0);
                    a++;
                    y++;

                    if (y > maxy - 4)
                    {
                        y = maxy - 4;
                        WndScroll(1, 1, maxx - 2, y, 1);
                        if (SW->statbar)
                        {
                            memmove(headers, headers + 1,
                              sizeof(MLHEAD) * (maxy - 2));
                        }
                        else
                        {
                            memmove(headers, headers + 1,
                              sizeof(MLHEAD) * (maxy - 1));
                        }
                        getheader(a, &headers[y - 1], 1);
                    }
                }
                break;

            case Key_Home:
                a = CurArea.first;
                update(headers, a, y = 1);
                break;

            case Key_End:
                a = CurArea.last;
                update(headers, a, y = 1);
                break;

            case '+':
                ulistTerm();
                for (j = CurArea.first; j <= CurArea.messages; j++)
                {
                    ulistAddUid(MsgnToUid(j));
                }
                update(headers, a, y = 1);
                break;

            case '-':
                ulistTerm();
                update(headers, a, y = 1);
                break;

            case Key_Spc:
                if (headers[y - 1].sel == 0)
                {
                    headers[y - 1].sel = 1;
                    ulistAddUid(headers[y - 1].umsgid);
                }
                else
                {
                    ulistDropUid(headers[y - 1].umsgid);
                    headers[y - 1].sel = 0;
                }
                showit(&headers[y - 1], y, 1);
                down = 1;
                break;

            case Key_Del:
                DeleteMsgs(&a);
                if (a > CurArea.last)
                {
                    a = CurArea.last;
                }
                update(headers, a, y = 1);
                break;

            case Key_A_H:
                if (ST->helpfile)
                {
                    DoHelp(2);
                }
                break;

            case Key_Ent:
                CurArea.current = a;
                done = 1;
                break;

            case Key_A_X:
            case Key_Esc:
                done = 1;
                break;

            case Key_A_S:
                long_subj ^= 1;
                update(headers, a, y = 1);
                break;

            case Key_A_A:
                display_address ^= 1;
                update(headers, a, y = 1);
                break;

            case Key_A_M:
                MoveMsgs(&a);
                if (a > CurArea.last)
                {
                    a = CurArea.last;
                }
                update(headers, a, y = 1);
                break;

            default:
                break;
            }
            break;
        }
    }
    in_list = 0;
    ulistTerm();
    xfree(headers);
    WndClose(hWnd);
    WndCurr(hCurr);
}
