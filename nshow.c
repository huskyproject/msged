/*
 *  NSHOW.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Routines for displaying message information on the screen.  This
 *  includes both text and header information.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "winsys.h"
#include "menu.h"
#include "main.h"
#include "dialogs.h"
#include "specch.h"
#include "dosmisc.h"
#include "version.h"
#include "date.h"
#include "vsevops.h"
#include "nshow.h"
/* for pgdn, go_dwn etc, for re-adjustment of screen size */
static LINE * top    = NULL;    /* top of screen */
static LINE * bottom = NULL;    /* bottom of screen */
static char line[256];          /* used as temporary storage */
static int scrHdrLen = 7;       /* for screen header length */
static WND * hMsgWnd, * hMsgCurr; /* main window and Msg window */
int groupmove = 0;
HotGroup Hot;
int InitScreen(void)
{
    if(SW->usemouse == NO)
    {
        term.Abil |= NOMOUSE;
    }

    TTopen();

    if(maxx_force)
    {
        maxx = maxx_force;

        if(maxx_force > term.NCol)
        {
            fprintf(stderr,
                    "Warning: MaxX setting does not seem to match your actual terminal size. Screen\n" "corruption will result from this. If you know your MaxX setting is correct,\n" "ignore this message. If you see screen corruption, remove the MaxX setting\n"
                                                                                                                                                                                       "setting from your configuration.\n");
        }
    }
    else
    {
        maxx = term.NCol;
    }

    if(maxx >= sizeof(line))
    {
        maxx = sizeof(line) - 1;
    }

    if(maxy_force)
    {
        maxy = maxy_force;

        if(maxy_force > term.NRow)
        {
            fprintf(stderr,
                    "Warning: MaxY setting does not seem to match your actual terminal size. Screen\n" "corruption will result from this. If you know your MaxY setting is correct,\n" "ignore this message. If you see screen corruption, remove the MaxY setting\n"
                                                                                                                                                                                       "setting from your configuration.\n");
        }
    }
    else
    {
        maxy = term.NRow;
    }

    hMnScr     = WndOpen(0, 0, maxx - 1, maxy - 1, NBDR, 0, cm[CM_NTXT]);
    SW->redraw = TRUE;
    return 0;
} /* InitScreen */

/*
 *  Adds an item to a HotGroup.
 */
void AddHG(HotGroup * h, int num, int id, int x1, int y1, int x2, int y2)
{
    h->harr[num].id = id;
    h->harr[num].x1 = x1;
    h->harr[num].y1 = y1;
    h->harr[num].x2 = x2;
    h->harr[num].y2 = y2;
}

/*
 *  Builds the hotspots on the main window and initializes the menu
 *  system.  We cannot use a hotspot for the menu system; we wish to
 *  send the messages un-altered to the menu processing function.
 */
void BuildHotSpots(void)
{
    Hot.wid = hMnScr->wid;
    Hot.num = 6;

    if(SW->statbar)
    {
        AddHG(&Hot, 0, ID_MGRGT, maxx - 2, 6, maxx - 1, maxy - 2);
        AddHG(&Hot, 1, ID_MGLFT, 0, 6, 1, maxy - 2);
        AddHG(&Hot, 2, ID_SCRUP, 0, 6, maxx - 1, 7);
        AddHG(&Hot, 3, ID_SCRDN, 0, maxy - 2, maxx - 1, maxy - 2);
        AddHG(&Hot, 4, ID_LNDN, 65, 2, 69, 2);
        AddHG(&Hot, 5, ID_LNUP, 70, 2, 75, 2);
    }
    else
    {
        AddHG(&Hot, 0, ID_MGRGT, maxx - 2, 6, maxx - 1, maxy - 1);
        AddHG(&Hot, 1, ID_MGLFT, 0, 6, 1, maxy - 1);
        AddHG(&Hot, 2, ID_SCRUP, 0, 6, maxx - 1, 7);
        AddHG(&Hot, 3, ID_SCRDN, 0, maxy - 1, maxx - 1, maxy - 1);
        AddHG(&Hot, 4, ID_LNDN, 65, 2, 69, 2);
        AddHG(&Hot, 5, ID_LNUP, 70, 2, 75, 2);
    }

    /* push the group onto the HotGroup stack */
    PushHotGroup(&Hot);
    /*
     *  Setup the mouse menu on top of the screen.  It'll use the
     *  parent window for displaying the menu, and we send any messages
     *  in that vicinity for processing by it.
     */
    MouseMnu.cmdtab[0].col = 0 + maxx - MNU_LEN - 1;
    MouseMnu.cmdtab[1].col = 7 + maxx - MNU_LEN - 1;
    MouseMnu.cmdtab[2].col = 14 + maxx - MNU_LEN - 1;
    MouseMnu.cmdtab[3].col = 21 + maxx - MNU_LEN - 1;
    MnuSetColours(cm[MN_BTXT], cm[CM_NINF], cm[MN_STXT]);
    /* set up the dialog box colours */
    SetDialogColors();
} /* BuildHotSpots */

/*
 *  Kills the hotspots on the stack.
 */
void KillHotSpots(void)
{
    PopHotGroup();
}

/*
 *  Draws the msg header on the screen.  This should only have to be
 *  drawn once during normal operation.
 */
void DrawHeader(void)
{
    static char line[256];

    TTBeginOutput();
    WndClear(0, 1, maxx - 1, 4, cm[CM_NINF]);
    memset(line, SC8, maxx);  /* clear dividing line */

    WndPutsn(0, 5, maxx, cm[CM_DTXT] | F_ALTERNATE, line);
    WndWriteStr(0, 1, cm[CM_FTXT], " From :");  /* header info */
    WndWriteStr(0, 2, cm[CM_FTXT], " To   :");
    WndWriteStr(0, 3, cm[CM_FTXT], " Subj :");
    WndWriteStr(0, 4, cm[CM_FTXT], " Attr :");

    if(SW->statbar)
    {
        int l;
        sprintf(line, " %s %s %c ", PROG, VERNUM VERPATCH, SC7);
        l = strlen(line) - 2;
        WndPutsn(0, maxy - 1, l, cm[CM_ITXT], line);

        if(l < (maxx - 1))
        {
            WndPutsn(l, maxy - 1, (maxx - 1) - l, cm[CM_ITXT] | F_ALTERNATE, line + l);
        }
    }

    TTEndOutput();
} /* DrawHeader */

/*
 *  Clears the screen (only used with the list function to make it
 *  look better when it returns).
 */
void ClearScreen(void)
{
    if(SW->statbar)
    {
        WndClear(0, 0, maxx - 1, maxy - 2, cm[CM_FTXT]);
    }
    else
    {
        WndClear(0, 0, maxx - 1, maxy - 1, cm[CM_FTXT]);
    }
}

/*
 *  Clears message information (header and text) from the screen.
 */
void ClearMsgScreen(void)
{
    TTBeginOutput();
    WndClear(8, 1, maxx - 1, 4, cm[CM_FTXT]);
    RefreshMsg(NULL, 6);
    TTEndOutput();
}

/*
 *  Shows a new area on the screen.
 */
void ShowNewArea(void)
{
    char tmpbuf[140];

    if(!groupmove)
    {
        TTBeginOutput();
        sprintf(tmpbuf,
                "%-*.*s",
                (maxx - 31) > 139 ? 139 : maxx - 31,
                (maxx - 31) > 139 ? 139 : maxx - 31,
                CurArea.description);
        WndClear(0, 0, maxx - 29, 0, cm[CM_NINF]);
        memset(line, SC8, maxx + 1);  /* clear dividing line */
        WndPutsn(0, 5, maxx, cm[CM_DTXT] | F_ALTERNATE, line);
        WndWriteStr(2, 0, cm[CM_NINF], tmpbuf);

        if(SW->showaddr)
        {
            sprintf(line, "%s", show_address(&CurArea.addr));
            WndWriteStr(3, 5, cm[CM_NINF], line);

            if(dv_running())
            {
                WndWriteStr(maxx - 7, 5, cm[CM_DTXT], "DV");
            }
        }

        if(SW->usemouse)
        {
            EVT e;
            ProcessMenu(&MouseMnu, &e, 1);
        }

        TTEndOutput();
    }
} /* ShowNewArea */

int OpenMsgWnd(int wid, int dep, char * title, char * msg, int x, int y)
{
    TTBeginOutput();
    hMsgCurr = WndTop();
    hMsgWnd  = WndPopUp(wid, dep, INSBDR | SHADOW, cm[IP_BTXT], cm[IP_NTXT]);

    if(hMsgWnd == NULL)
    {
        return 0;
    }

    if(title)
    {
        WndTitle(title, cm[IP_BTXT]);
    }

    if(msg)
    {
        WndWriteStr(x, y, cm[IP_NTXT], msg);
    }

    TTEndOutput();
    return 1;
}

void SendMsgWnd(char * msg, int y)
{
    WND * hCurr;

    hCurr = WndTop();

    if(!hCurr)
    {
        return;
    }

    TTBeginOutput();
    WndClearLine(y, cm[IP_NTXT]);
    WndPutsCen(y, cm[IP_NTXT], msg);
    TTEndOutput();
}

int CloseMsgWnd(void)
{
    WndClose(hMsgWnd);
    WndCurr(hMsgCurr);
    return 0;
}

/*
 *  Builds a text version of the attributes set in an _attribute
 *  structure.
 */
void MakeMsgAttrs(char * buf, struct _attributes * att, int scanned, int times_read)
{
    char * nul = "";

    sprintf(buf,
            "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
            att->priv ? "Pvt " : nul,
            att->crash ? "Cra " : nul,
            att->rcvd ? "Rcv " : nul,
            att->sent ? "Snt " : nul,
            att->attach ? "Att " : nul,
            att->killsent ? "K/s " : nul,
            att->freq ? "Frq " : nul,
            att->ureq ? "URq " : nul,
            att->hold ? "Hld " : nul,
            att->orphan ? "Orp " : nul,
            att->forward ? "Trs " : nul,
            att->local ? "Loc " : nul,
            att->direct ? "Dir " : nul,
            att->rreq ? "RRq " : nul,
            att->rcpt ? "RRc " : nul,
            att->areq ? "ARq " : nul,
            scanned ? "Scn " : nul,
            times_read > 2 ? "Read " : nul,
            att->kfs ? "Kfs " : nul,
            att->tfs ? "Tfs " : nul,
            att->as ? "A/s " : nul,
            att->immediate ? "Imm " : nul,
            att->lock ? "Lock " : nul,
            att->cfm ? "Cfm " : nul,
            att->zon ? "Zon " : nul,
            att->hub ? "Hub " : nul);
}

void ShowAddress(FIDO_ADDRESS * addr, int y)
{
    sprintf(line, "  %s", show_address(addr));
    WndPutsn(32, y, 26, cm[CM_HTXT], line);
}

void ShowNameAddress(char * name, FIDO_ADDRESS * addr, int y, int newrcvd, int nm_only)
{
    char tmp[256];

    if(addr->fidonet || (addr->notfound && !(CurArea.uucp || CurArea.news)))
    {
        /* show FidoNet-type names */
        if(nm_only)
        {
            if(name)
            {
                sprintf(line, "%s,", name);
            }
            else
            {
                strcpy(line, ", ");
            }
        }
        else
        {
            if(CurArea.netmail || (CurArea.echomail && y == 1))
            {
                if(name)
                {
                    sprintf(line, "%s, %s", name, show_address(addr));
                }
                else
                {
                    sprintf(line, ", %s", show_address(addr));
                }

                if(SW->showsystem && ST->nodepath != NULL && v7lookupsystem(addr, tmp))
                {
                    strcat(line, ", ");
                    strcat(line, tmp);
                }
            }
            else
            {
                if(name)
                {
                    sprintf(line, "%s", name);
                }
                else
                {
                    strcpy(line, " ");
                }
            }
        }

        strcpy(line + maxx - 28 - 1, " ");

        if(newrcvd)
        {
            WndPutsn(8, y, maxx - 28, cm[CM_BTXT], line);
        }
        else
        {
            WndPutsn(8, y, maxx - 28, cm[CM_HTXT], line);
        }
    }
    else
    {
        /* show Internet-type names */
        if(name == NULL)
        {
            sprintf(line, "%s", show_address(addr));
            strcpy(line + maxx - 28 - 1, " ");
            WndPutsn(8, y, maxx - 28, cm[CM_HTXT], line);
        }
        else
        {
            char * truename = NULL;
            /* Note: Normally, name should only contain the user name.
                     However, during certain stages of header editing, it might
                     also contain the e-mail address. Therefore, we parse the
                     address to make sure that really only the address is
                     showed. */
            parse_internet_address(name, NULL, &truename);
            sprintf(line, "%s (%s)", show_address(addr), name);
            strcpy(line + maxx - 28 - 1, " ");

            if(newrcvd)
            {
                WndPutsn(8, y, maxx - 28, cm[CM_BTXT], line);
            }
            else
            {
                WndPutsn(8, y, maxx - 28, cm[CM_HTXT], line);
            }

            release(truename);
        }
    }
} /* ShowNameAddress */

void ShowSubject(char * subj)
{
    WndPutsn(8, 3, 72, cm[CM_HTXT], subj);
}

void ShowAttrib(msg * m)
{
    MakeMsgAttrs(line, &m->attrib, m->scanned, m->times_read);

    if(SW->datearrived)
    {
        WndPutsn(8, 4, 52, cm[CM_HTXT], line);
    }
    else
    {
        WndPutsn(8, 4, 71, cm[CM_HTXT], line);
    }
}

/*
 *  Puts the message header onto the screen.
 */
void ShowMsgHeader(msg * m)
{
    int i, r = 0;
    unsigned long rep = 0;

    TTBeginOutput();

    if(SW->statbar)
    {
        int l;
        sprintf(line, " %s %s %c ", PROG, VERNUM VERPATCH, SC7);
        l = strlen(line);
        WndPutsn(0, maxy - 1, l - 2, cm[CM_ITXT], line);

        if(l - 2 < maxx - 1)
        {
            WndPutsn(l - 2, maxy - 1, (maxx - 1) - (l - 2), cm[CM_ITXT] | F_ALTERNATE,
                     line + (l - 2));
        }

        WndClear(maxx - 1, maxy - 1, maxx - 1, maxy - 1, cm[CM_ITXT]);
    }

    if(!m)
    {
        if(SW->dmore || !SW->statbar)
        {
            strcpy(line, "(0 of 0)");
        }
        else
        {
            sprintf(line, "0 of 0 %c", SC7);
        }
    }
    else
    {
        if(SW->dmore || !SW->statbar)
        {
            sprintf(line, "(%ld of %ld)", CurArea.current, CurArea.messages);
        }
        else
        {
            sprintf(line, "%ld of %ld %c", CurArea.current, CurArea.messages, SC7);
        }
    }

    /*
     *  The above function displays on screen the stat bar, should
     *  work on any width display.
     */
    if(SW->dmore || !SW->statbar)
    {
        WndPutsn((strlen(CurArea.description) + 4), 0, 18, cm[CM_NTXT], line);
    }
    else
    {
        int l = strlen(line);
        WndPutsn((strlen(PROG) + strlen(VERNUM VERPATCH) + 6), maxy - 1, l - 1, cm[CM_ITXT], line);

        if(l - 1 < 18)
        {
            WndPutsn((strlen(PROG) + strlen(VERNUM VERPATCH) + 6 + (l - 1)),
                     maxy - 1,
                     18 - (l - 1),
                     cm[CM_ITXT] | F_ALTERNATE,
                     line + (l - 1));
        }
    }

    /*
     *  This is used to display the current message number out of the
     *  total in the area.
     */

#if defined (MSDOS) && !defined (__FLAT__)

    if(SW->statbar)
    {
        sprintf(line, "%c %3ldK ", SC7, (long)(corerem() / 1024));
        WndPutsn(maxx - 7, maxy - 1, 1, cm[CM_ITXT] | F_ALTERNATE, line);
        WndPutsn(maxx - 6, maxy - 1, 6, cm[CM_ITXT], line + 1);
    }

#endif

    if(SW->showtime)
    {
        WndPrintf(40, 5, cm[CM_HTXT], " %s ", itime(time(NULL)));
    }

    if(m == NULL)
    {
        TTEndOutput();
        return;
    }

    /* show the message links, up and down */
    if(m->replyto)
    {
        sprintf(line, "%6ld", m->replyto);
    }
    else
    {
        strcpy(line, "       ");
    }

    line[6] = '\0';
    WndPrintf(maxx - 17, 2, cm[CM_FTXT], line);
    WndPrintf(maxx - 11, 2, cm[CM_FTXT] | F_ALTERNATE, "%c", (m->replyto) ? SC15 : ' ');

    for(i = 0; i < 10; i++)
    {
        if(m->replies[i] != 0)
        {
            if(!rep)
            {
                rep = m->replies[i];
            }

            r++;
        }
    }

    if(rep && (r == 1))
    {
        sprintf(line, "%c%ld      ", SC14, rep);
        line[8] = '\0';
        WndPutsn(maxx - 10, 2, 1, cm[CM_FTXT] | F_ALTERNATE, line);
        WndWriteStr(maxx - 9, 2, cm[CM_FTXT], line + 1);
    }
    else if(rep && (r > 1))
    {
        sprintf(line, "%c%c%ld     ", SC14, SC14, rep);
        line[8] = '\0';
        WndPutsn(maxx - 10, 2, 2, cm[CM_FTXT] | F_ALTERNATE, line);
        WndWriteStr(maxx - 8, 2, cm[CM_FTXT], line + 2);
    }
    else
    {
        strcpy(line, "        ");
        WndWriteStr(maxx - 10, 2, cm[CM_FTXT], line);
    }

    /* show the rest of the header information */
    ShowNameAddress(m->isfrom, &m->from, 1, 0, 0);
    ShowNameAddress(m->isto, &m->to, 2, m->newrcvd, 0);
    ShowSubject(m->subj);
    ShowAttrib(m);
    /* show the date of creation and the date of arrival */
    WndPrintf(maxx - 20, 1, cm[CM_HTXT], "%s", mtime(m->timestamp));

    if(SW->datearrived && m->time_arvd != 0 && m->time_arvd != m->timestamp)
    {
        WndPrintf(maxx - 20, 4, cm[CM_HTXT], "%s", mtime(m->time_arvd));
    }
    else
    {
        WndPutsn(maxx - 20, 4, 19, cm[CM_HTXT], " ");
    }

    TTEndOutput();
} /* ShowMsgHeader */

void PutLine(LINE * l, int y)
{
    int Attr =
        (l->block) ? cm[CM_BTXT] : (l->quote) ? cm[CM_QTXT] : (l->hide) ? cm[CM_KTXT] : (l->templt)
        ?
        cm[CM_TTXT] : cm[CM_NTXT];
    char * s, * pcr, * plf;
    int len;

    strcpy(line, l->text);

#ifdef UNIX

    if(*line == '\001')
    {
        *line = '@';
    }

#endif

    pcr = strchr(line, '\r');
    plf = strchr(line, '\n');

    if(pcr == NULL)
    {
        s = plf;
    }
    else if(plf == NULL)
    {
        s = pcr;
    }
    else if(pcr < plf)
    {
        s = pcr;
    }
    else
    {
        s = plf;
    }

    if(s != NULL)
    {
        if(SW->showcr)
        {
            *s = SC20;
        }
        else
        {
            *s = '\0';
        }

        len = s - line;
    }
    else
    {
        len = strlen(line);
    }

    if(len)
    {
        if(len > maxx)
        {
            len = maxx;
        }

        WndPutsn(0, y, len, Attr, line);
    }

    if(SW->showcr)
    {
        if(line[len] && len < maxx)
        {
            WndPutsn(len, y, 1, Attr | F_ALTERNATE, line + len);
            len++;
        }

        if(SW->showeol && len < maxx)
        {
            WndPrintf(len, y, Attr | F_ALTERNATE, "%c", SC21);
            len++;
        }
    }

    if(len < maxx)
    {
        WndPutsn(len, y, maxx - len, Attr, " ");
    }
} /* PutLine */

void MsgScroll(int Dir)
{
    if(SW->statbar)
    {
        WndScroll(0, 6, maxx - 1, maxy - 2, Dir);
    }
    else
    {
        WndScroll(0, 6, maxx - 1, maxy - 1, Dir);
    }
}

void Go_Up(void)
{
    if(!message)
    {
        return;
    }

    if(top == NULL)
    {
        return;
    }

    TTBeginOutput();

    if(top->prev)
    {
        while(top->prev)   /* we want to skip hidden lines */
        {
            top = top->prev;

            if(SW->shownotes || (*(top->text) != '\01'))
            {
                MsgScroll(0);
                PutLine(top, 6);
                break;     /* stop when we atctually write something */
            }
        }
    }

    TTEndOutput();
} /* Go_Up */

void Go_Dwn(void)
{
    int i = 1;

    scrHdrLen = (SW->statbar) ? 7 : 6;

    if(!message)
    {
        return;
    }

    if(top == NULL)
    {
        return;
    }

    for(bottom = top; i < (maxy - scrHdrLen); bottom = bottom->next)
    {
        if(!bottom->next)
        {
            break;
        }

        i++;
    }

    if(i == (maxy - scrHdrLen))
    {
        if(bottom->next)
        {
            TTBeginOutput();
            bottom = bottom->next;
            top    = top->next;
            MsgScroll(1);

            if(SW->statbar)
            {
                PutLine(bottom, maxy - 2);
            }
            else
            {
                PutLine(bottom, maxy - 1);
            }

            TTEndOutput();
        }
    }
} /* Go_Dwn */

void Go_PgDwn(void)
{
    int i;

    scrHdrLen = (SW->statbar) ? 7 : 6;

    if(!message)
    {
        return;
    }

    if(top == NULL)
    {
        return;
    }

    for(i = 0; i < (maxy - scrHdrLen); i++)
    {
        if(!top->next)
        {
            break;
        }

        top = top->next;
    }

    if(i != 0)
    {
        RefreshMsg(top, 6);
    }
} /* Go_PgDwn */

void Go_PgUp(void)
{
    int i;

    scrHdrLen = (SW->statbar) ? 7 : 6;

    if(!message)
    {
        return;
    }

    if(top == NULL)
    {
        return;
    }

    for(i = 0; i < (maxy - scrHdrLen); i++)
    {
        if(!top->prev)
        {
            break;
        }

        top = top->prev;
    }

    if(i != 0)
    {
        RefreshMsg(top, 6);
    }
} /* Go_PgUp */

void RefreshMsg(LINE * line, int y)
{
    LINE * t      = line;
    static LINE l =
    {
        " ", 0, 0, 0, 0, 0, NULL, NULL
    };
    int i = y;

    top = line;

    if(y >= maxy)
    {
        return;
    }

    if(!line)
    {
        TTBeginOutput();

        if(SW->statbar)
        {
            WndClear(0, y, maxx - 1, maxy - 2, cm[CM_NTXT]);
        }
        else
        {
            WndClear(0, y, maxx - 1, maxy - 1, cm[CM_NTXT]);
        }

        TTEndOutput();
        return;
    }

    TTBeginOutput();

    if(SW->statbar)
    {
        while(t && i <= maxy - 2)
        {
            PutLine(t, i);
            t = t->next;
            i++;
        }

        if(i <= maxy - 2)
        {
            while(i <= maxy - 2)
            {
                PutLine(&l, i);
                i++;
            }
        }
    }
    else
    {
        while(t && i <= maxy - 1)
        {
            PutLine(t, i);
            t = t->next;
            i++;
        }

        if(i <= maxy - 1)
        {
            while(i <= maxy - 1)
            {
                PutLine(&l, i);
                i++;
            }
        }
    }

    TTEndOutput();
} /* RefreshMsg */
