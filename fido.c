/*
 *  FIDO.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Fido/Opus style message base support functions for Msged.
 *
 *  17-Dec-91  Added share support to all file routines and converted ANSI
 *             file access code to low level read()/write().
 *  26-Jan-92  Fixed bug in msg routines that seemed to cause a crash on
 *             zero length msgs (zero, as in no ctrl info and text).
 *  23-Feb-92  Removed locking routines.
 *  01-Apr-92  Made New msgs remain open until writing the text, also added
 *             some kludge code to handle the reading of Opus msgs.
 *  12-Jul-92  Added some more functions to make the interface solid.
 *  03-Dec-92  Fix to WriteHeader.
 */

#define CHUNKSZ 256
#define TEXTLEN 96

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <smapi/progprot.h>
#include <smapi/msgapi.h>
#include <assert.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "unused.h"
#include "date.h"
#include "memextra.h"
#include "dirute.h"
#include "fido.h"


#ifdef __MSC__
#include <sys/locking.h>
#endif

#if defined(PACIFIC)

#include <unixio.h>
#define O_WRONLY 0x0001
#define O_BINARY 0x0000
#define O_CREAT  0x0000
#define O_RDWR   0x0002
#define O_RDONLY 0x0000
#define S_IREAD  0x0100
#define S_IWRITE 0x0080
#define EACCES   0x18
#define EMFILE   0x19
#define SH_DENYNO 0x40

int sopen(char *filename, unsigned int access, int flags,...);

#elif defined(SASC)

#define SH_DENYNO 0x40
#define sopen(a,b,c,d) open((a),(b))
#include <fcntl.h>
#ifndef O_BINARY
#define O_BINARY 0x0000
#endif

#elif defined(UNIX) || defined(__DJGPP__)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef sopen
#define sopen(a,b,c,d) open((a),(b),(d))
#endif

#ifndef O_BINARY
#define O_BINARY 0x0000
#endif

#else

#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <fcntl.h>

#endif

/* file access shortcuts */

#if defined(_MSC_VER) && (_MSC_VER >= 1200) && defined(_MAKE_DLL)
#   define O_RDONLY        _O_RDONLY
#   define O_WRONLY        _O_WRONLY
#   define O_RDWR          _O_RDWR
#   define O_CREAT         _O_CREAT
#   define O_BINARY        _O_BINARY
#   define S_IWRITE        _S_IWRITE
#   define S_IREAD         _S_IREAD
#   define SH_DENYNO       _SH_DENYNO
#endif


#define OPENR   O_RDONLY | O_BINARY             /* open read-only */
#define OPENC   O_WRONLY | O_BINARY | O_CREAT   /* open/create */
#define OPENRW  O_RDWR | O_BINARY               /* open read/write */


/* prototypes */

#include "normal.h"
#include "charset.h"

static int compare(const void *i, const void *j);

static void timet_to_char(time_t now, unsigned char arr[]);
static time_t char_to_timet(unsigned char arr[]);

typedef struct _fidoheader
{
    char from[36];              /* who from,             */
    char to[36];                /* who to,               */
    char subj[72];              /* message subject,      */
    char date[20];              /* creation date,        */
    unsigned char times[2];     /* number of times read, */
    unsigned char dest[2];      /* destination node,     */
    unsigned char orig[2];      /* originating node      */
    unsigned char cost[2];      /* actual cost this msg  */
    unsigned char orig_net[2];  /* originating net       */
    unsigned char dest_net[2];  /* destination net       */
    unsigned char written[4];   /* when it was written   */
    unsigned char arrived[4];   /* when it arrived       */
    unsigned char reply[2];     /* thread to previous msg */
    unsigned char attrib[2];    /* message attributes */
    unsigned char up[2];        /* thread to next msg    */
}
MFIDO;
#define MFIDO_SIZE 36+36+72+20+2+2+2+2+2+2+4+4+2+2+2
/* local vars */

static int fd = -1;                   /* current file handle */
static unsigned long *msgarr = NULL;  /* array of *.msg numbers */
static unsigned long msgarrsz = 0;    /* # of numbers in the array */
static unsigned long oldsz = 0;       /* total size of the array */

unsigned long FidoMsgnToUid(unsigned long n)
{
    if (n > msgarrsz || n == 0)
    {
        return 0;
    }
    return msgarr[(size_t) (n - 1)];
}

unsigned long FidoUidToMsgn(unsigned long n)
{
    unsigned long i;

    for (i = 0; i < msgarrsz; i++)
    {
        if (msgarr[(size_t) i] == n)
        {
            break;
        }
    }

    return (i == msgarrsz) ? 0 : i + 1;
}

int FidoMsgWriteText(char *text, unsigned long n, unsigned long mlen)
{
    char i = 0;

    unused(n);
    unused(mlen);

    if (text == NULL)
    {
        farwrite(fd, &i, sizeof(char));
        return (TRUE);
    }
    farwrite(fd, text, strlen(text));
    return TRUE;
}

static char *path = NULL;


int FidoMsgWriteHeader(msg * m, int type)
{
    MFIDO msghead;
    time_t now = time(NULL);
    unsigned long n = m->msgnum;
    int done = 0;
    unsigned long x;

    if (path == NULL) path = xmalloc(PATHLEN);

    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
    sprintf(path, "%s/%lu.msg", CurArea.path, n);
    if (m->new)
    {
        while (!done)
        {
            fd = sopen(path, OPENR, SH_DENYNO, S_IMODE);
            if (fd != -1)
            {
                close(fd);
                fd = -1;
                n++;
            }
            else
            {
                fd = sopen(path, OPENC, SH_DENYNO, S_IMODE);
                if (fd == -1)
                {
                    return ERR_OPEN_MSG;
                }
                done = 1;
            }
            sprintf(path, "%s/%lu.msg", CurArea.path, n);
        }
    }
    else
    {
        fd = sopen(path, OPENRW, SH_DENYNO, S_IMODE);
        if (fd == -1)
        {
            return ERR_OPEN_MSG;
        }
    }

    if (m->new)
    {
        msgarrsz++;
        if (msgarrsz > oldsz)
        {
            unsigned long *t;
            t = xcalloc(1, (size_t) ((oldsz += CHUNKSZ) * sizeof *t));
            memcpy(t, msgarr, (size_t) (sizeof(unsigned long) * msgarrsz));
            release(msgarr);
            msgarr = t;
        }
        msgarr[(size_t) (msgarrsz - 1)] = n;
    }

    memset(&msghead, 0, sizeof msghead);

    m->msgnum = n;

    msghead.attrib[0] = (unsigned char)((m->attrib.killsent << 7) |
      (m->attrib.orphan << 6) | (m->attrib.forward << 5) |
      (m->attrib.attach << 4) | (m->attrib.sent << 3) | (m->attrib.rcvd << 2) |
      (m->attrib.crash << 1) | (m->attrib.priv));

    msghead.attrib[1] = (unsigned char)((m->attrib.ureq << 7) |
      (m->attrib.areq << 6) | (m->attrib.rcpt << 5) | (m->attrib.rreq << 4) |
      (m->attrib.freq << 3) | (m->attrib.direct << 2) | (m->attrib.hold << 1) |
      (m->attrib.local));

    x = FidoMsgnToUid((unsigned long)m->replyto);
    msghead.reply[0] = (unsigned char)(x & 0xff);
    msghead.reply[1] = (unsigned char)((x >> 8) & 0xff);
    x = FidoMsgnToUid((unsigned long)m->replies[0]);
    msghead.up[0] = (unsigned char)(x & 0xff);
    msghead.up[1] = (unsigned char)((x >> 8) & 0xff);
    msghead.times[0] = (unsigned char)(m->times_read & 0xff);
    msghead.times[1] = (unsigned char)((m->times_read >> 8) & 0xff);
    msghead.cost[0] = (unsigned char)(m->cost & 0xff);
    msghead.cost[1] = (unsigned char)((m->cost >> 8) & 0xff);

    msghead.dest_net[0] = (unsigned char)(m->to.net & 0xff);
    msghead.dest_net[1] = (unsigned char)((m->to.net >> 8) & 0xff);
    msghead.dest[0] = (unsigned char)(m->to.node & 0xff);
    msghead.dest[1] = (unsigned char)((m->to.node >> 8) & 0xff);

    if (m->isfrom != NULL)
    {
        size_t len_from;
        len_from = strlen(m->isfrom);
        memcpy(msghead.from, m->isfrom, min(sizeof msghead.from, len_from));
        msghead.from[sizeof(msghead.from) - 1] = '\0';
    }
    else
    {
        msghead.from[0] = '\0';
    }

    if (m->isto != NULL)
    {
        size_t len_to;
        len_to = strlen(m->isto);
        memcpy(msghead.to, m->isto, min(sizeof msghead.to, len_to));
        msghead.to[sizeof(msghead.to) - 1] = '\0';
    }
    else
    {
        msghead.to[0] = '\0';
    }

    if (m->subj != NULL)
    {
        size_t len_subj;
        len_subj = strlen(m->subj);
        memcpy(msghead.subj, m->subj, min(sizeof msghead.subj, len_subj));
        msghead.subj[sizeof(msghead.subj) - 1] = '\0';
    }
    else
    {
        msghead.subj[0] = '\0';
    }

    memcpy(msghead.date, mtime(m->timestamp), 20);

    msghead.orig_net[0] = (unsigned char)(m->from.net & 0xff);
    msghead.orig_net[1] = (unsigned char)((m->from.net >> 8) & 0xff);
    msghead.orig[0] = (unsigned char)(m->from.node & 0xff);
    msghead.orig[1] = (unsigned char)((m->from.node >> 8) & 0xff);

    if (SW->opusdate)
    {
        timet_to_char(now, msghead.written);
        memcpy(msghead.arrived, msghead.written, sizeof msghead.arrived);
    }
    else
    {
        timet_to_char(m->timestamp, msghead.written);
        if (m->time_arvd)
        {
            timet_to_char(m->time_arvd, msghead.arrived);
        } else  /* don't output bogus ... */
        {
            memcpy(msghead.arrived, msghead.written, sizeof msghead.arrived);
        }
    }

    assert(sizeof(MFIDO) == MFIDO_SIZE);
    farwrite(fd, (char *)&msghead, sizeof(MFIDO));

    if (type == WR_HEADER)
    {
        close(fd);
        fd = -1;
    }

    return TRUE;
}

msg *FidoMsgReadHeader(unsigned long n, int type)
{
    MFIDO msghead;
    unsigned long msgn;
    msg *m;

    if (path == NULL) path = xmalloc(PATHLEN);


    if (n > 0 && n <= msgarrsz)
    {
        msgn = msgarr[(size_t) (n - 1)];
    }
    else
    {
        return NULL;
    }

    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
    memset(&msghead, 0, sizeof msghead);
    sprintf(path, "%s/%lu.msg", CurArea.path, msgn);
    fd = sopen(path, OPENR, SH_DENYNO, S_IMODE);
    if (fd == -1)
    {
        return NULL;
    }

    m = xcalloc(1, sizeof *m);

    assert(sizeof(MFIDO) == MFIDO_SIZE);
    farread(fd, (char *)&msghead, (int)sizeof(MFIDO));

    m->msgnum = msgn;

    m->from.net = (msghead.orig_net[1] << 8) | msghead.orig_net[0];
    m->from.node = (msghead.orig[1] << 8) | msghead.orig[0];
    m->to.net = (msghead.dest_net[1] << 8) | msghead.dest_net[0];
    m->to.node = (msghead.dest[1] << 8) | msghead.dest[0];

    memset(path, 0, sizeof path);
    memcpy(path, msghead.date, sizeof msghead.date);

    m->timestamp = parsedate(path);

    if (msghead.arrived[0] != 0 || msghead.arrived[1] != 0 || msghead.arrived[2] != 0 || msghead.arrived[3] != 0)
    {
        m->time_arvd = char_to_timet(msghead.arrived);
    }

    m->isto = xcalloc(1, sizeof msghead.to + 1);
    m->isfrom = xcalloc(1, sizeof msghead.from + 1);
    m->subj = xcalloc(1, sizeof msghead.subj + 1);

    memcpy(m->isto, msghead.to, sizeof msghead.to);
    memcpy(m->isfrom, msghead.from, sizeof msghead.from);
    memcpy(m->subj, msghead.subj, sizeof msghead.subj);

    if (type != RD_HEADER_BRIEF)
    {
        m->replyto = FidoUidToMsgn((unsigned long)((msghead.reply[1] << 8)
                                                   | msghead.reply[0]));
        m->replies[0] = FidoUidToMsgn((unsigned long)((msghead.up[1] << 8)
                                                      | msghead.up[0]));
    }

    m->attrib.ureq = ((msghead.attrib[1] >> 7) & 0x01);
    m->attrib.areq = ((msghead.attrib[1] >> 6) & 0x01);
    m->attrib.rcpt = ((msghead.attrib[1] >> 5) & 0x01);
    m->attrib.rreq = ((msghead.attrib[1] >> 4) & 0x01);
    m->attrib.freq = ((msghead.attrib[1] >> 3) & 0x01);
    m->attrib.direct = ((msghead.attrib[1] >> 2) & 0x01);
    m->attrib.hold = ((msghead.attrib[1] >> 1) & 0x01);
    m->attrib.local = ((msghead.attrib[1]) & 0x01);

    m->attrib.killsent = ((msghead.attrib[0] >> 7) & 0x01);
    m->attrib.orphan = ((msghead.attrib[0] >> 6) & 0x01);
    m->attrib.forward = ((msghead.attrib[0] >> 5) & 0x01);
    m->attrib.attach = ((msghead.attrib[0] >> 4) & 0x01);
    m->attrib.sent = ((msghead.attrib[0] >> 3) & 0x01);
    m->attrib.rcvd = ((msghead.attrib[0] >> 2) & 0x01);
    m->attrib.crash = ((msghead.attrib[0] >> 1) & 0x01);
    m->attrib.priv = ((msghead.attrib[0]) & 0x01);

    m->from.zone = CurArea.addr.zone;
    m->to.zone = CurArea.addr.zone;
    m->cost = (msghead.cost[1] << 8) | msghead.cost[0];
    m->times_read = (msghead.times[1] << 8) | msghead.times[0];

    m->to.fidonet = 1;
    m->from.fidonet = 1;
    m->text = NULL;

    if (type == RD_HEADER || type == RD_HEADER_BRIEF)
    {
        close(fd);
        fd = -1;
    }

    return m;
}

char *FidoMsgReadText(unsigned long n)
{
    static char *next = NULL;
    static char *end = NULL;
    static unsigned long s = 0;
    int i, l;
    char *t;
    char *text;
    char eol = '\0';

    unused(n);
    if (next == NULL && s != 0)
    {
        s = 0;
        return NULL;
    }

    if (s == 0)
    {
        s = BUFLEN;
        memset(msgbuf, 0, (size_t) (s - 1));
        lseek(fd, (long)sizeof(MFIDO), SEEK_SET);
    }

    if (next == NULL)
    {
        i = farread(fd, msgbuf, (size_t) (s - 1));
        if (i < 1)
        {
            next = NULL;
            s = 0;
            return (NULL);
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
        memcpy(msgbuf, next, l + 1);
        i = farread(fd, msgbuf + l, (size_t) (s - l - 1));
        if (i < 1)
        {
            next = NULL;
            return xstrdup(msgbuf);
        }
        *(msgbuf + l + i) = '\0';
        normalize(msgbuf + l);
        end = msgbuf + strlen(msgbuf);
        next = msgbuf;
        t = memchr(next, '\n', l + i);
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

int FidoMsgClose(void)
{
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
    return TRUE;
}

int FidoMsgAreaClose(void)
{
    CurArea.status = 0;
    return TRUE;
}

static int compare(const void *i, const void *j)
{
    return ((int)(*(unsigned long *)i - *(unsigned long *)j));
}

void ScanArea(char *path)
{
    struct _dta fileinfo;
    int status;
    unsigned long msgnum;
    unsigned long *t;

    status = dir_findfirst(path, DIR_NORMAL | DIR_ICASE, &fileinfo);
    msgarrsz = 0;

    while (status != -1)
    {
        if (fileinfo.size >= sizeof(MFIDO))
        {
            msgnum = atol(fileinfo.name);
            msgarrsz++;
            if (msgarrsz >= oldsz)
            {
                t = xcalloc((size_t) (oldsz += CHUNKSZ), sizeof *t);
                if (msgarr != NULL)
                {
                    /* copy old array across */
                    memcpy(t, msgarr, (size_t) (sizeof(unsigned long) * msgarrsz));
                }
                release(msgarr);
                msgarr = t;
            }
            /* assign new msgnumber */
            msgarr[(size_t) (msgarrsz - 1)] = msgnum;
        }
        status = dir_findnext(&fileinfo);
    }

    if (msgarr == NULL)
    {
        /* no files at all in dir */
        msgarr = malloc(sizeof(unsigned long) * CHUNKSZ);
    }
    else
    {
        qsort((void *)msgarr, (int)msgarrsz, (size_t) sizeof(unsigned long), compare);
    }
}

long FidoMsgAreaOpen(AREA * a)
{
    short int c = 10, l;
    unsigned long msgnum;
    unsigned char shortbuf[2];

    if (path == NULL) path = xmalloc(PATHLEN);

    sprintf(path, "%s/*.msg", a->path);
    a->last = a->first = 1;
    a->current = a->lastread = 1;
    a->status = 1;

    ScanArea(path);
    a->scanned = 1;

    sprintf(path, "%s/%s", a->path, ST->lastread);
    fd = sopen(path, OPENR, SH_DENYNO, S_IMODE);
    if (fd != -1)
    {
        farread(fd, (char *) shortbuf, sizeof shortbuf);
        c = get_word(shortbuf);
        if (farread(fd, (char *) shortbuf, sizeof shortbuf) != sizeof shortbuf)
        {
            l = c;
        }
        else
        {
            l = get_word(shortbuf);
        }
        close(fd);
        fd = -1;
    }
    else
    {
        l = 0;
        c = 0;
    }

    if (msgarrsz != 0)
    {
        a->last = msgarrsz;

        msgnum = msgarrsz;
        while (msgnum > 1 && msgarr[(size_t) (msgnum - 1)] > (unsigned long)c)
        {
             msgnum--;
        }

        a->current = (!msgnum) ? a->last : msgnum;

        msgnum = msgarrsz;
        while (msgnum > 1 && msgarr[(size_t) (msgnum - 1)] != (unsigned long)l)
        {
            msgnum--;
        }

        a->lastread = (!msgnum) ? a->last : msgnum;
    }
    return (long)msgarrsz;
}

int FidoAreaSetLast(AREA * a)
{
    int fd;
    short i = 0;
    unsigned char shortbuf[2];

    if (path == NULL) path = xmalloc(PATHLEN);

    sprintf(path, "%s/%s", a->path, ST->lastread);
    fd = sopen(path, OPENRW, SH_DENYNO, S_IMODE);
    if (fd == -1)
    {
        if (fd == -1 && errno != EACCES && errno != EMFILE)
        {
            fd = sopen(path, OPENC, SH_DENYNO, S_IMODE_LASTREAD);
            if (fd == -1)
            {
                return FALSE;
            }
            lseek(fd, 0L, SEEK_SET);
            farwrite(fd, (byte far *)&i, sizeof(short));
            farwrite(fd, (byte far *)&i, sizeof(short));
            close(fd);
            return TRUE;
        }
        return FALSE;
    }

    lseek(fd, 0L, SEEK_SET);
    if (msgarr && a->lastread > 0 && a->current > 0 &&
        a->lastread <=msgarrsz && a->current <= msgarrsz)
    {
        i = (short)msgarr[(size_t) (a->current - 1)];
        shortbuf[0] = i & 0xFF; shortbuf[1] = (i >> 8) & 0xFF;
        farwrite(fd, (char *) shortbuf, sizeof(shortbuf));
        i = (short)msgarr[(size_t) (a->lastread - 1)];
        shortbuf[0] = i & 0xFF; shortbuf[1] = (i >> 8) & 0xFF;
        farwrite(fd, (char *) shortbuf, sizeof(shortbuf));
    }
    else
    {
        farwrite(fd, (byte far *)&i, sizeof(short));
        farwrite(fd, (byte far *)&i, sizeof(short));
    }
    close(fd);
    return TRUE;
}

int FidoMsgDelete(unsigned long n)
{
    if (path == NULL) path = xmalloc(PATHLEN);

    /* delete the message */

    sprintf(path, "%s/%lu.msg", CurArea.path, n);
    remove(path);

    /* we now re-scan the area just for the sake of it */

    sprintf(path, "%s/*.msg", CurArea.path);
    ScanArea(path);

    return TRUE;
}

/*
 *  This is what the "dos date" looks like, but we don't rely on the
 *  internal structure of the compiler to do our routines.
 */

#if 0
typedef struct _dosdate
{
    unsigned int day  : 5;
    unsigned int mon  : 4;
    unsigned int year : 7;
    unsigned int sec  : 5;
    unsigned int min  : 6;
    unsigned int hour : 5;
}
DOSDATE;
#endif

static void timet_to_char(time_t now, unsigned char arr[])
{
    struct tm *ts;
    unsigned long x;
    ts = localtime(&now);
    x = ts->tm_year - 80;
    x = (x << 4) | (ts->tm_mon + 1);
    x = (x << 5) | ts->tm_mday;
    x = (x << 5) | ts->tm_hour;
    x = (x << 6) | ts->tm_min;
    x = (x << 5) | (ts->tm_sec / 2);
    arr[0] = (unsigned char)((x >> 16) & 0xff);
    arr[1] = (unsigned char)((x >> 24) & 0xff);
    arr[2] = (unsigned char)(x & 0xff);
    arr[3] = (unsigned char)((x >> 8) & 0xff);
}

static time_t char_to_timet(unsigned char arr[])
{
    struct tm tms;
    time_t tt;
    unsigned long x;

                        /* the many unsigned long casts are needed  */
                        /* to make it work in a 16 bit environment */
    x = ( ((unsigned long) arr[0]) << 16) |
        ( ((unsigned long) arr[1]) << 24) |
           (unsigned long) arr[2]         |
        ( ((unsigned long) arr[3]) << 8);

    tms.tm_sec = (int) ((x & 0x1f) * 2);
    x >>= 5;
    tms.tm_min = (int) (x & 0x3f);
    x >>= 6;
    tms.tm_hour = (int) (x & 0x1f);
    x >>= 5;
    tms.tm_mday = (int) (x & 0x1f);
    x >>= 5;
    tms.tm_mon = (int) ((x & 0x0f) - 1);
    x >>= 4;
    tms.tm_year = (int) ((x & 0x7f) + 80);
    tms.tm_isdst = -1;
    tt = mktime(&tms);
    return tt;
}

int FidoMsgLock(void)
{
    return 0;
}

int FidoMsgUnlock(void)
{
    return 0;
}
