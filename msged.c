/*
 *  MSGED.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Msged Mail Reader.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef UNIX
#include <process.h>
#endif

#include "addr.h"
#include "nedit.h"
#include "maintmsg.h"
#include "misc.h"
#include "keys.h"
#include "winsys.h"
#include "menu.h"
#include "main.h"
#include "dialogs.h"
#include "bmg.h"
#include "screen.h"
#include "memextra.h"
#include "strextra.h"
#include "areas.h"
#include "version.h"
#include "specch.h"
#include "dosmisc.h"
#include "config.h"
#include "list.h"
#include "help.h"
#include "getopts.h"
#include "keycode.h"
#include "helpcmp.h"
#include "helpinfo.h"
#include "system.h"
#include "msged.h"
#include "maincmds.h"
#include "readmail.h"
#include "makemsgn.h"
#include "nshow.h"
#include "charset.h"
#include "wrap.h"
#include "textfile.h"

#ifdef MSDOS
#ifdef USE_CRITICAL
#include "critical.h"
#endif
#if !defined(NODOSSWAP) && !defined(__FLAT__)
#include <dos.h>  /* FP_OFF macro */
#include "spawn.h"
#endif
#endif

#ifdef USE_MSGAPI
#include "msg.h"
#endif

#ifndef randomize
#define randomize() srand((unsigned) time(NULL))
#endif

/* prototypes */

static void highest(void);
static void gotomsg(unsigned long i);
static void pm_next_area(void);

/* local/global variables */

int scan = 0;                   /* set scan = 1 to scan for new mail at startup */

static int endMain = 0;
static int errorlevel = 0;
int direction = RIGHT;          /* travel direction in msgbase */
char msgbuf[BUFLEN];            /* message buffer used for reading */
int msgederr = 0;               /* errno for msged */
int set_rcvd = 1;               /* used to tell readmsg() not to set rcvd */

/* only used readmail between.c and msged.c */
static unsigned long root = 0;       /* root message of a thread */
static unsigned long back = 0;       /* Where you were before you said "go
                                      * root" */
static unsigned long areastart = 0;  /* msg number we started on in this
                                      * area */
static unsigned long lastfound = 0;  /* msg number last found */
static unsigned long oldmsg = 0;
static int command;

#if defined(MSDOS) && defined(__TURBOC__)
extern unsigned _stklen = 16384;
#endif

static void delete(void)
{
    deletemsg();
}

static void move(void)
{
    if (message != NULL)
    {
        movemsg();
    }
}

static void outtxt(void)
{
    if (message != NULL)
    {
        writetxt();
    }
}

static void set(void)
{
    set_switch();
}

static void chngaddr(void)
{
    change_curr_addr();
}

static void chngname(void)
{
    change_username();
}

static void chngnodel(void)
{
    change_nodelist();
}

static void do_help(void)
{
    show_help();
}

static void first(void)
{
    CurArea.current = CurArea.first;
}

char *r_getbind(unsigned int key)
{
    unsigned int i = 0;
    void (*action) (void);

    if (key & 0xff)
    {
        action = mainckeys[key & 0xff];
    }
    else
    {
        action = mainakeys[(key >> 8) & 0xff];
    }

    while (maincmds[i].label != NULL && action != maincmds[i].action)
    {
        i++;
    }

    return maincmds[i].label;
}

char *r_getlabels(int i)
{
    return maincmds[i].label;
}

void r_assignkey(unsigned int key, char *label)
{
    unsigned int i = 0;

    while ((maincmds[i].label != NULL) &&
      (strncmp(maincmds[i].label, label, strlen(maincmds[i].label)) != 0))
    {
        i++;
    }

    if (maincmds[i].label != NULL)
    {
        if (key & 0xff)
        {
            mainckeys[key & 0xff] = maincmds[i].action;
        }
        else
        {
            mainakeys[(key >> 8) & 0xff] = maincmds[i].action;
        }
    }
}

void dispose(msg * message)
{
    if (message == NULL)
    {
        return;
    }

    if (message->text != NULL)
    {
        message->text = clearbuffer(message->text);
    }

    release(message->isto);
    release(message->isfrom);
    release(message->reply);
    release(message->msgid);
    release(message->subj);
    release(message->to.domain);
    release(message->from.domain);
    release(message->replyarea);
    release(message->charset_name);
    release(message);
}

msg *KillMsg(msg * m)
{
    dispose(m);
    return NULL;
}

/* edithdr - lets the user edit the header only */

static void edithdr(void)
{
    int q;

    if (message == NULL)
    {
        return;
    }

    q = 0;
    while (!q)
    {
        if (EditHeader(message) == Key_Esc)
        {
            if (confirm("Cancel?"))
            {
                message = KillMsg(message);
                return;
            }
        }
        else
        {
            q = 1;
        }
    }
    MsgWriteHeader(message, WR_HEADER);
    message = KillMsg(message);
}

/* cleanup - make sure you exit straight after calling this! */

void cleanup(char *msg,...)
{
    AREA *a;

    if (CurArea.status)
    {
        highest();
        AreaSetLast(&CurArea);
        MsgAreaClose();
    }
    setcwd(ST->home);
    WndClose(hMnScr);
    TTgotoxy(term.NRow - 1, 0);
    TTclose();

    for (a = &(arealist[SW->area = 0]); SW->area < SW->areas; a = &(arealist[++SW->area]))
    {
        errorlevel |= (a->netmail && a->new) ? 0x01 : 0;
        errorlevel |= (a->echomail && a->new) ? 0x02 : 0;
        errorlevel |= (a->uucp && a->new) ? 0x04 : 0;
        errorlevel |= (a->news && a->new) ? 0x08 : 0;
        errorlevel |= (a->local && a->new) ? 0x10 : 0;
    }

    if (msg)
    {
        va_list ap;
        va_start(ap, msg);
        putchar('\n');
        vprintf(msg, ap);
        va_end(ap);
    }

    destroy_charset_maps();
    DeinitMem();
#ifdef USE_MSGAPI
    MsgApiTerm();
#endif
    TTCurSet(1);
/*#ifdef UNIX
    TTScolor(7);
#endif */

#ifdef USE_CRITICAL
    remove24h();
#endif
}

static void quit(void)
{
    endMain = 1;
}

/* search - search header and message text for keyword */

static void search(void)
{
    LINE *l = NULL;
    static char prompt[256] = "";
    static char patstr[256] = "";
    char tempstr[60];
    char totalmsg[12];          /* to allow for upto 4,294,967,295 msgs */
    msg *m = NULL;
    unsigned long tmp = 0;
    int done = 0;
    unsigned int boxwidth;      /* boxwidth of search display */
    unsigned int mlength;

    if (!GetString(" Keyword Search (message header and text) ", "Search for:", prompt, 64))
    {
        return;
    }

    if (strlen(prompt) == 0)
    {
        return;
    }

    sprintf(tempstr, " Searching for \"%.30s\" ", prompt);

    sprintf(totalmsg, "%ld", CurArea.messages);
    mlength = (2 * strlen(totalmsg)) + 26;
    if (mlength <= (strlen(tempstr) + 18))
    {
        boxwidth = (strlen(tempstr) + 18);
    }
    else
    {
        boxwidth = mlength;
    }

    if (!OpenMsgWnd(boxwidth, 6, tempstr, NULL, 0, 0))
    {
        return;
    }

    message = KillMsg(message);

    /*
     *  We use this to ensure the rcvd bit doesn't get set.  Possibly a
     *  mistake if the msg found is addressed to the user...
     */

    set_rcvd = 0;

    SendMsgWnd("Press Esc to stop", 0);

    if ((stricmp(prompt, patstr) == 0) && (lastfound == CurArea.current) &&
      (CurArea.current < CurArea.messages))
    {
        if (direction == RIGHT)
        {
            tmp = CurArea.current + 1;
        }
        else
        {
            if (CurArea.current > 1)
            {
                tmp = CurArea.current - 1;
            }
        }
        bmg_setsearch(prompt);
    }
    else
    {
        tmp = CurArea.current;
        lastfound = 0;
        strcpy(patstr, prompt);
        bmg_setsearch(prompt);
    }

    for (;;)
    {
        if ((direction == RIGHT) && (tmp > CurArea.messages))
        {
            break;
        }
        else if (tmp == 0)
        {
            break;
        }

        /* check for event here */

        if (done == TRUE || (KeyHit() && GetKey() == Key_Esc))
        {
            break;
        }

        l = NULL;

        if ((m = readmsg(tmp)) != NULL)
        {
            sprintf(tempstr, "Reading message #%ld of #%ld", tmp, CurArea.messages);
            SendMsgWnd(tempstr, 1);

            if (bmg_search(m->isto) != NULL)
            {
                done = TRUE;
                break;
            }

            if (bmg_search(m->isfrom) != NULL)
            {
                done = TRUE;
                break;
            }

            if (bmg_search(m->subj) != NULL)
            {
                done = TRUE;
                break;
            }

            l = m->text;

            while ((l != NULL) && (done == FALSE))
            {
                if (l->text && (strlen(l->text) > 0))
                {
                    if (bmg_search(l->text) != NULL)
                    {
                        done = TRUE;
                        break;
                    }
                }
                l = l->next;
            }
            if (done == TRUE)
            {
                break;
            }

            dispose(m);
            m = NULL;
        }

        if (direction == RIGHT)
        {
            tmp++;
        }
        else
        {
            tmp--;
        }
    }

    CloseMsgWnd();

    set_rcvd = 1;               /* readmsg() is now free to set it */

    if (done)
    {
        CurArea.current = tmp;
        lastfound = tmp;
        oldmsg = tmp;
        message = m;

        TTBeginOutput();
        ShowMsgHeader(message);

        if (l != NULL)
        {
            l->block = TRUE;
            RefreshMsg(l, 6);
        }
        else
        {
            RefreshMsg(message->text, 6);
        }
        TTEndOutput();
    }
    else
    {
        dispose(m);
    }
}

/* hdrsearch - search header for keyword */

static void hdrsearch(void)
{
    LINE *l = NULL;
    static char prompt[256] = "";
    static char patstr[256] = "";
    char tempstr[40];
    char totalmsg[12];          /* to allow for upto 4,294,967,295 msgs */
    msg *m = NULL;
    unsigned long tmp = 0;
    int done = 0;
    unsigned int boxwidth;      /* boxwidth of search display */
    unsigned int mlength;

    if (!GetString(" Keyword Search (message header) ", "Search for:", prompt, 64))
    {
        return;
    }

    if (strlen(prompt) == 0)
    {
        return;
    }

    sprintf(tempstr, " Searching for \"%.30s\" ", prompt);

    sprintf(totalmsg, "%ld", CurArea.messages);
    mlength = (2 * strlen(totalmsg)) + 26;
    if (mlength <= (strlen(tempstr) + 18))
    {
        boxwidth = (strlen(tempstr) + 18);
    }
    else
    {
        boxwidth = mlength;
    }

    if (!OpenMsgWnd(boxwidth, 6, tempstr, NULL, 0, 0))
    {
        return;
    }

    message = KillMsg(message);

    /*
     *  We use this to ensure the rcvd bit doesn't get set.  Possibly a
     *  mistake if the msg found is addressed to the user...
     */

    set_rcvd = 0;

    SendMsgWnd("Press Esc to stop", 0);

    if ((stricmp(prompt, patstr) == 0) && (lastfound == CurArea.current) &&
      (CurArea.current < CurArea.messages))
    {
        if (direction == RIGHT)
        {
            tmp = CurArea.current + 1;
        }
        else
        {
            if (CurArea.current > 1)
            {
                tmp = CurArea.current - 1;
            }
        }
        bmg_setsearch(prompt);
    }
    else
    {
        tmp = CurArea.current;
        lastfound = 0;
        strcpy(patstr, prompt);
        bmg_setsearch(prompt);
    }

    for (;;)
    {
        if ((direction == RIGHT) && (tmp > CurArea.messages))
        {
            break;
        }
        else if (tmp == 0)
        {
            break;
        }

        /* check for event here */

        if (done == TRUE || (KeyHit() && GetKey() == Key_Esc))
        {
            break;
        }

        l = NULL;

        if ((m = MsgReadHeader(tmp, RD_HEADER_BRIEF)) != NULL)
        {
            sprintf(tempstr, "Reading message #%ld of #%ld", tmp, CurArea.messages);
            SendMsgWnd(tempstr, 1);

            if (bmg_search(m->isto) != NULL)
            {
                done = TRUE;
                break;
            }

            if (bmg_search(m->isfrom) != NULL)
            {
                done = TRUE;
                break;
            }

            if (bmg_search(m->subj) != NULL)
            {
                done = TRUE;
                break;
            }

            if (done == TRUE)
            {
                break;
            }

            dispose(m);
            m = NULL;
        }

        if (direction == RIGHT)
        {
            tmp++;
        }
        else
        {
            tmp--;
        }
    }

    CloseMsgWnd();

    set_rcvd = 1;               /* readmsg() is now free to set it */

    if (done)
    {
        dispose(m);
        m = readmsg(tmp);

        message = m;
        CurArea.current = tmp;
        lastfound = tmp;
        oldmsg = tmp;

        TTBeginOutput();

        ShowMsgHeader(message);

        if (l != NULL)
        {
            l->block = TRUE;
            RefreshMsg(l, 6);
        }
        else
        {
            RefreshMsg(message->text, 6);
        }

        TTEndOutput();
    }
    else
    {
        dispose(m);
    }
}

/* spmail - searchs for personal mail in a single area */

static void spmail(void)
{
    LINE *l = NULL;
    static char prompt[256] = "";
    static char patstr[256] = "";
    char tempstr[40];
    char totalmsg[12];          /* to allow for upto 4,294,967,295 msgs */
    msg *m = NULL;
    unsigned long tmp;
    int done = 0;
    int boxwidth;               /* boxwidth for search display */
    int mlength;

    strcpy(prompt, ST->username);

    if (strlen(prompt) == 0)
    {
        return;
    }

    sprintf(tempstr, " Searching for \"%.30s\" ", prompt);
    sprintf(totalmsg, "%ld", CurArea.messages);
    mlength = (2 * strlen(totalmsg)) + 26;

    if (mlength <= (strlen(tempstr) + 18))
    {
        boxwidth = (strlen(tempstr) + 18);
    }
    else
    {
        boxwidth = mlength;
    }

    if (!OpenMsgWnd(boxwidth, 6, tempstr, NULL, 0, 0))
    {
        return;
    }

    message = KillMsg(message);

    /*
     *  We use this to ensure the rcvd bit doesn't get set.  Possibly a
     *  mistake if the msg found is addressed to the user...
     */

    set_rcvd = 0;

    SendMsgWnd("Press Esc to stop", 0);

    if ((stricmp(prompt, patstr) == 0) && (lastfound == CurArea.current))
    {
        tmp = CurArea.current + 1;
    }
    else
    {
        tmp = CurArea.current + 1;
        lastfound = 0;
        strcpy(patstr, prompt);
        bmg_setsearch(prompt);
    }

    for (; tmp <= CurArea.messages; tmp++)
    {
        /* check for event here */

        if (done == TRUE || (KeyHit() && (GetKey()) == Key_Esc))
        {
            break;
        }

        l = NULL;

        if ((m = readmsg(tmp)) != NULL)
        {
            sprintf(tempstr, "Area: %s", CurArea.tag);
            SendMsgWnd(tempstr, 1);
            sprintf(tempstr, "Reading message #%ld of #%ld", tmp, CurArea.messages);
            SendMsgWnd(tempstr, 2);

            bmg_setsearch(ST->username);

            if (bmg_search(m->isto) != NULL)
            {
                done = TRUE;
                break;
            }

            if (done == TRUE)
            {
                break;
            }

            dispose(m);
            m = NULL;
        }
    }

    CloseMsgWnd();

    set_rcvd = 1;               /* readmsg() is now free to set it */

    if (done)
    {
        CurArea.current = tmp;
        lastfound = tmp;
        oldmsg = tmp;
        message = m;

        TTBeginOutput();

        ShowMsgHeader(message);

        if (l != NULL)
        {
            l->block = TRUE;
            RefreshMsg(l, 6);
        }
        else
        {
            RefreshMsg(message->text, 6);
        }

        TTEndOutput();
    }
    else
    {
        dispose(m);
    }
}

/* pmail - personal mail scan */

static int x = -1;
static int firstarea = 0;

static void pmail(void)
{
    LINE *l = NULL;
    static char prompt[256] = "";
    static char patstr[256] = "";
    static char username[256] = "";
    char tempstr[40];
    char totalmsg[12];          /* to allow upto 4,294,967,295 msgs */
    msg *m = NULL;
    unsigned long tmp;
    int done = 0;
    int boxwidth;               /* boxwidth for search display */
    int mlength;
    unsigned int chkkey = 0;
    int again = 1;

    while (again)
    {
        again = 0;
        if (x == -1)
        {
            firstarea = SW->area;
            strcpy(username, ST->username);
        }

        x += 1;

        strcpy(prompt, username);

        if (strlen(prompt) == 0)
        {
            return;
        }

        sprintf(tempstr, " Searching for \"%.30s\" ", prompt);

        sprintf(totalmsg, "%ld", CurArea.messages);
        mlength = (2 * strlen(totalmsg)) + 26;
        if (mlength <= (strlen(tempstr) + 18))
        {
            boxwidth = (strlen(tempstr) + 18);
        }
        else
        {
            boxwidth = mlength;
        }

        if ((long)arealist[SW->area].messages != (long)arealist[SW->area].lastread)
        {
            if (!OpenMsgWnd(boxwidth, 6, tempstr, NULL, 0, 0))
            {
                return;
            }
        }

        message = KillMsg(message);

        /*
         *  We use this to ensure the rcvd bit doesn't get set.  Possibly a
         *  mistake if the msg found is addressed to the user...
         */

        set_rcvd = 0;

        if ((long)arealist[SW->area].messages != (long)arealist[SW->area].lastread)
        {
            SendMsgWnd("Press Esc to stop", 0);
        }

        if (stricmp(prompt, patstr) == 0 && lastfound == CurArea.current)
        {
            tmp = CurArea.current + 1;
        }
        else
        {
            tmp = CurArea.current + 1;
            lastfound = 0;
            strcpy(patstr, prompt);
            bmg_setsearch(prompt);
        }

        for (; tmp <= CurArea.messages; tmp++)
        {
            /* check for event here */

            if (done == TRUE || (KeyHit() && (chkkey = GetKey()) == Key_Esc))
            {
                break;
            }

            l = NULL;

            if ((m = readmsg(tmp)) != NULL)
            {
                sprintf(tempstr, "Area: %s", CurArea.tag);

                if ((long)arealist[SW->area].messages != (long)arealist[SW->area].lastread)
                {
                    SendMsgWnd(tempstr, 1);
                }

                sprintf(tempstr, "Reading message #%ld of #%ld", tmp, CurArea.messages);

                if ((long)arealist[SW->area].messages != (long)arealist[SW->area].lastread)
                {
                    SendMsgWnd(tempstr, 2);
                }

                bmg_setsearch(username);
                if (bmg_search(m->isto) != NULL)
                {
                    done = TRUE;
                    break;
                }

                if (done == TRUE)
                {
                    break;
                }

                dispose(m);
                m = NULL;
            }
        }

        if ((long)arealist[SW->area].messages != (long)arealist[SW->area].lastread)
        {
            CloseMsgWnd();
        }

        set_rcvd = 1;           /* readmsg() is now free to set it */

        if (done)
        {
            CurArea.current = tmp;
            lastfound = tmp;
            oldmsg = tmp;
            message = m;

            ShowMsgHeader(message);

            if (l != NULL)
            {
                l->block = TRUE;
                RefreshMsg(l, 6);
            }
            else
            {
                RefreshMsg(message->text, 6);
            }
        }
        else
        {
            dispose(m);
            pm_next_area();
            if (SW->area != firstarea && chkkey != Key_Esc)
            {
                again = 1;
            }
            else
            {
                set_area(firstarea);
                SetupArea();
                firstarea = 0;
                x = -1;
            }
        }
    }
}

static void gotomsg0(void)
{
    gotomsg(0L);
}

/* confirm - Ask if the user is doing something on purpose (Never! :-) */

int confirm(char *option)       /* Allows more meaningful confirm messages */
{
    if (!SW->confirmations)
    {
        return TRUE;
    }
    return ChoiceBox("", option, "Yes", "No", NULL) == ID_ONE ? TRUE : FALSE;
}

/* SetupArea - Sets up the operating vars for the current area */

void SetupArea(void)
{
    lastfound = 0;
    direction = RIGHT;
    back = CurArea.current;
    root = CurArea.current;
    areastart = CurArea.current;

    /*
     *  Set this area up with it's info: The template, The username,
     *  lastread (fido areas) name, useroffset (squish areas).
     */

    release(ST->username);
    release(ST->template);
    release(ST->lastread);

    ST->username = xstrdup(user_list[CurArea.username].name);
    SW->useroffset = user_list[CurArea.username].offset;
    ST->lastread = xstrdup(user_list[CurArea.username].lastread);

    if (templates != NULL)
    {
        ST->template = xstrdup(templates[CurArea.template]);
    }
}

/* set_area - Opens a new area & sets all the vars */

void set_area(int newarea)
{
    int done = 0;
    int ret;

    if (CurArea.status)         /* close current area, if open */
    {
        highest();
        AreaSetLast(&CurArea);
        MsgAreaClose();
    }
    message = KillMsg(message);
    SW->area = newarea;

    while (!CurArea.status && !done)
    {
        CurArea.messages = MsgAreaOpen(&CurArea);

        if (!CurArea.status)
        {
            ret = ChoiceBox(" Error ", "Error opening area;", "Retry",
              "New area", "Cancel");

            switch (ret)
            {
            case ID_TWO:
                SW->area = selectarea("Pick New Area", SW->area);
                break;

            case ID_THREE:
                done = TRUE;
                break;

            default:
                break;
            }
        }
        else
        {
            done = TRUE;
        }
    }
    ShowNewArea();            /* display the new area */
}

/*
 *  Scans all the areas for new mail, calling scan_areas(), but is
 *  callable from other modules.
 */

void area_scan(int all)
{
    scan_areas(all);
}

/*
 *  Scans all the areas for new mail, calling al_scan_areas(), but is
 *  callable from other modules.  Calls from Arealist.
 */

void arealist_area_scan(int all)
{
    al_scan_areas(all);
}

/*
 * void ...(void) wrapper functions for void scan_areas(int)
 */

void scan_all_areas(void)
{
  scan_areas(1);
}

void scan_unscanned_areas(void)
{
    scan_areas(0);
}

/*
 *  Scans all the areas for new messages.  Saves the current area and
 *  returns to it.
 *  Parameter: int all: =1: scan all areas, =0: scan unscanned areas
 */



static void scan_areas(int all)
{
    char line[255];
    char temp[20];
    int a;
    int l;
    int x,y;

    TTgetxy(&x,&y);

    a = SW->area;
    l = strlen(PROG) + strlen(VERSION CLOSED);
    if (!SW->dmore)
    {
        l += sprintf(temp, "%ld of %ld", CurArea.current,  CurArea.messages);
    }

    if (!SW->statbar)
    {
        if (!OpenMsgWnd(50, 6, " Scanning areas for new messages ", NULL, 0, 0))
        {
            return;
        }
        SendMsgWnd("Press Esc to stop", 0);
    }

    if (CurArea.status)
    {
        highest();
        AreaSetLast(&CurArea);
        MsgAreaClose();
    }

    if (all)  /* reset the "scanned" flag of all areas */
    {
        for (SW->area = 0; SW->area < SW->areas; SW->area++)
        {
            arealist[SW->area].scanned = 0;
        }
    }


    for (SW->area = 0; SW->area < SW->areas; SW->area++)
    {
        if ((!all) && (arealist[SW->area].scanned))
        {
            continue;
        }

        if (SW->statbar)
        {
            TTBeginOutput();
            sprintf(line, "%.40s", CurArea.description);
            if (!SW->dmore)
            {
                line[maxx - l - 20] = '\0';

                WndPutsn(l + 9,  maxy - 1, maxx - l - 10, cm[CM_ITXT], "Scanning:");
                WndWriteStr(l + 19,  maxy - 1, cm[CM_ITXT], line);
            }
            else
            {
                line[79 - strlen(PROG) -
                     strlen(VERSION CLOSED) - 16] = '\0';

                WndPutsn(l + 6, maxy - 1, maxx - l - 7, cm[CM_ITXT], "Scanning:");
                WndWriteStr(l + 16, maxy - 1, cm[CM_ITXT], line);
            }
            TTEndOutput();
        }
        else
        {
            sprintf(line, "%.40s", CurArea.description);
            SendMsgWnd(line, 1);
        }

        if (KeyHit() && GetKey() == Key_Esc)
        {
            break;
        }

        CurArea.messages = MsgAreaOpen(&CurArea);

        if (CurArea.status)
        {
            MsgAreaClose();
        }
    }

    TTBeginOutput();

    if (!SW->statbar)
    {
        CloseMsgWnd();
    }

    SW->area = a;
    CurArea.messages = MsgAreaOpen(&CurArea);
    if (SW->statbar)
    {
        if (!SW->dmore)
        {
            WndPutsn(l + 9, maxy - 1, maxx - l - 10, cm[CM_ITXT], " ");
        }
        else
        {
            WndPutsn(l + 6, maxy - 1, maxx - l - 7, cm[CM_ITXT], " ");
        }
    }

#if defined(MSDOS) && !defined(__FLAT__)
    if (SW->statbar)
    {
        sprintf(line, "%c %3ldK ", SC7, (long)(corerem() / 1024));
        WndPutsn(maxx - 7, maxy - 1, 7, cm[CM_ITXT], line);
    }
#endif

    if (SW->statbar && !SW->dmore)
    {
        sprintf(line, "%ld of %ld %c", CurArea.current, CurArea.messages, SC7);
        WndPutsn((strlen(PROG) + strlen(VERSION CLOSED) + 6), maxy - 1, 18, cm[CM_ITXT], line);
    }
    TTgotoxy(x,y);

    TTEndOutput();
}

/*
 *  Scans all areas for new messages, from Arealist screen.  Saves the
 *  current area and returns to it.
 */

static void al_scan_areas(int all)
{
    char line[255];
    int a = SW->area;

    if (!OpenMsgWnd(50, 6, " Scanning areas for new messages ", NULL, 0, 0))
    {
        return;
    }

    SendMsgWnd("Press Esc to stop", 0);

    if (CurArea.status)
    {
        highest();
        AreaSetLast(&CurArea);
        MsgAreaClose();
    }

    if (all)
    {
        for (SW->area = 0; SW->area < SW->areas; SW->area++)
        {
            arealist[SW->area].scanned = 0;
        }
    }

    for (SW->area = 0; SW->area < SW->areas; SW->area++)
    {
        if ((!all) && arealist[SW->area].scanned)
        {
            continue;
        }

        sprintf(line, "%.40s", CurArea.description);
        SendMsgWnd(line, 1);

        if (KeyHit() && GetKey() == Key_Esc)
        {
            break;
        }


        CurArea.messages = MsgAreaOpen(&CurArea);

        if (CurArea.status)
        {
            MsgAreaClose();
        }

    }
    CloseMsgWnd();
    SW->area = a;
}

/* next_area - goes to the next area with unread messages */

static void next_area(void)
{
    int NewArea;
    int OldArea;

    if (SW->areas < 2)
    {
        return;
    }

    OldArea = SW->area;

    NewArea = (SW->area + 1) % SW->areas;
    while (((long)arealist[NewArea].messages <=
      (long)arealist[NewArea].lastread) && arealist[NewArea].scanned)
    {
        NewArea = (NewArea + 1) % SW->areas;
        if (NewArea == OldArea)
        {
            ChoiceBox(" Notice ", "There are no more unread messages in this area", "Ok", NULL, NULL);
            break;
        }
    }

    set_area(NewArea);
    SetupArea();
}

static void pm_next_area(void)
{
    int NewArea;

    if (SW->areas < 2)
    {
        return;
    }

    NewArea = (SW->area + 1) % SW->areas;
    set_area(NewArea);
    SetupArea();
}

/* prev_area - goes to the previous area with unread messages */

static void prev_area(void)
{
    int OldArea;
    int NewArea;

    if (SW->areas < 2)
    {
        return;
    }

    OldArea = SW->area;
    NewArea = SW->area;

    NewArea--;
    NewArea = (NewArea < 0) ? SW->areas - 1 : NewArea;
    while ((((long)arealist[NewArea].messages -
            (long)arealist[NewArea].lastread) <= 0) && arealist[NewArea].scanned)
    {
        NewArea--;
        NewArea = (NewArea < 0) ? SW->areas - 1 : NewArea;
        if (NewArea == OldArea)
        {
            NewArea--;
            NewArea = (NewArea < 0) ? SW->areas - 1 : NewArea;
            break;
        }
    }

    set_area(NewArea);
    SetupArea();
}

/* highest - sets the higest read message */

static void highest(void)
{
    CurArea.lastread = min(CurArea.current, CurArea.messages);
    root = CurArea.current;
}

/* left - goes one message to the left */

static void left(void)
{
    direction = LEFT;
    if (CurArea.current > 1)
    {
        CurArea.current--;
    }
    root = CurArea.current;
}

/* right - goes one message to the right */

static void right(void)
{
    direction = RIGHT;
    if (CurArea.current < CurArea.messages)
    {
        CurArea.current++;
    }
    else if (SW->rightnextunreadarea)
    {
        next_area();
    }
    highest();
}

/*
 *  Gets a number from the user and goes to that message
 *  number (if valid).
 */

static void gotomsg(unsigned long i)
{
    EVT e;
    WND *hWnd, *hCurr;
    char buf[10];
    int done = 0;
    int disp = 1;
    int ret;
    int pos;

    TTBeginOutput();

    hCurr = WndTop();
    hWnd = WndPopUp(30, 6, INSBDR | SHADOW, cm[IP_BTXT], cm[IP_NTXT]);
    WndTitle(" Jump to Message ", cm[IP_NTXT]);
    WndWriteStr(1, 1, cm[IP_NTXT], "Message #");

    TTEndOutput();

    if (i != 0)
    {
        sprintf(buf, "%lu", i);
    }
    else
    {
        strcpy(buf, "");
    }
    pos = strlen(buf);

    while (!done)
    {
        ret = WndGetLine(17, 1, 6, buf, cm[IP_ETXT], &pos, 0, 0, disp, &e);
        switch (e.msgtype)
        {
        case WND_WM_CHAR:
            switch (ret)
            {
            case Key_Ent:
                i = atoi(buf);

                if (i == 0)
                {
                    done = 1;
                    i = CurArea.current;
                }
                else
                {
                    if (i > 0 && i <= CurArea.messages)
                    {
                        done = 1;
                    }
                }
                break;

            case Key_Esc:
                done = 1;
                i = CurArea.current;
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
        disp = 0;
    }
    WndClose(hWnd);
    WndCurr(hCurr);
    CurArea.current = i;
    highest();
}

/* newarea - gets a new area from the user and goes to it */

static int newarea(void)
{
    int new;

    new = mainArea();

    if (new >= 0)
    {
        if (CurArea.status)
        {
            highest();
            AreaSetLast(&CurArea);
            MsgAreaClose();
        }
        set_area(new);
        SetupArea();
    }

    return new;
}

/* start - begins reading the initial configuration file */

static int start(char *cfg, char *afn)
{
    opening(cfg, afn);
    SW->area = 0;
    message = NULL;
    return 0;
}

/* go_last - goes to the highest-read message in the msgbase */

static void go_last(void)
{
    CurArea.current = min(CurArea.lastread, CurArea.messages);
    root = CurArea.current;
}

/* slast - goes to the very last msg in the msgbase */

static void slast(void)
{
    CurArea.current = CurArea.messages;
    highest();
}

/* astart - goes to the first read msg in the msgbase (for this session) */

static void astart(void)
{
    CurArea.current = min(areastart, CurArea.messages);
    highest();
}

/* link_from - goes to the message that the current message is a reply to */

static void link_from(void)
{
    if (!message || !message->replyto)
    {
        return;
    }

    CurArea.current = message->replyto;
}

/* view - toggles the showing of hidden information */

static void view(void)
{
    SW->shownotes = !SW->shownotes;
    message = KillMsg(message);
}

/*
 *  Links up the msgbase, putting up a small menu if there is more
 *  than one thread to go to.  Quite complicated.
 */

static void link_to(void)
{
    msg *m;
    unsigned long cnt, k = 0;
    char txtbuf[40];
    char *replies[11];

    if (!message)
    {
        return;
    }

    for (cnt = 0; cnt < 10; cnt++)
    {
        if (message->replies[(size_t) cnt] != 0)
        {
            k++;
        }
    }
    if (k < 1)
    {
        return;
    }
    else
    {
        if (k == 1)
        {
            for (cnt = 0; cnt <= 10; cnt++)
            {
                if (message->replies[(size_t) cnt] != 0)
                {
                    break;
                }
            }
            CurArea.current = message->replies[(size_t) cnt];
            CurArea.lastread = max(CurArea.lastread, CurArea.current);
            return;
        }
    }

    k = 0;
    for (cnt = 0; cnt < 10; cnt++)
    {
        if (message->replies[(size_t) cnt] != 0)
        {
            if ((m = MsgReadHeader(message->replies[(size_t) cnt],
                                   RD_HEADER)) != NULL)
            {
                sprintf(txtbuf, "%5ld : %.22s", message->replies[(size_t) cnt],
                  m->isfrom);

                replies[(size_t) k++] = xstrdup(txtbuf);

                KillMsg(m);
            }
        }
    }

    replies[(size_t) k] = NULL;

    cnt = DoMenu(maxx - 43, 9, maxx - 6, 9 + (int)k - 1, replies, 0, SELBOX_LINKTO, "");

    if (cnt != (unsigned long)-1)
    {
        CurArea.current = atoi(replies[(size_t) cnt]);
        CurArea.lastread = max(CurArea.lastread, CurArea.current);
    }

    for (k = 0; k < 10; k++)
    {
        if (replies[(size_t) k] == NULL)
        {
            break;
        }
        xfree(replies[(size_t) k]);
    }
}

/*
 *  go_next; Does the same as above but ignores all but the first reply
 *  chain.
 */

static void go_next(void)
{
    if (!message || !message->replies[0])
    {
        return;
    }

    back = CurArea.current;
    CurArea.current = message->replies[0];
    CurArea.lastread = max(CurArea.lastread, CurArea.current);
}

/*
 *  Goes to the first msg in a reply chain (you must have navigated
 *  through it already).
 */

static void go_root()
{
    back = CurArea.current;
    CurArea.current = root;
    highest();
}

/* go_back - goes back to where you were in the reply chain (untested) */

static void go_back()
{
    CurArea.current = back;
    highest();
}

/*  rotate - Sets the ROT13 char */

static void rotate()
{
    rot13 = (rot13 + 1) % 3;
    message = KillMsg(message);
}

void shell_to_dos(void)
{
#if defined(MSDOS) && !defined(NODOSSWAP) && !defined(__FLAT__)
    int rc;
    char swapfn[PATHLEN];
    char **envp = environ, **env, *envbuf, *envptr, *ep;
    int swapping = USE_ALL | HIDE_FILE | DONT_SWAP_ENV;
    int envlen = 0;

    if (envp != NULL)
    {
        for (env = envp; *env != NULL; env++)
        {
            envlen += strlen(*env) + 1;
        }
    }

    if (envlen)
    {
        envlen = (envlen + 32) & 0xfff0;
        envbuf = xmalloc(envlen);
        envptr = envbuf;

        if (FP_OFF(envptr) & 0x0f)
        {
            envptr += 16 - (FP_OFF(envptr) & 0x0f);
        }

        ep = envptr;

        for (env = envp; *env != NULL; env++)
        {
            strcpy(ep, *env);
            ep = ep + strlen(*env) + 1;
        }
        *ep = 0;
    }
    if (ST->swap_path)
    {
        sprintf(swapfn, "%s\\%s", ST->swap_path, SWAP_FILENAME);
    }
    else
    {
        strcpy(swapfn, SWAP_FILENAME);
    }

    rc = prep_swap(swapping, swapfn);

    if (rc > -1)
    {
        rc = do_spawn(swapping, ST->comspec, "", envlen, envptr);
    }
    else
    {
        fprintf(stderr, "\nError occured during do_spawn(); rc=%d. Press Enter to return...", rc);
        while (GetKey() != 13)
        {
        }
    }
#elif defined(PACIFIC)
    spawnl(ST->comspec, NULL);
#elif defined(__FLAT__) || defined(OS2)
    spawnl(0, ST->comspec, ST->comspec, NULL);
#elif defined(UNIX)
    system(ST->comspec);
#else
    system("");
#endif
}

static void go_dos(void)
{
    char tmp[PATHLEN];

    mygetcwd(tmp, PATHLEN);
    setcwd(ST->home);
    WndClose(hMnScr);
    KillHotSpots();
    TTgotoxy(term.NRow - 1, 0);
    TTclose();
    cursor(1);
    fputs("\nEnter the command \"EXIT\" to return to " PROG ".\n", stderr);
    shell_to_dos();
    cursor(0);
    InitScreen();
    BuildHotSpots();
    DrawHeader();
    ShowNewArea();
    message = KillMsg(message);
    cursor(0);
    setcwd(tmp);
}

static void rundos(void)
{
    WND *hCurr, *hWnd;
    char curdir[PATHLEN];
    char cmd[64], tmp[40];
    int ret;

    mygetcwd(curdir, PATHLEN);
    memset(cmd, 0, sizeof cmd);

#if defined(MSDOS)
    if (!GetString(" System Command ", "Enter DOS command to execute:", cmd, 64))
    {
        return;
    }
#elif defined(OS2)
    if (!GetString(" System Command ", "Enter OS/2 command to execute:", cmd, 64))
    {
        return;
    }
#else
    if (!GetString(" System Command ", "Enter system command to execute:", cmd, 64))
    {
        return;
    }
#endif

    if (cmd[0] == '\0')
    {
        strcpy(cmd, ST->comspec);
    }

    hCurr = WndTop();
    hWnd = WndOpen(0, 0, maxx - 1, maxy - 1, NBDR, 0, cm[CM_NTXT]);

    cursor(1);
    ret = system(cmd);
    cursor(0);

#if defined(MSDOS)
    sprintf(tmp, "DOS command returned %d", ret);
#elif defined(OS2)
    sprintf(tmp, "OS/2 command returned %d", ret);
#else
    sprintf(tmp, "System command returned %d", ret);
#endif
    ChoiceBox(" Info ", tmp, "  Ok  ", NULL, NULL);

    WndClose(hWnd);
    WndCurr(hCurr);
    setcwd(curdir);
    message = KillMsg(message);
}

static void nada(void)
{
    /* do nothing */
}

void dolist(void)
{
    if (SW->direct_list)
        endMain = 1;
    else
    {
        if (message != NULL)
        {
            do_list();
            ClearScreen();
            DrawHeader();
            ShowNewArea();
        }
    }
}

static void list(void)
{
    dolist();
}

int CKey(int ch)
{
    return ((int)ConvertKey(ch));
}

void show_usage(void)
{
    printf(
      "%-30s; %s\n"
      "-------------------------------------------------------------------------------\n"
      "\n"
      "Usage: MSGED [options]\n"
      "\n"
      "-a<areafile>            Use <areafile> instead of SQUISH.CFG.\n"
      "-c<configfile>          Use <configfile> instead of MSGED.CFG.\n"
      "-I                      Display debug information at startup, then exit.\n"
      "-?                      Display this help.\n"
      "-h                      Display this help.\n"
      "-hc <source> <target>   Compile help file.\n"
      "-hi <source>            Decompile compiled help file.\n"
      "-k                      Display keyboard scan codes.\n",
      PROG " " VERSION CLOSED "; Mail Reader",
      "Compiled on " __DATE__ " at " __TIME__
    );
}

int cmd_dbginfo = 0;
int cmd_usage = 0;
int cmd_helpcmp = 0;
int cmd_helpinfo = 0;
int cmd_keycode = 0;
static char cmd_cfgfnm[250];
static char cmd_areafnm[250];

opt_t opttable[] =
{
    {"?", OPTBOOL, &cmd_usage},
    {"i", OPTBOOL, &cmd_dbginfo},
    {"I", OPTBOOL, &cmd_dbginfo},
    {"hc", OPTBOOL, &cmd_helpcmp},
    {"HC", OPTBOOL, &cmd_helpcmp},
    {"hi", OPTBOOL, &cmd_helpinfo},
    {"HI", OPTBOOL, &cmd_helpinfo},
    {"h", OPTBOOL, &cmd_usage},
    {"H", OPTBOOL, &cmd_usage},
    {"k", OPTBOOL, &cmd_keycode},
    {"K", OPTBOOL, &cmd_keycode},
    {"c", OPTSTR, cmd_cfgfnm},
    {"C", OPTSTR, cmd_cfgfnm},
    {"a", OPTSTR, cmd_areafnm},
    {"A", OPTSTR, cmd_areafnm},
    {NULL, 0, NULL}
};

static void message_reading_mode(void)
{
    EVT event;
    int newmsg;

    if (window_resized)
    {
        window_resized = 0;  /* ack! */
        WndClose(hMnScr);
        KillHotSpots();
        TTclose();
        InitScreen();
        adapt_margins();
        BuildHotSpots();
    }
    
    DrawHeader();
    
    endMain = 0;
    message = KillMsg(message);
    message = readmsg(CurArea.current);
    
    if (!CurArea.status || message == NULL || !CurArea.messages)
    {
        ClearMsgScreen();
        ShowMsgHeader(message);
        ChoiceBox(" Notice ", "There are no messages stored in this area", "  Ok  ", NULL, NULL);
    }
    
    newmsg = 1;
    
    while (!endMain)
    {
        
        if (!CurArea.messages || newmsg || !CurArea.status || message == NULL)
        {
            TTBeginOutput();

            if (!CurArea.status || message == NULL || !CurArea.messages)
            {
                ClearMsgScreen();
            }
            
            ShowMsgHeader(message);
            
            newmsg = 0;
            
            if (message != NULL)
            {
                RefreshMsg(message->text, 6);
            }

            TTEndOutput();
        }
        
        oldmsg = CurArea.current;
        command = MnuGetMsg(&event, hMnScr->wid);
        switch (event.msgtype)
        {
        case WND_WM_RESIZE:
            window_resized = 1;  /* we'll exit to redraw the
                                    screen */
            break;
            
        case WND_WM_COMMAND:
            switch (command)
            {
            case LMOU_CLCK:
                switch (event.id)
                {
                case ID_LNDN:
                    link_from();
                    break;
                    
                case ID_LNUP:
                    link_to();
                    break;
                    
                default:
                    break;
                }
                break;
                
            case MOU_LBTDN:
            case LMOU_RPT:
                switch (event.id)
                {
                case ID_SCRUP:
                    Go_Up();
                    break;
                    
                case ID_SCRDN:
                    Go_Dwn();
                    break;
                    
                case ID_MGLFT:
                    left();
                    break;
                    
                case ID_MGRGT:
                    right();
                    break;
                    
                default:
                    break;
                }
                break;
                
            default:
                break;
            }
            break;
            
        case WND_WM_MOUSE:
            switch (command)
            {
            case MOU_LBTDN:
                if (event.x >= (maxx - MNU_LEN - 1) && event.y == 0)
                {
                    command = ProcessMenu(&MouseMnu, &event, 0);
                }
                
                if (command == ID_QUIT)
                {
                    quit();
                }
                break;
                
            default:
                break;
            }
            break;
            
        case WND_WM_CHAR:
            switch (command)
            {
            case Key_PgUp:
                Go_PgUp();
                break;
                
            case Key_PgDn:
            case Key_Spc:
                Go_PgDwn();
                break;
                
            case Key_Up:
                Go_Up();
                break;
                
            case Key_Dwn:
                Go_Dwn();
                break;
                
            default:
                if (command & 0xff)
                {
                    if (isdigit(command & 0xff))
                    {
                        gotomsg((command & 0xff)- 0x30);
                    }
                    else
                    {
                        if (mainckeys[command & 0xff])
                        {
                            (*mainckeys[command & 0xff]) ();
                        }
                    }
                }
                else
                {
                    if (mainakeys[command >> 8])
                    {
                        (*mainakeys[command >> 8]) ();
                    }
                }
                break;
            }
            break;
        }
        
        if (window_resized && endMain == 0) endMain = 2;
        
        if (CurArea.messages > 0 &&
            (!message || oldmsg != CurArea.current || CurArea.current == 0))
        {
            message = KillMsg(message);
            if (CurArea.current == 0)
            {
                CurArea.current = 1;
            }
            if (CurArea.status)
            {
                message = readmsg(CurArea.current);
                newmsg = 1;
            }
        }
    }
}


int main(int argc, char *argv[])
{
    int optup;

    optup = getopts(argc, argv, opttable);
    if (cmd_keycode || cmd_helpcmp || cmd_helpinfo || cmd_usage)
    {
        if (cmd_usage)
        {
            show_usage();
        }
        else if (cmd_keycode)
        {
            keycode();
        }
        else if (cmd_helpcmp)
        {
            helpcmp(argc - optup + 1, &argv[optup - 1]);
        }
        else if (cmd_helpinfo)
        {
            helpinfo(argc - optup + 1, &argv[optup - 1]);
        }
        return (0);
    }

#ifdef USE_CRITICAL
    /* Removed by cleanup() */
    install24h();
#endif

#if !defined(PACIFIC) && !defined(__MINGW32__)
    tzset();
#endif

    randomize();

#ifdef USE_MSGAPI
    MsgApiInit();
#endif

    if (*cmd_cfgfnm && *cmd_areafnm)
    {
        command = start(cmd_cfgfnm, cmd_areafnm);
    }
    else if (*cmd_cfgfnm)
    {
        command = start(cmd_cfgfnm, NULL);
    }
    else if (*cmd_areafnm)
    {
        command = start(NULL, cmd_areafnm);
    }
    else
    {
        command = start(NULL, NULL);
    }

    if (ST->helpfile)
    {
        HelpInit(ST->helpfile);
    }

    BuildHotSpots();
    if (scan)
    {
        arealist_area_scan(1);
    }
    /* DrawHeader(); */
    RegisterKeyProc(CKey);      /* to allow for macros in the system */

    while (endMain == 2 || newarea() >= 0)
    {
        if (SW->direct_list)
        {
            while (do_list())
            {
                message_reading_mode();
            }
        }
        else
        {
            message_reading_mode();
        }

        if (CurArea.status && endMain != 2)
        {
            highest();
            AreaSetLast(&CurArea);
            MsgAreaClose();
        }
    }

    cleanup(NULL);
    return errorlevel;
}

