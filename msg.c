/*
 *  MSG.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Note: Because the SDM routines are NOT the same (the Msgn are the
 *  same as the UIDs, so the Msgn's are not contiguous), I basically had
 *  to create my own API to distance my code from the real API.  (Making
 *  it easy to integrate other message types).  All msgbase specific
 *  stuff should be kept within the module that handles it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#if !defined(UNIX) && !defined(SASC)
#include <io.h>
#endif

#ifdef UNIX
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __MSC__
#include <sys/locking.h>
#endif

#if !defined(UNIX) && !defined(SASC)
#include <share.h>
#endif

#include <smapi/msgapi.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "memextra.h"
#include "date.h"
#include "normal.h"
#include "unused.h"
#include "msg.h"
#include "charset.h"

static char cinfbuf[BUFLEN];    /* control info buffer */

static unsigned long num_msgs;  /* number of messages in msgbase */
static unsigned long new = 0;   /* if msg being written is new */

static time_t stampToTimeT(struct _stamp *st);
static struct _stamp *timeTToStamp(time_t);

static struct _minf minf;
extern word _stdc msgapierr;

static MSG *Ahandle = NULL;     /* area handle */
static MSGH *mh = NULL;         /* message handle */
static XMSG xmsg;               /* squish message header */

static UMSGID replyto = 0;      /* to ensure correct uplinks when mxx is used */

/*
 *  SquishMsgAreaOpen; Scans an area for messages, opening the message
 *  base and filling the message[] array.
 */

long SquishMsgAreaOpen(AREA * a)
{
    unsigned long lastread;     /* lastread record */
    char work[256];             /* path to sql file */
    int sql;                    /* file handle */
    unsigned long k = 0;        /* counter */
    struct stat bstat;          /* stats for sql file */
    unsigned char buffer[4];    /* for reading and writing the sql */
    
    a->scanned = 1;
    a->last = 1;
    a->first = 1;
    a->lastread = 0;
    a->current = 0;
    a->status = 0;

    /* open the msgbase */

    if (mh != NULL)
    {
        MsgCloseMsg(mh);
        mh = NULL;
    }

    if (Ahandle != NULL)
    {
        if (SW->squish_lock)
        {
            MsgUnlock(Ahandle);
        }

        if (MsgCloseArea(Ahandle) == -1)
        {
            return 0;
        }
    }

    Ahandle = MsgOpenArea((byte *)a->path, MSGAREA_CRIFNEC,
                          ((a->msgtype == JAM) ? MSGTYPE_JAM:MSGTYPE_SQUISH));
    if (Ahandle == NULL)
    {
        return 0;
    }

    if (SW->squish_lock)
    {
        if (MsgLock(Ahandle) == -1)  /* Lock failed - return */
        {
            MsgCloseArea(Ahandle);
            Ahandle = NULL;
            return 0;
        }
    }

    sprintf(work, "%s.sql", a->path);
    sql = sopen(work, O_BINARY | O_RDONLY, SH_DENYNO, S_IWRITE | S_IREAD);
    if (sql != -1)
    {
#if defined(PACIFIC) || defined(LATTICE)
        stat(work, &bstat);
#else
        fstat(sql, &bstat);
#endif
        /* we make it big enough */
        if (bstat.st_size < SW->useroffset * 4)
        {
            close(sql);
            sql = sopen(work, O_BINARY | O_RDWR, SH_DENYNO, S_IWRITE | S_IREAD);
            if (sql != -1)
            {
#if defined(PACIFIC) || defined(LATTICE)
                stat(work, &bstat);
#else
                fstat(sql, &bstat);
#endif
                lastread = 0;
                buffer[0] = buffer[1] = buffer[2] = buffer[3] = '\0';
                k = bstat.st_size / 4;
                lseek(sql, 0L, SEEK_END);
                while (SW->useroffset > k)
                {
                    farwrite(sql, buffer, 4);
                    k++;
                }
            }
        }
        else
        {
            /* we read the data in */
            lseek(sql, SW->useroffset * 4, SEEK_SET);
            if (farread(sql, buffer, 4) == 4)
            {
                lastread = buffer[0] + (((unsigned long)(buffer[1])) << 8) +
                    (((unsigned long)(buffer[2])) << 16) +
                    (((unsigned long)(buffer[3])) << 24);

                if (CurArea.netmail)
                {
                    a->lastread = MsgUidToMsgn(Ahandle, lastread, UID_PREV);
                }
                else
                {
                    a->lastread = MsgUidToMsgn(Ahandle, lastread, UID_NEXT);
                }
                a->current = a->lastread;
            }
        }
        close(sql);
    }

    a->last = MsgHighMsg(Ahandle);
    a->status = 1;

    if (a->last >= 1 && a->current == 0)
    {
        a->lastread = 0;
        a->current = 1;
    }

    return a->last;
}

/*
 *  SquishMsgReadHeader; Reads in the message header and control
 *  information for the message.
 */

msg *SquishMsgReadHeader(unsigned long n, int type)
{
    char path[PATHLEN];
    msg *m;
    int i = 0;

    if (Ahandle == NULL)
    {
        return NULL;
    }

    if (mh != NULL)
    {
        /* if open, close it */
        MsgCloseMsg(mh);
    }

    /* open msg we want to open */
    mh = MsgOpenMsg(Ahandle, MOPEN_READ, n);
    if (mh == NULL)
    {
        return NULL;
    }

    if (MsgReadMsg(mh, &xmsg, 0L, 0L, NULL, BUFLEN, (byte *) cinfbuf) == (dword) - 1)
    {
        /* no message header or control info! */
        MsgCloseMsg(mh);
        mh = NULL;
        return NULL;
    }

    m = xcalloc(1, sizeof *m);

    /* basically copy info across to msg */
    m->msgnum = MsgMsgnToUid(Ahandle, n);
    m->from.zone = xmsg.orig.zone;
    m->from.net = xmsg.orig.net;
    m->from.node = xmsg.orig.node;
    m->from.point = xmsg.orig.point;

    m->to.zone = xmsg.dest.zone;
    m->to.net = xmsg.dest.net;
    m->to.node = xmsg.dest.node;
    m->to.point = xmsg.dest.point;

    m->from.domain = NULL;
    m->to.domain = NULL;

    if (xmsg.date_written.date.yr != 0)
    {
        m->timestamp = stampToTimeT(&xmsg.date_written);
        m->time_arvd = stampToTimeT(&xmsg.date_arrived);
    }
    else
    {
        /* only use this when necesary */
        memset(path, 0, sizeof path);
        memcpy(path, xmsg.__ftsc_date, sizeof xmsg.__ftsc_date);
        m->timestamp = parsedate(path);
    }

    m->isto = xcalloc(1, sizeof xmsg.to + 1);
    m->isfrom = xcalloc(1, sizeof xmsg.from + 1);
    m->subj = xcalloc(1, sizeof xmsg.subj + 1);

    memcpy(m->isto, xmsg.to, sizeof xmsg.to);
    memcpy(m->isfrom, xmsg.from, sizeof xmsg.from);
    memcpy(m->subj, xmsg.subj, sizeof xmsg.subj);

    m->attrib.priv = (xmsg.attr & MSGPRIVATE) != 0;
    m->attrib.crash = (xmsg.attr & MSGCRASH) != 0;
    m->attrib.rcvd = (xmsg.attr & MSGREAD) != 0;
    m->attrib.sent = (xmsg.attr & MSGSENT) != 0;
    m->attrib.attach = (xmsg.attr & MSGFILE) != 0;
    m->attrib.forward = (xmsg.attr & MSGFWD) != 0;
    m->attrib.orphan = (xmsg.attr & MSGORPHAN) != 0;
    m->attrib.killsent = (xmsg.attr & MSGKILL) != 0;
    m->attrib.local = (xmsg.attr & MSGLOCAL) != 0;
    m->attrib.hold = (xmsg.attr & MSGHOLD) != 0;
    m->attrib.direct = (xmsg.attr & MSGXX2) != 0;
    m->attrib.freq = (xmsg.attr & MSGFRQ) != 0;
    m->attrib.rreq = (xmsg.attr & MSGRRQ) != 0;
    m->attrib.rcpt = (xmsg.attr & MSGCPT) != 0;
    m->attrib.areq = (xmsg.attr & MSGARQ) != 0;
    m->attrib.ureq = (xmsg.attr & MSGURQ) != 0;

    m->attrib.lock = (xmsg.attr & MSGLOCKED) != 0;

    if (xmsg.attr & MSGSCANNED)
    {
        m->scanned = 1;
    }

    if (type != RD_HEADER_BRIEF)
    {
        m->replyto = MsgUidToMsgn(Ahandle, xmsg.replyto, UID_EXACT);

        while (i < 9)
        {
            if (xmsg.replies[i] != 0)
            {
                m->replies[i] = MsgUidToMsgn(Ahandle, xmsg.replies[i], UID_EXACT);
            }
            else
            {
                m->replies[i] = 0;
            }
            i++;
        }
        m->replies[9] = 0;
    }

    m->cost = 0;
    m->times_read = 0;
    m->text = NULL;
    m->to.fidonet = 1;
    m->from.fidonet = 1;

    if (type == RD_HEADER || type == RD_HEADER_BRIEF)
    {
        MsgCloseMsg(mh);
        mh = NULL;
    }

    return m;
}

/*
 *  SquishMsgReadText; Reads in the entire message, adds the control
 *  information to the beginning of the message and continues to return
 *  the message to the caller line by line (at each subsequent call).
 *  Basically a conversion of the correspoding Fido function.
 */

char *SquishMsgReadText(unsigned long n)
{
    static char *next = NULL;
    static char *end = NULL;
    char *t = NULL;
    char *text = NULL;
    char eol = '\0';
    unsigned long i, l;
    static unsigned long ofs = 0, s = 0;

    unused(n);
    if (Ahandle == NULL)
    {
        return NULL;
    }

    if (next == NULL && s != 0)
    {
        /* we are finished */
        s = ofs = 0;
        return NULL;
    }

    if (s == 0)
    {
        /* ready to read in new msg */
        memset(msgbuf, 0, BUFLEN - 1);
        next = msgbuf;
        if (MsgGetCtrlLen(mh) > 0)
        {
            /* copy control info from */
            t = cinfbuf;  /* insert /r's */
            *(t + (size_t) MsgGetCtrlLen(mh)) = '\0';
            if (*t != '\0')
            {
                *next++ = *t++;
                while (*t != '\0')
                {
                    if (*t == '\01')
                    {
                        *next++ = '\r';  /* add a \r to the text */
                    }
                    *next++ = *t++;
                }
                if (*(next - 1) == '\01')
                {
                    next--;
                    *(next) = '\0';
                }
                else
                {
                    *next++ = '\r';
                    *next = '\0';  /* terminate string         */
                }
            }
            next = msgbuf;
            end = msgbuf + strlen(msgbuf);
            normalize(msgbuf);
        }
        else
        {
            next = NULL;
        }

        s = BUFLEN;
    }

    /* return msg a line at a time */
    if (next == NULL)
    {
        i = MsgReadMsg(mh, NULL, ofs, s - 1, (byte *) msgbuf, 0L, NULL);
        ofs += i;
        if (i < 1)
        {
            s = ofs = 0;
            next = NULL;
            return NULL;
        }
        next = msgbuf;
        while (i && *next == '\0')
        {
            i--;
            next++;
        }
        normalize(next);
        end = msgbuf + strlen(msgbuf);
        if (end < next)
        {
            next = end;
        }
    }

    if (end - next == 0)
    {
        t = NULL;
    }
    else
    {
        t = memchr(next, '\n', (int)(end - next));
    }

    if (t == NULL)
    {
        l = strlen(next);
        memmove(msgbuf, next, (size_t) (l + 1));
        i = MsgReadMsg(mh, NULL, ofs, s - l - 1,
          (byte *)(msgbuf + (size_t) l), 0L, NULL);
        ofs += i;
        if (i < 1)
        {
            next = NULL;
            return xstrdup(msgbuf);
        }
        *(msgbuf + (size_t) l + (size_t) i) = '\0';
        normalize(msgbuf + (size_t) l);
        end = msgbuf + strlen(msgbuf);
        next = msgbuf;
        t = strchr(next, '\n');
    }

    if (t != NULL)
    {
        eol = *(t + 1);
        *(t + 1) = '\0';
    }

    text = xstrdup(next);

    if (t != NULL)
    {
        *(t + 1) = eol;
        next = t + 1;
    }
    else
    {
        next = NULL;
    }

    return text;
}

/*
 *  SquishMsgWriteHeader; Writes message header to the message base,
 *  creates new frame if new message, and makes sure links are correct.
 */

int SquishMsgWriteHeader(msg * m, int type)
{
    unsigned long n;
    int i = 0;

    n = MsgUidToMsgn(Ahandle, m->msgnum, UID_EXACT);

    if (Ahandle == NULL)
    {
        return FALSE;
    }

    if (mh != NULL)  /* close old msg, if left open */
    {
        MsgCloseMsg(mh);
    }

    if (m->new)
    {
        /*
         *  If new, store current number of messages (for use in
         *  SquishMsgWriteText, and create a new frame for the new
         *  message.
         */

        num_msgs = MsgGetNumMsg(Ahandle);
        mh = MsgOpenMsg(Ahandle, MOPEN_CREATE, 0L);
        if (mh == NULL)
        {
            return ERR_OPEN_MSG;
        }
        new = TRUE;
    }
    else
    {
        /* else we open the message to be changed */
        mh = MsgOpenMsg(Ahandle, MOPEN_RW, n);
        if (mh == NULL)
        {
            return ERR_OPEN_MSG;
        }
        new = FALSE;
    }

    memset(&xmsg, 0, sizeof xmsg);

    xmsg.attr = 0;
    if (m->attrib.priv)
    {
        xmsg.attr |= MSGPRIVATE;
    }
    if (m->attrib.crash)
    {
        xmsg.attr |= MSGCRASH;
    }
    if (m->attrib.rcvd)
    {
        xmsg.attr |= MSGREAD;
    }
    if (m->attrib.sent)
    {
        xmsg.attr |= MSGSENT;
    }
    if (m->attrib.attach)
    {
        xmsg.attr |= MSGFILE;
    }
    if (m->attrib.forward)
    {
        xmsg.attr |= MSGFWD;
    }
    if (m->attrib.orphan)
    {
        xmsg.attr |= MSGORPHAN;
    }
    if (m->attrib.killsent)
    {
        xmsg.attr |= MSGKILL;
    }
    if (m->attrib.local)
    {
        xmsg.attr |= MSGLOCAL;
    }
    if (m->attrib.hold)
    {
        xmsg.attr |= MSGHOLD;
    }
    if (m->attrib.direct)
    {
        xmsg.attr |= MSGXX2;
    }
    if (m->attrib.freq)
    {
        xmsg.attr |= MSGFRQ;
    }
    if (m->attrib.rreq)
    {
        xmsg.attr |= MSGRRQ;
    }
    if (m->attrib.rcpt)
    {
        xmsg.attr |= MSGCPT;
    }
    if (m->attrib.areq)
    {
        xmsg.attr |= MSGARQ;
    }
    if (m->attrib.ureq)
    {
        xmsg.attr |= MSGURQ;
    }
    if (m->attrib.lock)
    {
        xmsg.attr |= MSGLOCKED;
    }

    if (new == FALSE)
    {
        /*
         *  If the old message had been scanned, then we make sure that
         *  the MSGSCANNED bit is set on the way out. New messages get
         *  this bit stripped.
         */

        if (m->scanned && !m->new)
        {
            xmsg.attr |= MSGSCANNED;
        }
    }

    if (m->replyto != 0)
    {
        /* get the links for replies */
        xmsg.replyto = MsgMsgnToUid(Ahandle, m->replyto);
    }

    for (i = 0; i < 9; i++)
    {
        if (m->replies[i] != 0)
        {
            xmsg.replies[i] = MsgMsgnToUid(Ahandle, m->replies[i]);
        }
    }

    i = 0;

    while (m->replies[i] != 0 && i < 9)
    {
        i++;
    }

    if (i == 9)
    {
        i = 8;
    }

    if (!m->new && replyto != 0)
    {
        xmsg.replies[i] = replyto;
        replyto = 0;
    }

    xmsg.dest.zone = (word) m->to.zone;
    xmsg.dest.net = (word) m->to.net;
    xmsg.dest.node = (word) m->to.node;
    xmsg.dest.point = (word) m->to.point;

    xmsg.orig.zone = (word) m->from.zone;
    xmsg.orig.net = (word) m->from.net;
    xmsg.orig.node = (word) m->from.node;
    xmsg.orig.point = (word) m->from.point;

    if (m->isto != NULL)
    {
        memcpy(xmsg.to, m->isto, min(sizeof xmsg.to, strlen(m->isto)));
    }

    if (m->isfrom != NULL)
    {
        memcpy(xmsg.from, m->isfrom, min(sizeof xmsg.from, strlen(m->isfrom)));
    }

    if (m->subj != NULL)
    {
        memcpy(xmsg.subj, m->subj, min(sizeof xmsg.subj, strlen(m->subj)));
    }

    memcpy(xmsg.__ftsc_date, mtime(m->timestamp), 20);

    xmsg.date_written = *timeTToStamp(m->timestamp);

    if (m->time_arvd != 0)
    {
        xmsg.date_arrived = *timeTToStamp(m->time_arvd);
    }
    else
    {
        xmsg.date_arrived = xmsg.date_written;
    }

    if (type == WR_HEADER || !new)
    {
        MsgWriteMsg(mh, FALSE, &xmsg, NULL, 0L, 0L, 0L, NULL);
        MsgCloseMsg(mh);
        mh = NULL;
    }

    return TRUE;
}

/*
 *  strip_whitel; Strips the white spaces from the control info,
 *  copying it to the msgbuf array while it's at it.  Returns new
 *  length.
 */

static dword strip_whitel(void)
{
    char *s, *c, *cptr;

    /* we put it in the cinfbuf, killing any \r & \n's in the process */
    cptr = cinfbuf;
    s = msgbuf;
    c = s + strlen(s) + 1;
    while (s != c)
    {
        /* copy buffer across, ignoring any fluff in the source buffer */
        switch (*s)
        {
        case '\r':
        case '\n':
            s++;
            break;
        default:
            *cptr++ = *s++;
            break;
        }
    }
    *cptr = '\0';
    return cptr - cinfbuf;
}


/*
 *  SquishMsgWriteText; Writes message text (and header if a new
 *  message), to the message base.  If the message is not new and
 *  new text is larger, it creates a new frame, while keeping the
 *  same position in the index.
 */

int SquishMsgWriteText(char *text, unsigned long msgn, unsigned long mlen)
{
    static char *tptr, *c;
    static int ready = FALSE, ctrl = TRUE;
    static unsigned long n = 0;
    char cz = 0;
    unsigned long clen;

    if (Ahandle == NULL)
    {
        return FALSE;
    }

    if (ready == FALSE)
    {
        /* starting on new message; reset pointers */
        ready = TRUE;
        tptr = msgbuf;
        n = MsgUidToMsgn(Ahandle, msgn, UID_EXACT);
    }

    if (text == NULL)
    {
        if (ctrl)
        {
            /* no body in the message */
            if (new)
            {
                clen = strip_whitel();
                MsgWriteMsg(mh, FALSE, &xmsg, NULL, 0L, mlen, clen,
                  (byte *)cinfbuf);
                MsgWriteMsg(mh, TRUE, NULL, (byte *)&cz,
                  sizeof(char), mlen, 0L, NULL);
            }
            else
            {
                mh = MsgOpenMsg(Ahandle, MOPEN_RW, n);
                if (mh == NULL)
                {
                    ready = FALSE;
                    ctrl = TRUE;
                    new = FALSE;  /* we only change the header */
                    return FALSE;
                }
                MsgWriteMsg(mh, FALSE, &xmsg, NULL, 0L, mlen, 0L, NULL);
            }
        }
        else
        {
            MsgWriteMsg(mh, TRUE, NULL, (byte *)&cz, sizeof(char),
              mlen, 0L, NULL);
        }

        if (new)
        {
            /*
             *  Message is a reply - save new number so next header
             *  written can use it for the uplink number.
             */

            if (xmsg.replyto)
            {
                replyto = MsgMsgnToUid(Ahandle, MsgGetNumMsg(Ahandle));
            }
            if (num_msgs == MsgGetNumMsg(Ahandle))
            {
                CurArea.messages--;
            }
        }

        new = ready = FALSE;
        ctrl = TRUE;
        return TRUE;
    }

    if (*text == '\01' && ctrl)
    {
        c = text;
        while (*c != '\0')
        {
            /* store the control info */
            *tptr++ = *c++;
        }
        *tptr = '\0';
    }
    else
    {
        if (*text != '\01' && ctrl)
        {
            ctrl = FALSE;
            clen = strip_whitel();
            if (!new)
            {
                /* we are modifying a non-new message */
                mh = MsgOpenMsg(Ahandle, MOPEN_RW, n);
                if (mh == NULL)
                {
                    ready = FALSE;
                    ctrl = TRUE;
                    new = FALSE;
                    return FALSE;
                }

                if (MsgReadMsg(mh, &xmsg, 0L, 0L, NULL, 0L, NULL) == (dword) - 1)
                {
                    new = FALSE;
                    ready = FALSE;
                    ctrl = TRUE;
                    return FALSE;
                }

                MsgCloseMsg(mh);  /* copy xmsg information across */

                mh = MsgOpenMsg(Ahandle, MOPEN_CREATE, n);
                if (mh == NULL)
                {
                    ready = FALSE;
                    ctrl = TRUE;
                    new = FALSE;
                    return FALSE;
                }

                /* messy, but it works */
                MsgWriteMsg(mh, FALSE, &xmsg, (byte *)text,
                  strlen(text), mlen, clen, (byte *)cinfbuf);
            }
            else
            {
                MsgWriteMsg(mh, FALSE, &xmsg, (byte *)text,
                  strlen(text), mlen, clen, (byte *)cinfbuf);
            }
        }
        else
        {
            MsgWriteMsg(mh, TRUE, NULL, (byte *)text, strlen(text),
              mlen, 0L, NULL);
        }
    }

    return TRUE;
}

/*
 *  SquishMsgClose; Closes the message currently opened.
 */

int SquishMsgClose(void)
{
    if (mh == NULL)
    {
        return TRUE;
    }
    if (MsgCloseMsg(mh) == -1)
    {
        printf("\n!SquishMsgClose(): Message didn't close, error %ud!\n", msgapierr);
        exit(-1);
        return ERR_CLOSE_MSG;
    }
    else
    {
        mh = NULL;
        return TRUE;
    }
}


/*
 * Area locking functions
 */

int SquishMsgLock(void)
{
    if (!SW->squish_lock)
    {
        return MsgLock(Ahandle);
    }
    else
    {
        return 0;  /* area already locked */
    }
}

int SquishMsgUnlock(void)
{
    if (!SW->squish_lock)
    {
        return MsgUnlock(Ahandle);
    }
    else
    {
        return 0;
    }
}


/*
 *  SquishMsgAreaClose; Closes the area currently opened.
 */

int SquishMsgAreaClose(void)
{
    if (Ahandle == NULL)
    {
        return TRUE;
    }

    if (SW->squish_lock)
    {
        MsgUnlock(Ahandle);
    }

    if (MsgCloseArea(Ahandle) == -1)
    {
        printf("\n!SquishMsgAreaClose(): Area didn't close, error %ud!\n", msgapierr);
        exit(-1);
        return ERR_CLOSE_AREA;
    }
    else
    {
        CurArea.status = 0;
        Ahandle = NULL;
        return TRUE;
    }
}

/*
 *  SquishUidToMsgn; Returns the corresponding message number of a UID.
 */

unsigned long SquishUidToMsgn(unsigned long n)
{
    return MsgUidToMsgn(Ahandle, n, UID_EXACT);
}

/*
 *  SquishMsgnToUid; Returns the corresponding UID of a message number.
 */

unsigned long SquishMsgnToUid(unsigned long n)
{
    return MsgMsgnToUid(Ahandle, n);
}

/*
 *  SquishAreaSetLast; Sets the last message read in the .sql file and
 *  closes the message area.  If the .sql file doesn't exist, create it.
 */

int SquishAreaSetLast(AREA * a)
{
    char work[255];
    long i = 1;
    int ret = TRUE;
    int fd;
    unsigned char buffer[4];
    
    if (mh != NULL)
    {
        MsgCloseMsg(mh);
        mh = NULL;
    }

    if (Ahandle != NULL)
    {
        sprintf(work, "%s.sql", a->path);

        fd = sopen(work, O_BINARY | O_RDWR, SH_DENYNO, S_IWRITE | S_IREAD);
        if (fd == -1)
        {
            if (errno != EACCES && errno != EMFILE)
            {
                fd = sopen(work, O_BINARY | O_WRONLY | O_CREAT,
                  SH_DENYNO, S_IWRITE | S_IREAD);

                if (fd == -1)
                {
                    ret = FALSE;
                }
                else
                {
                    buffer[0] = buffer[1] = buffer[2] = buffer[3] = '\0';
                    lseek(fd, 0L, SEEK_SET);
                    for (i = 0; SW->useroffset > (int)i; i++)
                    {
                        farwrite(fd, buffer, 4);
                    }

                    i = MsgMsgnToUid(Ahandle, CurArea.lastread);
                    buffer[0] = i & 0xFF;
                    buffer[1] = (i >> 8) & 0xFF;
                    buffer[2] = (i >> 16) & 0xFF;
                    buffer[3] = (i >> 24) & 0xFF;

                    farwrite(fd, buffer, 4);
                    close(fd);
                }
            }
            else
            {
                ret = FALSE;
            }
        }
        else
        {
            lseek(fd, SW->useroffset * 4, SEEK_SET);

            if (SW->use_lastr)
            {
                i = MsgMsgnToUid(Ahandle, CurArea.lastread);
            }
            else
            {
                i = MsgMsgnToUid(Ahandle, CurArea.current);
            }

            buffer[0] = i & 0xFF;
            buffer[1] = (i >> 8) & 0xFF;
            buffer[2] = (i >> 16) & 0xFF;
            buffer[3] = (i >> 24) & 0xFF;

            farwrite(fd, buffer, 4);
            close(fd);
        }
    }
    return ret;
}

/*
 *  SquishMsgDelete; Erases a message in the current area, specified
 *  by the passed index.
 */

int SquishMsgDelete(unsigned long n)
{
    unsigned long msgn;
    msgn = MsgUidToMsgn(Ahandle, n, UID_EXACT);
    if (MsgKillMsg(Ahandle, msgn) == -1)
    {
        return FALSE;
    }
    return TRUE;
}

static time_t stampToTimeT(struct _stamp *st)
{
    time_t tt;
    struct tm tms;
    tms.tm_sec = st->time.ss << 1;
    tms.tm_min = st->time.mm;
    tms.tm_hour = st->time.hh;
    tms.tm_mday = st->date.da;
    tms.tm_mon = st->date.mo - 1;
    tms.tm_year = st->date.yr + 80;
    tms.tm_isdst = -1;
    tt = mktime(&tms);
    return tt;
}

static struct _stamp *timeTToStamp(time_t tt)
{
    struct tm *tmsp;
    static struct _stamp st;
    tmsp = localtime(&tt);
    st.time.ss = tmsp->tm_sec >> 1;
    st.time.mm = tmsp->tm_min;
    st.time.hh = tmsp->tm_hour;
    st.date.da = tmsp->tm_mday;
    st.date.mo = tmsp->tm_mon + 1;
    st.date.yr = tmsp->tm_year - 80;
    return &st;
}

void MsgApiInit(void)
{
    minf.def_zone = 0;          /* set default zone */
    minf.req_version = 0;       /* level 0 of the MsgAPI */
    MsgOpenApi(&minf);          /* init the MsgAPI  */
}

void MsgApiTerm(void)
{
    MsgCloseApi();
}
