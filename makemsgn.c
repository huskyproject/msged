/*
 *  MAKEMSGN.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Routines to create new messages.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mctype.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef UNIX  /* UNIX only uses system(), and that is in stdlib.h */
#include <process.h>
#endif

#ifdef MSDOS
#include <dos.h>
#ifdef PACIFIC
#include <sys.h>
#else
#endif
#endif

#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "winsys.h"
#include "menu.h"
#include "dialogs.h"
#include "memextra.h"
#include "nshow.h"
#include "areas.h"
#include "readmail.h"
#include "keys.h"
#include "template.h"
#include "main.h"
#include "wrap.h"
#include "screen.h"
#include "userlist.h"
#include "vsevops.h"
#include "dirute.h"
#include "makemsgn.h"
#include "strextra.h"
#include "group.h"
#include "timezone.h"

#ifdef MSDOS
#ifdef USE_CRITICAL
#include "critical.h"
#endif
#if !defined(NODOSSWAP) && !defined(__FLAT__)
#include "spawn.h"
#endif
#endif

#define TEXTLEN 128
#define INPLEN  60

/* prototypes */

static void reply_msg(int type);
static void crosspost(msg * m);
static int externalEditor(msg * m);

extern int savecc;

static msg *EdMsg;              /* message header being edited */
static int scan_base;           /* force a scan of the msgbase */
int do_lookup = FALSE;          /* lookup thru the nodelist? */
static int editRet;

static int sent_msg(void)
{
    int rc;
    rc = ChoiceBox("", "WARNING: This message has already been sent! Continue?", "Yes", "No", NULL);
    return rc == ID_ONE ? 1 : 0;
}

/*
 *  Creates a duplicate of the passed message header, allocating
 *  memory for it in the process.
 */

#define copystr(d, s) { if (s) { d = xstrdup(s); } else { d = NULL; } }

msg *duplicatemsg(msg * from)
{
    msg *to;

    to = xcalloc(1, sizeof *to);

    *to = *from;                /* copy all the bits */

    copystr(to->isfrom, from->isfrom);
    copystr(to->isto, from->isto);
    copystr(to->subj, from->subj);
    copystr(to->to.domain, from->to.domain);
    copystr(to->from.domain, from->from.domain);
    copystr(to->msgid, from->msgid);
    copystr(to->reply, from->reply);
    copystr(to->replyarea, from->replyarea);
    copystr(to->charset_name, from->charset_name);

    to->text = NULL;

    return to;
}

void newmsg(void)
{
    reply_msg(MT_NEW);
}

void reply(void)
{
    reply_msg(MT_REP);
}

void quote(void)
{
    if (message && message->replyarea)
    {
        reply_msg(SW->rotharea);
    }
    else
    {
        reply_msg(SW->rquote);
    }
}

void reply_oarea(void)
{
    reply_msg(SW->rotharea);
}

void followup(void)
{
    reply_msg(SW->rfollow);
}

void replyextra(void)
{
    reply_msg(SW->rextra);
}

static int findArea(char *tag)
{
    int i = 0;
    int areano;

    while (i < SW->groupareas)
    {
        areano = group_getareano(i);
        if (areano >= 0 &&
            !stricmp(arealist[group_getareano(i)].tag, tag))
        {
            break;
        }
        i++;
    }

    if (i == SW->groupareas)
    {
        i = 0;
    }

    return i;
}

static void reply_msg(int type)
{
    FIDO_ADDRESS tmp;
    msg *hlink;
    msg *m;
    unsigned long t = CurArea.current;
    unsigned long tl = CurArea.lastread;
    unsigned long link = t;
    int oarea = SW->grouparea;
    int ogroup = SW->group;
    int q = 0;
    msg *oldmsg;                /* contains all the old information */
    unsigned long ulink;
    int toarea = 0, i;
    int count;                  /* for robot names */

    scan_base = 0;
    oldmsg = NULL;

    if (type & MT_NEW)
    {
        RefreshMsg(NULL, 6);
    }

    if ((!(type & MT_NEW) && CurArea.messages == 0) || !CurArea.status)
    {
        return;
    }

    group_set_group(0); /* allow crossposts etc. pp. into all areas, and not
                           only into areas of current group */

    if (type & MT_ARC)
    {
        toarea = SW->grouparea;
        if (message && message->replyarea)
        {
            toarea = findArea(message->replyarea);
        }
        toarea = selectarea("Answer In Area", toarea);
        if (msgederr)
        {
            /* This code at this place is COMPLETE bullshit. No message has
               ever been written, and no prior call to set_area has
               taken place! It should be removed, but first I want to study if
               there are any side effects of it's removal.

            if ((type & MT_ARC) || scan_base || CurArea.msgtype == SQUISH)
            {
                set_area(oarea);
            }
            else
            {
                CurArea.current = link;
                CurArea.messages++;
                CurArea.last++;
            }

            */

            group_set_group(ogroup);

            return;
        }
    }

    if (message)
    {
        oldmsg = duplicatemsg(message);
    }

    if (CurArea.messages == 0 || message == NULL)
    {
        message = xcalloc(1, sizeof *message);
    }

    ulink = message->msgnum;

    /*
     *  We work with m, not message - the set_area() function kills
     *  message automatically.
     */

    m = message;
    message = NULL;

    if (type & MT_REP)
    {
        m->text = clearbuffer(m->text);
    }

    if (type & MT_NEW)
    {
        clearmsg(m);
    }

    if (type & MT_FOL)
    {
        release(m->msgid);
        m->msgid = m->reply;
        m->reply = NULL;
    }

    release(m->reply);

    if (m->msgid)
    {
        m->reply = m->msgid;
        m->msgid = NULL;
    }
    if (type & MT_FOL)
    {
        release(m->isfrom);
        tmp = m->to;
    }
    else
    {
        if (!(type & MT_NEW))
        {
            release(m->isto);
            m->isto = m->isfrom;
        }
        else
        {
            m->to.zone = CurArea.addr.zone;
        }
        tmp = m->from;
    }

    m->isfrom = xstrdup(ST->username);

    if (type & MT_ARC)
    {
        set_area(toarea);
        if (!CurArea.status)
        {
            dispose(oldmsg);
            dispose(m);
            group_set_group(ogroup);
            set_area(oarea);
            return;
        }
    }

    /* TE: Only do lookup when composing a new message. Don't lookup
           when replying to an exisiting message, because we suppose that the
           sender wants the answer to the AKA that he is using. */
    do_lookup = (CurArea.netmail && (type & MT_NEW)) ? TRUE : FALSE;
    m->to = tmp;
    m->timestamp = time(NULL);
    if (SW->tzutc)
    {
        m->timezone = tz_my_offset();
        m->has_timezone = 1;
    }
    m->replyto = 0;
    m->new = 1;
    m->cost = 0;
    m->times_read = 0;
    m->scanned = 0;
    m->soteot = 0;
    m->time_arvd = 0;

    if (CurArea.echomail && CurArea.messages == 0)
    {
        m->msgnum = 2;
    }
    else
    {
        m->msgnum = MsgnToUid(CurArea.messages) + 1;
    }

    /* find a origin aka matching the destination zone for netmails */

    m->from = CurArea.addr;
    if (CurArea.addr.domain)
    {
        m->from.domain = xstrdup(CurArea.addr.domain);
    }

    if ((CurArea.netmail) && (!(type & MT_NEW)))
    {
        akamatch(&(m->from), &(m->to));
    }

    if (m->to.internet || m->to.bangpath)
    {
       char *oldptr =  m->to.domain;
       m->to.domain = compose_internet_address(m->to.domain, m->isto);
       release(oldptr);
       release(m->isto);
    }

    if (!(type & MT_NEW) && !(type & MT_ARC))
    {
        m->replyto = link;
    }
    else
    {
        m->replyto = 0;
    }

    clear_attributes(&m->attrib);
    memset(m->replies, 0, sizeof(m->replies));

    while (!q)
    {
        if (EditHeader(m) == Key_Esc)
        {
            if (confirm("Cancel?"))
            {
                dispose(oldmsg);

                group_set_group(ogroup);
                dispose(m);

                if (type & MT_ARC)
                    set_area(oarea);

                if (CurArea.status)
                {
                    CurArea.current = (t) ? t : CurArea.current;
                    CurArea.lastread = (tl) ? tl : CurArea.lastread;
                }
                return;
            }
        }
        else
        {
            q = 1;
        }
    }

    /*
     *  Create the template for the message.  Will not include template
     *  if being sent to robotnames.  Modified by Kieran Haughey, Andrew
     *  Clarke.
     */

    if (user_list[0].robotname == NULL)
    {
        user_list[0].robotname = "AreaFix";  /* default */
    }
    count = 0;
    for (i = 0; i < MAXUSERS; i++)
    {
        if (user_list[i].robotname == NULL || m->isto == NULL)
        {
            break;
        }
        if (stricmp(user_list[i].robotname, m->isto) == 0)
        {
            count++;
        }
    }

    if (!(type & MT_NEW))
    {
        RefreshMsg(NULL, 6);
    }

    if (count == 0)
    {
        MakeTemplateMsg(m, oldmsg, group_getareano(oarea), type | MT_NEW);
    }

    if (ST->editorName != NULL)
    {
        editRet = externalEditor(m);
    }
    else
    {
        editRet = editmsg(m, ((type & MT_NEW) || (type & MT_REP)) ? 0 : 1);
    }

    switch (editRet)
    {
    case SAVE:
        save(m);
        if (!(type & MT_ARC) && !(type & MT_NEW))
        {
            link = UidToMsgn(ulink);
            if (CurArea.msgtype != QUICK)
            {
                /* somewhere is leaking so i turned it off for QBBS */

                if ((hlink = MsgReadHeader(link, RD_HEADER)) != NULL)
                {
                    if (CurArea.msgtype == FIDO)
                    {
                        hlink->replies[0] = UidToMsgn(m->msgnum);
                    }
                    MsgWriteHeader(hlink, WR_HEADER);
                    dispose(hlink);
                }
            }
        }
        break;

    case ABORT:
        ChoiceBox("", "Message was aborted.", "  Ok  ", NULL, NULL);
        scan_base = 1;
        break;
    }

    dispose(oldmsg);
    dispose(m);

    group_set_group(ogroup);

    if ((type & MT_ARC) || scan_base || CurArea.msgtype == SQUISH)
    {
        set_area(oarea);
        CurArea.current = link;
    }
    else
    {
        CurArea.current = link;
        CurArea.messages++;
        CurArea.last++;
    }
}

void change(void)
{
    int q = 0;
    unsigned long t = CurArea.current;
    int oarea = SW->grouparea;
    int ogroup = SW->group;

    if (CurArea.messages == 0 || !CurArea.status || !message)
        return;

    if (message->attrib.sent || message->scanned)
    {
        if (!sent_msg())
        {
            return;
        }
    }

    message->attrib.sent = 0;
    message->attrib.orphan = 0;
    message->attrib.local = 1;
    message->scanned = 0;
    do_lookup = FALSE;

    if (message->to.internet || message->to.bangpath)
    {
       char *oldptr = message->to.domain;
       message->to.domain = compose_internet_address(message->to.domain,
                              message->isto);
       release(oldptr);
       release(message->isto);
    }

    while (!q)
    {
        if (EditHeader(message) == Key_Esc)
        {
            if (confirm("Cancel?"))
            {
                if (t)
                {
                    CurArea.current = t;
                }
                return;
            }
        }
        else
        {
            q = 1;
        }
    }

    if (ST->editorName != NULL)
    {
        editRet = externalEditor(message);
    }
    else
    {
        editRet = editmsg(message, FALSE);
    }

    switch (editRet)
    {
    case SAVE:
        save(message);
        break;

    case ABORT:
        ChoiceBox("", "Message was aborted.", "  Ok  ", NULL, NULL);
        break;
    }

    group_set_group(ogroup);

    if (scan_base)  /* While changing, CC:s and XC:s could occur. */
    {
        set_area(oarea);
    }

    if (t)
    {
        CurArea.current = t;
    }
    message = KillMsg(message);
}

int ChangeAttrib(msg * m)
{
    WND *hWnd, *hCurr;
    int ch, done = 0;

    hCurr = WndTop();
    hWnd = WndPopUp(62, 13, SBDR | SHADOW, cm[IN_BTXT], cm[IN_NTXT]);

    /* put up some help for the befuddled user */

    WndTitle(" Message Attributes (Alt-Z: erase all)", cm[IN_NTXT]);

    WndWriteStr(2, 0, cm[IN_NTXT], "Private             <Alt-P>   Crash               <Alt-C>");
    WndWriteStr(2, 1, cm[IN_NTXT], "File Attach         <Alt-A>   Kill/Sent           <Alt-K>");
    WndWriteStr(2, 2, cm[IN_NTXT], "Hold                <Alt-H>   Direct              <Alt-D>");
    WndWriteStr(2, 3, cm[IN_NTXT], "Sent                <Alt-S>   Received            <Alt-R>");
    WndWriteStr(2, 4, cm[IN_NTXT], "Orphan              <Alt-O>   File Request        <Alt-F>");
    WndWriteStr(2, 5, cm[IN_NTXT], "Return Rcpt         <Alt-E>   Return Rcpt Request <Alt-Q>");
    WndWriteStr(2, 6, cm[IN_NTXT], "Audit Request       <Alt-I>   File Update Request <Alt-U>");
    WndWriteStr(2, 7, cm[IN_NTXT], "Local               <Alt-L>   Intransit (Forward) <Alt-T>");
    WndWriteStr(2, 8, cm[IN_NTXT], "Kill File when Sent <Alt-X>   Truncate File w. S. <Alt-Y>");
    WndWriteStr(2, 9, cm[IN_NTXT], "Archive when Sent   <Alt-J>   Immediate Delivery  <Alt-V>");
    WndWriteStr(2,10, cm[IN_NTXT], "Lock                <Alt-W>   Confirm Rcpt Req    <Alt-M>");
    WndWriteStr(2,11, cm[IN_NTXT], "Route via Zone Gate <Alt-G>   Route via Hub       <Alt-B>");
/*  WndWriteStr(2,12, cm[IN_NTXT], "           Zap (erase) all attributes  <Alt-Z>           "); */

    WndCurr(hCurr);  /* make parent window active so we can write to it */

    ShowAttrib(m);

    while (!done)
    {
        ch = GetKey();
        switch (ch)
        {
        case Key_Up:
        case Key_Dwn:
        case Key_Ent:
        case Key_Esc:
            done = 1;           /* we return these to the caller */
            break;

        case Key_A_A:
            m->attrib.attach ^= 1;
            break;

        case Key_A_P:
            m->attrib.priv ^= 1;
            break;

        case Key_A_C:
            m->attrib.crash ^= 1;
            break;

        case Key_A_U:
            m->attrib.ureq ^= 1;
            break;

        case Key_A_F:
            m->attrib.freq ^= 1;
            break;

        case Key_A_K:
            m->attrib.killsent ^= 1;
            break;

        case Key_A_H:
            m->attrib.hold ^= 1;
            break;

        case Key_A_D:
            m->attrib.direct ^= 1;
            break;

        case Key_A_Q:
            m->attrib.rreq ^= 1;
            break;

        case Key_A_E:
            m->attrib.rcpt ^= 1;
            break;

        case Key_A_R:
            m->attrib.rcvd ^= 1;
            break;

        case Key_A_S:
            m->attrib.sent ^= 1;
            break;

        case Key_A_O:
            m->attrib.orphan ^= 1;
            break;

        case Key_A_I:
            m->attrib.areq ^= 1;
            break;

        case Key_A_L:
            m->attrib.local ^= 1;
            break;

        case Key_A_T:
            m->attrib.forward ^= 1;
            break;

        case Key_A_X:
            m->attrib.kfs ^= 1;
            break;

        case Key_A_Y:
            m->attrib.tfs ^= 1;
            break;

        case Key_A_J:
            m->attrib.as ^= 1;
            break;

        case Key_A_V:
            m->attrib.immediate ^= 1;
            break;

        case Key_A_W:
            m->attrib.lock ^= 1;
            break;

        case Key_A_M:
            m->attrib.cfm ^= 1;
            break;

        case Key_A_G:
            m->attrib.zon ^= 1;
            break;

        case Key_A_B:
            m->attrib.hub ^= 1;
            break;


        case Key_A_Z:
            m->attrib.attach = 0;
            m->attrib.priv = 0;
            m->attrib.crash = 0;
            m->attrib.ureq = 0;
            m->attrib.freq = 0;
            m->attrib.killsent = 0;
            m->attrib.hold = 0;
            m->attrib.direct = 0;
            m->attrib.rreq = 0;
            m->attrib.rcpt = 0;
            m->attrib.rcvd = 0;
            m->attrib.sent = 0;
            m->attrib.orphan = 0;
            m->attrib.areq = 0;
            m->attrib.local = 0;
            m->attrib.forward = 0;
            m->attrib.kfs = 0;
            m->attrib.as = 0;
            m->attrib.immediate = 0;
            m->attrib.tfs = 0;
            m->attrib.lock = 0;
            m->attrib.cfm = 0;
            m->attrib.zon = 0;
            m->attrib.hub = 0;
            m->scanned = 0;
            break;

        default:
            switch ((toupper(ch & 0xff)))
            {
            case 'A':
                m->attrib.attach ^= 1;
                break;

            case 'P':
                m->attrib.priv ^= 1;
                break;

            case 'C':
                m->attrib.crash ^= 1;
                break;

            case 'U':
                m->attrib.ureq ^= 1;
                break;

            case 'F':
                m->attrib.freq ^= 1;
                break;

            case 'K':
                m->attrib.killsent ^= 1;
                break;

            case 'H':
                m->attrib.hold ^= 1;
                break;

            case 'D':
                m->attrib.direct ^= 1;
                break;

            case 'Q':
                m->attrib.rreq ^= 1;
                break;

            case 'E':
                m->attrib.rcpt ^= 1;
                break;

            case 'R':
                m->attrib.rcvd ^= 1;
                break;

            case 'S':
                m->attrib.sent ^= 1;
                break;

            case 'O':
                m->attrib.orphan ^= 1;
                break;

            case 'I':
                m->attrib.areq ^= 1;
                break;

            case 'L':
                m->attrib.local ^= 1;
                break;

            case 'T':
                m->attrib.forward ^= 1;
                break;

            case 'X':
                m->attrib.kfs ^= 1;
                break;

            case 'Y':
                m->attrib.tfs ^= 1;
                break;

            case 'J':
                m->attrib.as ^= 1;
                break;

            case 'V':
                m->attrib.immediate ^= 1;
                break;

            case 'W':
                m->attrib.lock ^= 1;
                break;

            case 'M':
                m->attrib.cfm ^= 1;
                break;

            case 'G':
                m->attrib.zon ^= 1;
                break;

            case 'B':
                m->attrib.hub ^= 1;
                break;

            case 'Z':
                m->attrib.attach = 0;
                m->attrib.priv = 0;
                m->attrib.crash = 0;
                m->attrib.ureq = 0;
                m->attrib.freq = 0;
                m->attrib.killsent = 0;
                m->attrib.hold = 0;
                m->attrib.direct = 0;
                m->attrib.rreq = 0;
                m->attrib.rcpt = 0;
                m->attrib.rcvd = 0;
                m->attrib.sent = 0;
                m->attrib.orphan = 0;
                m->attrib.areq = 0;
                m->attrib.local = 0;
                m->attrib.forward = 0;
                m->attrib.kfs = 0;
                m->attrib.as = 0;
                m->attrib.immediate = 0;
                m->attrib.tfs = 0;
                m->attrib.lock = 0;
                m->attrib.cfm = 0;
                m->attrib.zon = 0;
                m->attrib.hub = 0;
                m->scanned = 0;
                break;

            default:
                break;
            }
        }
        ShowAttrib(m);
    }

    WndCurr(hWnd);
    WndClose(hWnd);
    WndCurr(hCurr);
    return ch;  /* pass the exit key back to caller */
}

/*
 *  Looks up the alias list and returns the new name, and changes the
 *  passed address to the alias addres, if the passed name had a
 *  corresponding alias.
 */

static char *alias_lookup(FIDO_ADDRESS * addr, char *isto)
{
    int l;
    char *name;

    /* search for a matching alias */

    for (l = 0; l < SW->otheraliases; l++)
    {
        if (!stricmp(aliaslist[l].alias, isto))
        {
            break;
        }
    }

    if (l >= SW->otheraliases)
    {
        return NULL;
    }
    else
    {
        name = xstrdup(aliaslist[l].name);
        addr->domain = NULL;
        *addr = aliaslist[l].addr;
        if (aliaslist[l].addr.domain != NULL)
        {
            addr->domain = xstrdup(aliaslist[l].addr.domain);
        }

/*      if (aliaslist[l].addr.internet == 1)
        {
            *addr = uucp_gate;
            addr->domain = NULL;
        }
        else
        {
            *addr = aliaslist[l].addr;
            if (aliaslist[l].addr.domain)
            {
                addr->domain = xstrdup(aliaslist[l].addr.domain);
            }
            else
            {
                addr->domain = NULL;
            }
        } */
    }
    return name;
}

/*
 *  Looks up the alias list to see if the passed alias has a subject.
 */

char *subj_lookup(char *isto)
{
    int l;

    for (l = 0; l < SW->otheraliases; l++)
    {
        if (!stricmp(aliaslist[l].alias, isto))
        {
            break;
        }
    }

    if (l >= SW->otheraliases || aliaslist[l].subj == NULL)
    {
        return NULL;
    }

    return xstrdup(aliaslist[l].subj);
}

/*
 *  Looks up name and addresses using alias, nodelist and fido userlist
 *  methods.  Returns a pointer to allocated memory containing the name
 *  to be used, and writes to the passed address variable.
 *
 *  Modifications by: Roland Gautschi (V7 lookup code - added flexibility).
 */

static char *addr_lookup(char *name, FIDO_ADDRESS * tmp)
{
    char *nname;
    char xname[73];
    char namefound[73];
    char userlistname[73];
    int ret;

    tmp->fidonet = 1;
    tmp->internet = 0;
    tmp->bangpath = 0;

    if ((nname = alias_lookup(tmp, name)) == NULL)
    {
        nname = xstrdup(name);  /* couldn't find an alias, so.. */
        strcpy(xname, name);    /* make a copy, just in case */

        if (do_lookup)
        {
            /*
             *  Userlists are checked first, because they may have overides
             *  wanted by the user.
             */

            if (ST->fidolist != NULL)
            {
                strcpy (userlistname, name);
                *tmp = lookup(userlistname, ST->fidolist);
                if ((tmp->notfound) && (ST->userlist != NULL))
                {
                    *tmp = lookup(userlistname, ST->userlist);
                }
                if (!tmp->notfound)
                {
                   release(nname);
                   nname = xstrdup(userlistname);
                }
            }
            else
            {
                tmp->notfound = 1;
            }

            if (tmp->notfound == 1 && ST->nodepath != NULL)
            {
                /* Check to see if an address has been entered. */

                if (m_isdigit(xname[0]) || xname[0] == ':' ||
                  xname[0] == '/' || xname[0] == '.')
                {
                    *tmp = parsenode(xname);
                    if (tmp->notfound == 0)
                    {
                        if (v7lookupnode(tmp, xname) == NULL)
                        {
                            tmp->notfound = 1;
                        }
                        else
                        {
                            xfree(nname);
                            nname = xstrdup(xname);
                        }
                    }
                }

                /*
                 *  If the node still hasn't been found, then lookup
                 *  normally (using the entered name).
                 */

                if (tmp->notfound == 1)
                {
                    *tmp = v7lookup(xname);

                    /* If we found something, copy it. */

                    if (tmp->notfound == 0)
                    {
                        if (strcmp(nname, xname) != 0)
                        {
                            sprintf(namefound, "%s at %s found, change?",
                              xname, show_address(tmp));
                            ret = ChoiceBox(" Change Name ", namefound,
                              "Yes", "No", NULL);
                            if (ret == ID_ONE)
                            {
                                xfree(nname);
                                nname = xstrdup(xname);
                            }
                        }
                    }
                }
            }
        }
    }

    return nname;
}

static void GetAddress(FIDO_ADDRESS * addr, char *from, char *subj)
{
    char *name, *str;

    if (do_lookup && (!CurArea.netmail))
    {
        do_lookup = FALSE;
    }

    name = addr_lookup(from, addr);

    /* Check for an Alias subject. */

    if ((str = subj_lookup(from)) != NULL)
    {
        strcpy(subj, str);
        release(str);
    }

    /* Check the name to see if it's a usenet message. */

    str = strchr(name, '@');
    if (str != NULL && (CurArea.netmail && CurArea.uucp))
    {
        addr->fidonet = 0;
        addr->notfound = 0;
        if (str == name)
        {
            addr->bangpath = 1;
            str++;
        }
        else
        {
            addr->internet = 1;
            str = name;
        }
        release(addr->domain);
        addr->domain = xstrdup(str);
        strcpy(from, str);
    }
    else if (str!=NULL && (CurArea.news || CurArea.uucp))
    {
        parse_internet_address(name, NULL, &str);
        strcpy(from, str);
    }
    else
    {
        strcpy(from, name);
    }
    release(name);
}

static int ChangeName(FIDO_ADDRESS * addr, char *from, char *subj, int y)
{
    EVT e;
    char tmp[73], tmp2[73];
    static int disp;
    int ch = 0, pos, done = 0;

    strcpy(tmp2, subj);
    tmp[0] = '\0';
    if (addr->internet && addr->domain != NULL)
    {
        strncpy(tmp, addr->domain, sizeof tmp - 1);
    }
    else
    {
        if (addr->bangpath && addr->domain != NULL)
        {
            strncat(strcpy(tmp, "@"), addr->domain, sizeof tmp - 1);
        }
        else
        {
            if (from != NULL)
            {
                strncpy(tmp, from, sizeof tmp - 1);
            }
        }
    }

    pos = strlen(tmp);
    disp = 1;

    while (!done)
    {
        if (addr->bangpath || addr->internet || CurArea.news || CurArea.uucp)
        {
            ch = WndGetLine(8, y, maxx - 28, tmp, cm[CM_ETXT], &pos, disp, 0, disp, &e);
        }
        else
        {
            ch = WndGetLine(8, y, 36, tmp, cm[CM_ETXT], &pos, disp, 0, disp, &e);
        }

        switch (e.msgtype)
        {
        case WND_WM_CHAR:
            switch (ch)
            {
            case Key_Esc:
                 done = 1;
                 break;

            case Key_Up:
            case Key_Dwn:
            case Key_Rgt:
            case Key_Lft:
            case Key_Ent:
                if (addr->fidonet)
                {
                    if (strlen(tmp) > 36)
                    {
                        tmp[35] = '\0';
                    }
                }
                GetAddress(addr, tmp, tmp2);
                strcpy(from, tmp);
                if (strcmp(subj, tmp2))
                {
                    strcpy(subj, tmp2);
                    ShowSubject(subj);
                }
                done = 1;
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

    return ch;
}

int ChangeAddress(FIDO_ADDRESS * addr, int y, int nm_len)
{
    EVT e;
    char tmp[41];
    int ch, pos, done = 0, disp = 1;

    strncpy(tmp, show_address(addr), 40);
    release(addr->domain);

    pos = strlen(tmp);

    while (!done)
    {
        ch = WndGetLine(8 + nm_len + 2, y, 50 - nm_len, tmp, cm[CM_ETXT], &pos, disp, 0, disp, &e);

        switch (e.msgtype)
        {
        case WND_WM_CHAR:
            switch (ch)
            {
            case Key_Esc:
                done = 1;
                break;

            case Key_Up:
            case Key_Dwn:
            case Key_Rgt:
            case Key_Lft:
            case Key_Ent:
                *addr = parsenode(tmp);
                done = 1;
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

    return ch;
}

int ChangeSubject(char *subj)
{
    EVT e;
    struct _dta fileinfo;
    char tmp[73];
    int ch, pos, ch2, done = 0, disp = 1;

    strncpy(tmp, subj, sizeof tmp - 1);

    pos = strlen(tmp);

    while (!done)
    {
        ch = WndGetLine(8, 3, 72, tmp, cm[CM_ETXT], &pos, disp, 0, disp, &e);

        switch (e.msgtype)
        {
        case WND_WM_CHAR:
            switch (ch)
            {
            case Key_Esc:
                done = 1;
                break;

            case Key_Up:
            case Key_Dwn:
            case Key_Rgt:
            case Key_Lft:
            case Key_Ent:
                strcpy(subj, tmp);
                if (strlen(subj) > 3 && *(subj + 1) == ':' &&
                  (*(subj + 2) == '\\' || *(subj + 2) == '/'))
                {
                    /*
                     *  If the file doesn't exist, then ask if user wants
                     *  to continue...
                     */

                    if (dir_findfirst(subj, DIR_NORMAL, &fileinfo) != 0)
                    {
                        ch2 = ChoiceBox("", "WARNING: File doesn't exist! Attach anyway?",
                          "Yes", "No", NULL);

                        if (ch2 == ID_ONE)
                        {
                            EdMsg->attrib.attach = 1;
                        }
                        else
                        {
                            /* continue editing */
                            continue;
                        }
                    }
                    else
                    {
                        EdMsg->attrib.attach = 1;
                    }
                }
                done = 1;
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
    ShowSubject(subj);
    return ch;
}

static char *getstring(char *buf)
{
    if (strlen(buf) > 0)
    {
        return xstrdup(buf);
    }
    else
    {
        return NULL;
    }
}

char *construct_uucpname(const char *cpdomain)
{
  if (strcmp(ST->uucpgate, "*") != 0)
  {
     return xstrdup(ST->uucpgate);
  }
  else
  {
     char *cpname;

     parse_internet_address(cpdomain, NULL, &cpname);
     return cpname;
  }
}

#define FD_FROM   0
#define FD_FADD   1
#define FD_TO     2
#define FD_TADD   3
#define FD_SUBJ   4
#define FD_ATTR   5

int EditHeader(msg * m)
{
    char tmp[80], tmp2[80];
    int field = 2;
    int ch = 0, done = 0;

    /*
     *  This is naughty, but it allows the functions to access the
     *  current message in those special cases.
     */

    EdMsg = m;

    ShowMsgHeader(m);

    while (!done)
    {
        strcpy(tmp, "");
        strcpy(tmp2, "");

        switch (field)
        {
        case FD_FROM:
            ShowNameAddress(m->isfrom, &m->from, 1, 0, 1);
            if (m->isfrom)
            {
                strcpy(tmp, m->isfrom);
                release(m->isfrom);
            }
            if (m->subj)
            {
                strcpy(tmp2, m->subj);
                release(m->subj);
            }
            ch = ChangeName(&m->from, tmp, tmp2, 1);
            m->isfrom = getstring(tmp);
            m->subj = getstring(tmp2);

            if (m->from.internet || m->from.bangpath)
            {
                char *oldname=m->isfrom;

                m->isfrom = construct_uucpname(m->isfrom);
                release (oldname);

            }
            ShowNameAddress(m->isfrom, &m->from, 1, 0, 1);
            break;

        case FD_FADD:
            if (CurArea.netmail && m->from.fidonet)
            {
                ch = ChangeAddress(&m->from, 1,
                                   (m->isfrom) ? strlen(m->isfrom) : 0);
                m->from.dontmatch = 1; /* no more aka matching here, please */
            }
            ShowNameAddress(m->isfrom, &m->from, 1, 0, 0);
            break;

        case FD_TO:
            if (CurArea.news)
            {
                release(m->isto);
                m->isto = xstrdup("All");
                m->to = uucp_gate;
                ch = Key_Dwn;
            }
            else
            {
                ShowNameAddress(m->isto, &m->to, 2, 0, 1);
                if (m->isto)
                {
                    strcpy(tmp, m->isto);
                    release(m->isto);
                }
                if (m->subj)
                {
                    strcpy(tmp2, m->subj);
                    release(m->subj);
                }
                ch = ChangeName(&m->to, tmp, tmp2, 2);
                m->isto = getstring(tmp);
                m->subj = getstring(tmp2);
            }

            /* generate the UUCP user name, if e-mail address was entered */
            if (m->to.internet || m->to.bangpath)
            {
                char *oldname = m->isto;

                m->isto = construct_uucpname(m->isto);
                release(oldname);
            }

            ShowNameAddress(m->isto, &m->to, 2, 0, 1);
            break;

        case FD_TADD:
            if (CurArea.netmail && m->to.fidonet)
            {
                ch = ChangeAddress(&m->to, 2, (m->isto) ? strlen(m->isto) : 0);
            }

            /* AKA matching, only for netmails */
            if (CurArea.netmail)
            {
                if (akamatch(&(m->from), &(m->to)))
                {
                    /* we found a better address, redisplay it */
                    ShowNameAddress(m->isfrom, &m->from, 1, 0, 0);
                }
            }
            break;

        case FD_SUBJ:
            if (m->subj)
            {
                strcpy(tmp, m->subj);
                release(m->subj);
            }
            ch = ChangeSubject(tmp);
            m->subj = getstring(tmp);
            break;

        case FD_ATTR:
            ch = ChangeAttrib(m);
            break;

        default:
            break;
        }

        if (ch == Key_Esc)
        {
            done = 1;
        }
        else if (ch == Key_Up)
        {
            field--;

            if (field < 0)
            {
                field = 5;
            }

            /* Handle the cases where going up stuffs the address showing. */

            switch (field)
            {
            case FD_FADD:
                ShowNameAddress(m->isto, &m->to, 2, 0, 0);
                break;

            case FD_ATTR:
                ShowNameAddress(m->isfrom, &m->from, 1, 0, 0);
                break;

            default:
                break;
            }
        }
        else if (ch == Key_Dwn || ch == Key_Ent)
        {
            if (field == 5 && ch == Key_Ent)
            {
                break;
            }

            field++;

            if (field > 5)
            {
                field = 0;
            }

            continue;
        }
    }

    ShowMsgHeader(m);
    cursor(0);

    return 0;
}

/*
 *  Clears all the attributes of a message, leaving Local set to TRUE.
 *  Not static; is called from maintmsg.c module.
 */

void clear_attributes(struct _attributes *h)
{
    memset(h, '\0', sizeof *h);
    h->crash = CurArea.crash;
    h->priv = CurArea.priv;
    h->killsent = CurArea.killsent;
    h->hold = CurArea.hold;
    h->direct = CurArea.direct;
    h->local = 1;
}

/* Contains the name and address of the recipient of the message */

typedef struct
{
    char *name;
    FIDO_ADDRESS addr;
}
NA;

static void CreateNormalCC(msg * m, NA * names[])
{
    LINE *current;
    char dfn[256];

    int i = 0;

    m->text = lineins(m->text);
    m->text->text = xstrdup("\n");
    m->text = lineins(m->text);

    sprintf(dfn, "* Original to:   %s\n", names[i]->name);

    i++;

    m->text->text = xstrdup(dfn);

    strcpy(dfn, "* Carbon Copies: ");

    current = m->text;
    current = lineins(current->next);
    current->text = xstrdup("\n");

    while (names[i] != NULL)
    {
        if (strlen(names[i]->name) + strlen(dfn) >= SW->rm)
        {
            strcat(dfn, "\n");
            release(current->text);

            current->text = xstrdup(dfn);
            current = lineins(current->next);
            current->text = xstrdup("\n");

            strcpy(dfn, "                 ");
        }
        strcat(dfn, names[i]->name);

        i++;

        if (names[i] != NULL)
        {
            strcat(dfn, ", ");
        }
    }

    if (*(current->text) == '\n')
    {
        release(current->text);
        current->text = xstrdup(dfn);
    }
}

static void CreateVerboseCC(msg * m, NA * names[])
{
    int i = 0;
    char dfn[256];

    m->text = lineins(m->text);
    m->text->text = xstrdup("\n");

    while (names[i] != NULL)
    {
        if (strchr(names[i]->name, '@') == NULL)
        {
            sprintf(dfn, "     %s at %s\n", names[i]->name, show_address(&names[i]->addr));
        }
        else
        {
            sprintf(dfn, "     %s\n", names[i]->name);
        }

        m->text = lineins(m->text);
        m->text->text = xstrdup(dfn);

        i++;
    }

    m->text = lineins(m->text);
    m->text->text = xstrdup("* Carbon Copies to:\n");
}

/*
 *  Gets the CC:s from a response file.
 */

static void GetFileCCs(char *file, NA * names[], int *idx)
{
    FILE *fp;
    char buf[256];
    char *s, *c, *t;
    int i;

    i = *idx;

    fp = fopen(file, "r");
    if (fp == NULL)
    {
        return;
    }

    while (fgets(buf, sizeof(buf) - 1, fp) != NULL)
    {
        s = buf;
        while (*s && m_isspace(*s))
        {
            s++;
        }

        if (!*s || *s == ';')
        {
            continue;
        }

        t = s + strlen(s) - 1;
        while (t > s && m_isspace(*t))
        {
            t--;
        }

        if (t == s)
        {
            continue;
        }
        else
        {
            if (*(t + 1))
            {
                t++;
            }
            *t = '\0';
        }

        names[i] = xcalloc(1, sizeof(NA));

        if ((c = strchr(s, '@')) != NULL)
        {
            /* Then we have an internet/bangpath. */

            names[i]->addr.fidonet = 0;
            if (c == s)
            {
                names[i]->addr.bangpath = 1;
                c++;
            }
            else
            {
                names[i]->addr.internet = 1;
                c = s;
            }
            names[i]->name = xstrdup(c);
            names[i]->addr.domain = xstrdup(c);
        }
        else
        {
            c = strchr(s, '!');

            if (c)
            {
                *c = '\0';
                names[i]->addr = parsenode(++c);
                names[i]->name = xstrdup(s);
            }
            else
            {
                names[i]->name = addr_lookup(s, &names[i]->addr);

                /* UUCP address from alias? */

                if ((c = strchr(names[i]->name, '@')) != NULL)
                {
                    names[i]->addr.fidonet = 0;
                    if (c == names[i]->name)
                    {
                        names[i]->addr.bangpath = 1;
                        c++;
                    }
                    else
                    {
                        names[i]->addr.internet = 1;
                        c = names[i]->name;
                    }
                    names[i]->addr.domain = xstrdup(c);
                }
            }
        }

        if (names[i]->addr.notfound)
        {
            names[i]->addr = CurArea.addr;
            if (CurArea.addr.domain)
            {
                names[i]->addr.domain = xstrdup(CurArea.addr.domain);
            }
        }
        i++;
    }
    names[i] = NULL;
    *idx = i;
    fclose(fp);
}

/*
 *  Saves a message if the first line does not contain "CC:" or "XC:",
 *  else it continues and either makes a carbon copy (or copies) or
 *  crosspost(s) of the message.
 *
 *  The idea here is: First get the addresses from the CC: list, if there
 *  is one (if not, check for XC: and leave).  If so, we then make up the
 *  text for the message (in a loop), then we go and save the messages using
 *  the names and addresses from the CC: list.
 */

void save(msg * m)
{
    LINE *current;
    NA *names[128];
    int num = 1, i;
    unsigned long k;
    int verbose_cc = 0;
    int blind_cc = 0;
    char *s, *t, *sp;

    current = m->text;
    do_lookup = TRUE;

/*    if (m->new) */
    {
        if (!strncmpi(current->text, "xc:", 3) && !CurArea.netmail)
        {
            if (m == message)
            {
                message = NULL;
                scan_base = 1;
            }
            crosspost(m);
            return;
        }
    }
    current = m->text;
    s = current->text;

    if (!strncmpi(current->text, "bc:", 3))
    {
        blind_cc = 1;
    }
    else if (!strncmpi(current->text, "hc:", 3))
    {
        blind_cc = 1;
    }
    else if (!strncmpi(current->text, "vc:", 3))
    {
        verbose_cc = 1;
    }
    else if (strncmpi(current->text, "cc:", 3))
    {
        writemsg(m);
        scan_base = TRUE;
        return;
    }
    release(m->reply);

    memset(names, 0, sizeof names);

    names[0] = xmalloc(sizeof(NA));

    names[0]->addr = m->to;
    if (m->to.internet || m->to.bangpath)
    {
        names[0]->name = compose_internet_address(m->to.domain, m->isto);
        names[0]->addr = m->to;
        names[0]->addr.domain = xstrdup(names[0]->name);
    }
    else
    {
        names[0]->name = xstrdup(m->isto);
        if (m->to.domain)
        {
            names[0]->addr.domain = xstrdup(m->to.domain);
        }
    }

    m->attrib.sent = 1;
    if (SW->savecc && SW->rawcc)
    {
        writemsg(m);
        m->attrib.killsent = 1;
    }
    m->attrib.sent = 0;


    /* Get names and addresses from CC: list. */

    while (current && *s && !strncmpi(s + 1, "c:", 2))
    {
        s += 3;
        while (*s && m_isspace(*s))
        {
            s++;
        }

        if (*s == '~')
        {
            sp = strchr(s, '\n');
            if (sp != NULL)
            {
                *sp = '\0';
            }
            GetFileCCs(s + 1, names, &num);
        }
        else
        {
            while (*s != '\0' && s != NULL)
            {
                while (*s && m_isspace(*s))
                {
                    s++;
                }
                if (*s == '\0')
                {
                    break;
                }

                /* get next name on line */

                t = strchr(s, ',');

                if (t)
                {
                    *t = '\0';
                }
                else
                {
                    t = strchr(s, '\n');
                    if (t)
                    {
                        *t = '\0';
                    }
                    else
                    {
                        t = s + strlen(s) - 1;
                    }
                }

                names[num] = xcalloc(1, sizeof(NA));

                /* UUCP address? */

                if ((sp = strchr(s, '@')) != NULL)
                {
                    names[num]->addr.fidonet = 0;
                    if (sp == s)
                    {
                        names[num]->addr.bangpath = 1;
                    }
                    else
                    {
                        names[num]->addr.internet = 1;
                    }
                    names[num]->name = xstrdup(s);
                    names[num]->addr.domain = xstrdup(s);
                }
                else
                {
                    /* look for address */

                    sp = strchr(s, '!');

                    if (sp)
                    {
                        *sp = '\0';
                    }

                    if (sp)
                    {
                        names[num]->addr = parsenode(++sp);
                        names[num]->name = xstrdup(s);
                    }
                    else
                    {
                        names[num]->name = addr_lookup(s, &names[num]->addr);

                        /* UUCP address from alias? */

                        sp = strchr(names[num]->name, '@');
                        if (sp != NULL)
                        {
                            names[num]->addr.fidonet = 0;
                            if (sp == names[num]->name)
                            {
                                names[num]->addr.bangpath = 1;
                                sp++;
                            }
                            else
                            {
                                names[num]->addr.internet = 1;
                                sp = names[num]->name;
                            }
                            names[num]->addr.domain = xstrdup(sp);
                        }
                    }
                }
                s = ++t;
                num++;
            }
        }
        if (current->next)
        {
            current = current->next;
        }
        else
        {
            i = 0;
            while (names[i] != NULL)
            {
                if (names[i]->name)
                {
                    release(names[i]->name);
                }
                if (names[i]->addr.domain)
                {
                    release(names[i]->addr.domain);
                }
                release(names[i]);
            }
            return;
        }

        s = current->text;
        m->text = current;
        release(current->prev->text);
        release(current->prev);
        current->prev = NULL;
    }

    names[num] = NULL;

    if (blind_cc == 0)
    {
        if (verbose_cc)
        {
            CreateVerboseCC(m, names);
        }
        else
        {
            CreateNormalCC(m, names);
        }
    }

    i = 0;

    release(m->msgid);
    k = MsgnToUid(CurArea.messages) + 1;
    m->new = 1;

    while (names[i] != NULL)
    {
        release(m->isto);
        release(m->to.domain);

        m->to = names[i]->addr;

        if (names[i]->addr.domain)
        {
            m->to.domain = xstrdup(names[i]->addr.domain);
        }

        if (names[i]->addr.internet || names[i]->addr.bangpath)
        {
            m->isto = construct_uucpname(m->to.domain);
        }
        else
        {
            m->isto = xstrdup(names[i]->name);
        }


        /* for *.msg bases - get the real msg num */

        m->msgnum = k++;
        writemsg(m);

        if (SW->savecc && !SW->rawcc)
        {
            m->attrib.killsent = 1;
        }

        release(names[i]->name);
        release(names[i]->addr.domain);
        i++;
    }

    scan_base = TRUE;
}

static void crosspost(msg * m)
{
    LINE *current;
    char *s, *t;
    char dfn[128];
    char *arean[128];
    unsigned int num = 0, i, k;

    current = m->text;
    s = current->text;

    arean[num] = xstrdup(CurArea.tag);
    num++;

    while (current && *s && !strncmpi(s, "xc:", 3))
    {
        s += 3;

        while (*s && m_isspace(*s))
        {
            s++;
        }

        while (*s != '\0' && s != NULL)
        {
            t = strchr(s, ',');
            if (t)
            {
                *t = '\0';
            }
            else
            {
                t = strchr(s, '\n');
                if (t)
                {
                    *t = '\0';
                }
                else
                {
                    t = s + strlen(s) - 1;
                }
            }
            while (*s && m_isspace(*s))
            {
                s++;
            }
            arean[num] = xstrdup(strupr(s));

            if (arean[num++] == NULL)
            {
                break;
            }

            s = ++t;
        }
        if (current->next)
        {
            current = current->next;
        }
        else
        {
            i = 0;
            while (arean[i] != NULL)
            {
                release(arean[i++]);
            }
            return;
        }
        s = current->text;
        m->text = current;

        if (current->prev->text)
        {
            release(current->prev->text);
        }

        release(current->prev);
        current->prev = NULL;
    }

    arean[num] = NULL;

    if (strncmpi(s, "cc:", 3) != 0)
    {
        m->text = lineins(m->text);
        m->text->text = xstrdup("\n");
        current = m->text;
        i = 0;
        strcpy(dfn, " * Crossposted in area ");

        while (arean[i] != NULL)
        {
            if (strlen(arean[i]) + 3 + strlen(dfn) >= SW->rm)
            {
                strcat(dfn, "\n");
                release(current->text);
                current->text = xstrdup(dfn);
                current = lineins(current->next);
                current->text = xstrdup("\n");
                strcpy(dfn, "                       ");
            }
            strcat(dfn, arean[i]);
            i++;
            if (arean[i] != NULL)
            {
                strcat(dfn, ", ");
            }
        }

        if (*(current->text) == '\n')
        {
            release(current->text);
            strcat(dfn,"\n");
            current->text = xstrdup(dfn);
        }
    }

    i = 0;
    while (arean[i] != NULL)
    {
        for (k = 0; k < SW->areas; k++)
        {
            if (arealist[k].tag && !stricmp(arean[i], arealist[k].tag))
            {
                break;
            }
        }
        if (k == SW->areas || !arealist[k].echomail)
        {
            release(arean[i]);
            i++;
            continue;
        }
        set_nongrouped_area(k);
        release(m->from.domain);
        m->from = CurArea.addr;

        if (CurArea.addr.domain)
        {
            m->from.domain = xstrdup(CurArea.addr.domain);
        }

        if (CurArea.status)
        {
            if (i > 0)
            {
                m->new = 1;
            }

            if (m->new)
            {
                m->msgnum = MsgnToUid(CurArea.messages) + 1;
            }

            if (strncmpi(s, "cc:", 3) == 0)
            {
                save(m);
            }
            else
            {
                writemsg(m);
            }
        }
        release(arean[i]);
        i++;
    }
    scan_base = TRUE;           /* reopen the base when we get back */
}

static void spawn_command(char *cmdline)
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
        rc = do_spawn(swapping, ST->comspec, cmdline, envlen, envptr);
    }
    else
    {
        fprintf(stderr, "\nError occured during do_spawn(); rc=%d. Press Enter to return...", rc);
        while (GetKey() != 13)
        {
        }
    }
#else
    system(cmdline);
#endif
}

static int externalEditor(msg * m)
{
    FILE *fq;
    LINE *current;
    static char cmd[200];
    WND *hWnd, *hCurr;
    static char linebuf[1000];
    size_t linelen;
    int hardquote, linenum;
    static char tmpfnm[] = "msged.tmp";
    int doformat, statready;
    struct stat ostat, nstat;

    doformat = SW->extformat;
    hardquote = 0;
    current = m->text;
    fq = fopen(tmpfnm, "w");
    if (fq != NULL)
    {
        while (current != NULL)
        {
            if (current->text != NULL)
            {
                fputs(current->text, fq);
            }
            current = current->next;
        }
        fclose(fq);
        statready = stat(tmpfnm, &ostat) == 0;

        maxx = term.NCol;
        maxy = term.NRow;

        hCurr = WndTop();
        hWnd = WndOpen(0, 0, term.NCol - 1, term.NRow - 1, NBDR, 0, cm[CM_NTXT]);
        WndCurr(hWnd);
        MouseOFF();
        cursor(1);

#if defined(MSDOS) && !defined(NODOSSWAP) && !defined(__FLAT__)
        sprintf(cmd, "/c %s %s", ST->editorName, tmpfnm);
#else
        sprintf(cmd, "%s %s", ST->editorName, tmpfnm);
#endif
        spawn_command(cmd);

        cursor(0);
        MouseON();
        WndClose(hWnd);
        WndCurr(hCurr);

        if (statready && stat(tmpfnm, &nstat) == 0 &&
          ostat.st_mtime == nstat.st_mtime && ostat.st_size == nstat.st_size)
        {
            /*
             *  Old and new timestamps and file sizes are identical, so no
             *  modifications were performed.
             */
            return ABORT;
        }

        m->text = clearbuffer(m->text);

        fq = fopen(tmpfnm, "r");
        if (fq != NULL)
        {
            linenum = 0;
            while (fgets(linebuf, sizeof linebuf, fq) != NULL)
            {
                linenum++;
                if (*(linebuf + strlen(linebuf) - 1) != '\n')
                {
                    strcat(linebuf, "\n");
                }
                if (current == NULL)
                {
                    m->text = xcalloc(1, sizeof(LINE));
                    current = m->text;
                }
                else
                {
                    current = InsertLine(current);
                }
                if (doformat)
                {
                    if (!hardquote)
                    {
                        linelen = strlen(linebuf);

                        if (strcmp(linebuf, "<<\n") == 0)
                        {
                            /* begin temporary hardquoting */
                            if (linenum == 1)
                            {
                                *linebuf = '\0';
                            }
                            else
                            {
                                strcpy(linebuf, "\n");
                            }
                            hardquote = 1;
                        }
                        else if (linebuf[linelen - 2] == '<' && linebuf[linelen - 3] == '<')
                        {
                            /*
                             *  The following checks for << at end of line
                             *  and replaces them with CR and \0.
                             */
                            linebuf[linelen - 3] = '\r';
                            linebuf[linelen - 2] = '\0';
                        }
                        else
                        {
                            /*
                             *  If not a quote and not prefixed with a
                             *  'special' character and greater than
                             *  width - 30? then remove CR
                             */

                            if (linelen > term.NCol - 30 && !isquote(linebuf) &&
                              !strchr("\01 !@#$%^&*~-_+=:;,./", *linebuf))
                            {
                                linebuf[linelen - 1] = '\0';
                            }
                        }
                    }
                    else
                    {
                        if (strcmp(linebuf, "<<\n") == 0)
                        {
                            /* end temporary hardquoting */
                            strcpy(linebuf, "\n");
                            hardquote = 0;
                        }
                    }
                }
                current->text = xstrdup(linebuf);
            }
            fclose(fq);
        }
    }

    if (current == NULL)
    {
        return ABORT;
    }

    return SAVE;
}

