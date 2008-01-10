/*
 *  QUICK.C
 *
 *  Written on 30-Jul-90 by jim nutt and released to the public domain.
 *
 *  Support for QuickBBS-style message bases.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#ifndef PACIFIC
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <stat.h>
#endif
#include <assert.h>
#include <huskylib/compiler.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "unused.h"
#include "memextra.h"
#include "date.h"
#include "normal.h"
#include "quick.h"
#include "charset.h"


typedef unsigned int bits;

#define CHUNKSZ 32

#define QLASTN 200
short qlast[QLASTN];

static char msgtxt[FILENAME_MAX];
static char msghdr[FILENAME_MAX];

				/* I/O macros for platform independent
				   binary I/O. copied from smapi, as
				   msged with quick can be compiled
				   without/smapi (!) */

static void put_word(unsigned char *ptr, unsigned short value)
{
    ptr[0] = (value & 0xFF);
    ptr[1] = (value >> 8) & 0xFF;
}

#define get_word(ptr)         \
    ((unsigned short)((unsigned char)(ptr)[0]) |         \
     (((unsigned short)((unsigned char)(ptr)[1])) << 8 ))


int read_qlast(FILE *f, short *p)
{
    unsigned char buf[QLASTN * 2], *pbuf = buf;
    int i;

    if (fread(buf, 2*QLASTN, 1, f) != 1)
    {
	return 0;
    }

    for (i=0; i < QLASTN; i++)
    {
	p[i] = get_word(pbuf);
	pbuf += 2;
    }
    return 1;

}

int write_qlast(FILE *f, short *p)
{
   unsigned char buf[QLASTN * 2], *pbuf = buf;
   int i;

   for (i=0; i < QLASTN; i++)
   {
       put_word(pbuf, p[i]); pbuf+=2;
   }

   assert(pbuf - buf == QLASTN * 2);

   return (fwrite(buf, 2*QLASTN, 1, f) == 2*QLASTN);
}



struct qinfo
{
    short low;
    short high;
    short active;
    short areas[200];
};
#define QINFO_SIZE (6+200*2)

int read_qinfo(FILE *f, struct qinfo *qinfo)
{
    unsigned char buf[QINFO_SIZE], *pbuf = buf;
    int i;

    if (fread(buf, QINFO_SIZE, 1, f) != 1)
    {
        return 0;
    }

    qinfo->low = get_word(pbuf); pbuf+=2;
    qinfo->high = get_word(pbuf); pbuf+=2;
    qinfo->active = get_word(pbuf); pbuf+=2;
    for (i=0; i < 200; i++)
    {
	qinfo->areas[i] = get_word(pbuf); pbuf+=2;
    }
    assert (pbuf - buf == QINFO_SIZE);
    return 1;
}            

int write_qinfo(FILE *f, struct qinfo *qinfo)
{
    unsigned char buf[QINFO_SIZE], *pbuf = buf;
    int i;

    put_word(pbuf, qinfo->low); pbuf+=2;
    put_word(pbuf, qinfo->high); pbuf+=2;
    put_word(pbuf, qinfo->active); pbuf+=2;
    for (i=0; i < 200; i++)
    {
	put_word(pbuf, qinfo->areas[i]); pbuf+=2;
    }
    assert (pbuf - buf == QINFO_SIZE);

    if (fwrite(buf, QINFO_SIZE, 1, f) != 1)
    {
        return 0;
    }

    return 1;
}            

struct qidx
{
    short number;
    char board;
};
#define QIDX_SIZE 3

int read_qidx(FILE *f, struct qidx *qidx)
{
    unsigned char buf[QIDX_SIZE], *pbuf = buf;

    if (fread(buf, QIDX_SIZE, 1, f) != 1)
    {
        return 0;
    }

    qidx->number = get_word(pbuf); pbuf+=2;
    qidx->board = *pbuf; pbuf++;

    assert (pbuf - buf == QIDX_SIZE);
    return 1;
}            

int write_qidx(FILE *f, struct qidx *qidx)
{
    unsigned char buf[QIDX_SIZE], *pbuf = buf;
 
    put_word(pbuf, qidx->number); pbuf+=2;
    *pbuf = qidx->board; pbuf++;

    assert (pbuf - buf == QIDX_SIZE);
 
    if (fwrite(buf, QIDX_SIZE, 1, f) != 1)
    {
        return 0;
    }
    return 1;
}            
  

struct qmsg
{
    short number;
    short replyto;
    short replyfrom;
    short times_read;		/* 8 bytes */
    unsigned short start;
    unsigned short count;
    short destnet;
    short destnode;		/* 16 bytes */
    short orignet;
    short orignode;
    char destzone;
    char origzone;
    short cost;			/* 24 bytes */

    /* message attributes */
    bits deleted  : 1;
    bits outnet   : 1;
    bits netmail  : 1;
    bits priv     : 1;
    bits rcvd     : 1;
    bits echo     : 1;
    bits local    : 1;
    bits xx1      : 1;
    bits killsent : 1;
    bits sent     : 1;
    bits attach   : 1;
    bits crash    : 1;
    bits rreq     : 1;
    bits areq     : 1;
    bits rrcpt    : 1;
    bits xx2      : 1;		/* 26 bytes */

    char board;			/* 27 bytes */
    char posttime[6];		/* 33 bytes */
    char postdate[9];		/* 42 bytes */
    char whoto[36];		/* 78 bytes */
    char whofrom[36];		/* 114 bytes */
    char subject[73];		/* 187 bytes */
};
#define QMSG_SIZE 187

static int read_qmsg(FILE *f, struct qmsg *qmsg)
{
    unsigned char buf[QMSG_SIZE], *pbuf = buf;
    unsigned short attr;

    if (fread(buf, QMSG_SIZE, 1, f) != 1)
    {
        return 0;
    }

    qmsg->number=get_word(pbuf); pbuf+=2;
    qmsg->replyto=get_word(pbuf); pbuf+=2;
    qmsg->replyfrom=get_word(pbuf); pbuf+=2;
    qmsg->times_read=get_word(pbuf); pbuf+=2;
    qmsg->start=get_word(pbuf); pbuf+=2;
    qmsg->count=get_word(pbuf); pbuf+=2;
    qmsg->destnet=get_word(pbuf); pbuf+=2;
    qmsg->destnode=get_word(pbuf); pbuf+=2;
    qmsg->orignet=get_word(pbuf); pbuf+=2;
    qmsg->orignode=get_word(pbuf); pbuf+=2;
    qmsg->destzone = *pbuf; pbuf++;
    qmsg->origzone = *pbuf; pbuf++;
    qmsg->cost=get_word(pbuf); pbuf+=2;

    attr = get_word(pbuf); pbuf+=2;
    qmsg->deleted = attr & 1; attr = attr >> 1;
    qmsg->outnet = attr & 1; attr = attr >> 1;
    qmsg->netmail = attr & 1; attr = attr >> 1;
    qmsg->priv = attr & 1; attr = attr >> 1;
    qmsg->rcvd = attr & 1; attr = attr >> 1;
    qmsg->echo = attr & 1; attr = attr >> 1;
    qmsg->local = attr & 1; attr = attr >> 1;
    qmsg->xx1 = attr & 1; attr = attr >> 1;
    qmsg->killsent = attr & 1; attr = attr >> 1;
    qmsg->sent = attr &  1; attr = attr >> 1;
    qmsg->attach = attr & 1; attr = attr >> 1;
    qmsg->crash = attr &  1; attr = attr >> 1;
    qmsg->rreq = attr & 1; attr = attr >> 1;
    qmsg->areq = attr &  1; attr = attr >> 1;
    qmsg->rrcpt = attr &  1; attr = attr >> 1;
    qmsg->xx2 = attr &  1; attr = attr >> 1;

    qmsg->board = *pbuf; pbuf++;
    memcpy(qmsg->posttime, pbuf, 6); pbuf+=6;
    memcpy(qmsg->postdate, pbuf, 9); pbuf+=9;
    memcpy(qmsg->whoto, pbuf, 36); pbuf+=36;
    memcpy(qmsg->whofrom, pbuf, 36); pbuf+=36;
    memcpy(qmsg->subject, pbuf, 73); pbuf+=73;
    assert (pbuf - buf == QMSG_SIZE);
    return 1;
}      

static int write_qmsg(FILE *f, struct qmsg *qmsg)
{
    unsigned char buf[QMSG_SIZE], *pbuf = buf;
    unsigned short attr;

    
    put_word(pbuf,qmsg->number); pbuf+=2;
    put_word(pbuf,qmsg->replyto); pbuf+=2;
    put_word(pbuf,qmsg->replyfrom); pbuf+=2;
    put_word(pbuf,qmsg->times_read); pbuf+=2;
    put_word(pbuf,qmsg->start); pbuf+=2;
    put_word(pbuf,qmsg->count); pbuf+=2;
    put_word(pbuf,qmsg->destnet); pbuf+=2;
    put_word(pbuf,qmsg->destnode); pbuf+=2;
    put_word(pbuf,qmsg->orignet); pbuf+=2;
    put_word(pbuf,qmsg->orignode); pbuf+=2;
    *pbuf = qmsg->destzone; pbuf++;
    *pbuf = qmsg->origzone; pbuf++;
    put_word(pbuf,qmsg->cost); pbuf+=2;

    attr =
	(qmsg->deleted ? 1 : 0) + 
	(qmsg->outnet  ? 2 : 0) +
	(qmsg->netmail ? 4 : 0) +
	(qmsg->priv ? 8  : 0 )+
	(qmsg->rcvd ? 16 : 0 ) + 
	(qmsg->echo ? 32 : 0 ) +
	(qmsg->local ? 64 : 0 ) +
	(qmsg->xx1 ? 128 : 0 ) +
	(qmsg->killsent ? 256 : 0 ) +
	(qmsg->sent ? 512 : 0 ) +
	(qmsg->attach ? 1024 : 0 ) +
	(qmsg->crash ? 2048 : 0 ) +
	(qmsg->rreq ? 4096 : 0 ) +
	(qmsg->areq ? 8192 : 0 ) +
	(qmsg->rrcpt ? 16384 : 0 ) +
	(qmsg->xx2 ? 32768U : 0 );

    put_word(pbuf, attr); pbuf+=2;

    *pbuf = qmsg->board; pbuf++;
    memcpy(pbuf, qmsg->posttime, 6); pbuf+=6;
    memcpy(pbuf, qmsg->postdate, 9); pbuf+=9;
    memcpy(pbuf, qmsg->whoto, 36); pbuf+=36;
    memcpy(pbuf, qmsg->whofrom, 36); pbuf+=36;
    memcpy(pbuf, qmsg->subject, 73); pbuf+=73;

    assert (pbuf - buf == QMSG_SIZE);

    if (fwrite(buf, QMSG_SIZE, 1, f) != 1)
    {
        return 0;
    }

    return 1;
}      

struct qtext
{
    unsigned char length;
    char text[BLOCKLEN];
};
#define QTEXT_SIZE (BLOCKLEN+1)

static int read_qtext(FILE *f, struct qtext *qtext)
{
    unsigned char buf[QTEXT_SIZE], *pbuf = buf;

    if (fread(buf, QTEXT_SIZE, 1, f) != 1)
    {
        return 0;
    }

    qtext->length= *pbuf; pbuf++;
    memcpy(qtext->text, pbuf, BLOCKLEN); pbuf+=BLOCKLEN;

    assert (pbuf - buf == QTEXT_SIZE);
    return 1;
}            

static int write_qtext(FILE *f, struct qtext *qtext)
{
    unsigned char buf[QTEXT_SIZE], *pbuf = buf;

    *pbuf = qtext->length; pbuf++;
    memcpy(pbuf, qtext->text, BLOCKLEN); pbuf+=BLOCKLEN;

    assert (pbuf - buf == QTEXT_SIZE);

    if (fwrite(buf, QTEXT_SIZE, 1, f) != 1)
    {
        return 0;
    }
    return 1;
}            


static struct qinfo info;
static struct qmsg header;

static char path[80];
static long start = -1;
static long count = 0;
static int position = 0;
static FILE *infofp = NULL;
static FILE *idxfp = NULL;
static FILE *textfp = NULL;
static FILE *hdrfp = NULL;
static FILE *toidxfp = NULL;
static short *messages = NULL;
static int maxmsgs;

int QuickMsgDelete(unsigned long n)
{
    struct qidx index;

    header.deleted = 1;

    if (fseek(hdrfp, (long)(messages[(size_t) (n - 1)] *
      (long)QIDX_SIZE), SEEK_SET))
    {
        return FALSE;
    }

    write_qmsg(hdrfp, &header);
    fflush(hdrfp);

    fseek(idxfp, messages[(size_t) (n - 1)] * (long)QIDX_SIZE, SEEK_SET);
    index.board = (char)CurArea.board;
    index.number = -1;
    write_qidx(idxfp, &index);
    fflush(idxfp);

    start = count = 0;
    position = 0;

    info.areas[CurArea.board - 1]--;
    fseek(infofp, 0L, SEEK_SET);
    write_qinfo(infofp, &info);
    fflush(infofp);

    return TRUE;
}

int QuickMsgWriteText(char *text, unsigned long n, unsigned long mlen)
{
    static struct qtext block;
    char *s;
    static char buf[768];
    struct stat b;
    static int f = 0;

    unused(mlen);
    if (f == 0)
    {
        f = 1;
#if defined(PACIFIC) || defined(LATTICE)
        stat(msgtxt, &b);
#else
        fstat(fileno(textfp), &b);
#endif
        start = b.st_size / QTEXT_SIZE;
        count = 0;
    }

    if (text == NULL)
    {
        n = position;
        memset(block.text, 0, sizeof block.text);
        strcpy(block.text, buf);
        block.length = (char)strlen(buf);
        fseek(textfp, (long)(start + count) * (long)QTEXT_SIZE, SEEK_SET);
        write_qtext(textfp, &block);
        fflush(textfp);
        header.start = (unsigned short)start;
        header.count = (unsigned short)++count;
        fseek(hdrfp, (long)n * (long)QMSG_SIZE, SEEK_SET);
        write_qmsg(hdrfp, &header);
        fflush(hdrfp);
        f = 0;
        memset(buf, 0, sizeof buf);
        return TRUE;
    }

    strcat(buf, text);
    while (strlen(buf) > sizeof block.text)
    {
        s = buf + sizeof block.text;
        memcpy(block.text, buf, sizeof block.text);
        strcpy(buf, s);
        block.length = sizeof block.text;
        fseek(textfp, (long)(start + count) * (long)QTEXT_SIZE, SEEK_SET);
        write_qtext(textfp, &block);
        fflush(textfp);
        count++;
    }

    return TRUE;
}

short quick2msg(short number)
{
    struct qidx index;
    int i = 0;

    if (number == 0)
    {
        return 0;
    }

    for (i = 0; i < CurArea.messages; i++)
    {
        if (fseek(idxfp, messages[(size_t) i] * (long) QIDX_SIZE, SEEK_SET))
        {
            return 0;
        }
        if (read_qidx(idxfp, &index) != 1)
        {
            return 0;
        }
        if (index.number == number)
        {
            return (short) (i + 1);
        }
    }
    return 0;
}

short msg2quick(short n)
{
    struct qidx index;

    if (n > (CurArea.messages + 1) || n == 0)
    {
        return 0;
    }

    if (fseek(idxfp, messages[(size_t) (n - 1)] * (long)QIDX_SIZE, SEEK_SET))
    {
        return 0;
    }

    if (read_qidx(idxfp, &index) != 1)
    {
        return 0;
    }

    return index.number;
}

short find_link(unsigned long n)
{
    struct qmsg header;
    struct stat b;
    long i;

    if (messages == NULL)
    {
        return 0;
    }

#if defined(PACIFIC) || defined(LATTICE)
    stat(msghdr, &b);
#else
    fstat(fileno(hdrfp), &b);
#endif

    i = (long)(messages[(size_t) (n - 1)]) * (long)QMSG_SIZE;
    if (i > b.st_size)
    {
        return 0;
    }

    fseek(hdrfp, i, SEEK_SET);
    if (read_qmsg(hdrfp, &header) != 1)
    {
        return 0;
    }

#if defined(PACIFIC) || defined(LATTICE) 
    fclose(textfp);
    textfp = fopen(msgtxt, "r+b");
    stat(msgtxt, &b);
#else
    fstat(fileno(textfp), &b);
#endif

    if ((((long)header.start + (long)header.count) *
      (long)QTEXT_SIZE) > b.st_size)
    {
        return 0;
    }

    if (header.deleted)
    {
        return 0;
    }

    return quick2msg(header.replyfrom);
}

msg *QuickMsgReadHeader(unsigned long n, int type)
{
    struct stat b;
    char path[80];
    msg *m;
    long i;

    unused(type);
    if (messages == NULL || n == 0)
    {
        return NULL;
    }
    memset(path, 0, sizeof path);

#if defined(PACIFIC) || defined(LATTICE)
    stat(msghdr, &b);
#else
    fstat(fileno(hdrfp), &b);
#endif

    i = (long)(messages[(size_t) (n - 1)]) * (long) QMSG_SIZE;
    if (i > b.st_size)
    {
        return NULL;
    }

    position = messages[(size_t) (n - 1)];

    fseek(hdrfp, i, SEEK_SET);
    if (read_qmsg(hdrfp, &header) != 1)
    {
        return NULL;
    }

    start = (long)header.start;
    count = (long)header.count;

#ifdef PACIFIC
    fclose(textfp);
    textfp = fopen(msgtxt, "r+b");
    stat(msgtxt, &b);
#else
    fstat(fileno(textfp), &b);
#endif
    if (((start + count) * (long)QTEXT_SIZE) > b.st_size)
    {
        return NULL;
    }

    if (header.deleted)
    {
        return NULL;
    }

    m = xcalloc(1, sizeof *m);
    m->msgnum = quick2msg(header.number);

    m->isfrom = xcalloc(header.whofrom[0] + 1, 1);
    strncpy(m->isfrom, header.whofrom + 1, header.whofrom[0]);
    m->isto = xcalloc(header.whoto[0] + 1, 1);
    strncpy(m->isto, header.whoto + 1, header.whoto[0]);
    m->subj = xcalloc(header.subject[0] + 1, 1);
    strncpy(m->subj, header.subject + 1, header.subject[0]);

    strncpy(path, header.postdate + 1, header.postdate[0]);
    strcat(path, " ");
    strncat(path, header.posttime + 1, header.posttime[0]);
    m->timestamp = parsedate(path);

    if (!(type & RD_HEADER_BRIEF))
    {
        if (header.replyto)
        {
            m->replyto = quick2msg(header.replyto);
        }
        if (header.replyfrom)
        {
            m->replies[0] = quick2msg(header.replyfrom);
        }
        if (type & RD_ALL)
        {
            header.times_read++;
            fseek(hdrfp, (long)position * (long)QMSG_SIZE, SEEK_SET);
            write_qmsg(hdrfp, &header);
        }
    }

    m->attrib.priv = header.priv;
    m->attrib.crash = header.crash;
    m->attrib.rcvd = header.rcvd;
    m->attrib.sent = header.sent;
    m->attrib.attach = header.attach;
    m->attrib.forward = 0;
    m->attrib.orphan = 0;
    m->attrib.killsent = header.killsent;
    m->attrib.local = header.local;
    m->attrib.hold = header.xx1;
    m->attrib.direct = header.xx2;
    m->attrib.freq = 0;
    m->attrib.rreq = header.rreq;
    m->attrib.rcpt = header.rrcpt;
    m->attrib.areq = header.areq;
    m->attrib.ureq = 0;

    m->to.zone = header.destzone;
    m->to.net = header.destnet;
    m->to.node = header.destnode;
    m->times_read = header.times_read;

    m->from.zone = header.destzone;
    m->from.net = header.destnet;
    m->from.node = header.destnode;

    m->to.fidonet = m->from.fidonet = 1;

    return m;
}

int QuickMsgWriteHeader(msg * m, int type)
{
    struct qidx index;
    struct tm *ts;
    struct stat b;
    FILE *fp;
    size_t len_to, len_from, len_subj;
    int c = (int)(CurArea.current - 1);

    unused(type);
    memset(&header, 0, sizeof header);

    if (m->new)
    {
        c = (int)CurArea.messages;
#ifdef PACIFIC
        stat(msghdr, &b);
#else
        fstat(fileno(hdrfp), &b);
#endif
        if (c >= maxmsgs)
        {
            messages = xrealloc(messages, (maxmsgs += CHUNKSZ) * sizeof(short));
        }
        messages[c] = (short)(b.st_size / QMSG_SIZE);
        start = (unsigned short)(header.start = 0);
        count = (unsigned short)(header.count = 0);
        info.areas[CurArea.board - 1]++;
        info.active++;
        header.number = ++info.high;
    }
    else
    {
        if ((header.number = msg2quick((short)m->msgnum)) == 0)
        {
            return FALSE;
        }
        header.start = (unsigned short)start;
        header.count = (unsigned short)count;
    }

    position = messages[c];

    header.replyto = msg2quick((short)m->replyto);
    header.replyfrom = msg2quick((short)(m->replies[0]));
    header.times_read = (short)m->times_read;
    header.destzone = (char)m->to.zone;
    header.destnet = (short)m->to.net;
    header.destnode = (short)m->to.node;
    header.origzone = (char)m->from.zone;
    header.orignet = (short)m->from.net;
    header.orignode = (short)m->from.node;
    header.cost = (short)m->cost;

    header.deleted = 0;
    header.outnet = 0;
    header.netmail = 0;
    header.echo = 0;

    if (CurArea.netmail)
    {
        header.netmail = 1;
        header.outnet = 1;
    }

    if (CurArea.echomail)
    {
        header.echo = 1;
    }

    header.priv = m->attrib.priv;
    header.rcvd = m->attrib.rcvd;
    header.local = m->attrib.local;
    header.xx1 = m->attrib.hold;
    header.killsent = m->attrib.killsent;
    header.sent = m->attrib.sent;
    header.attach = m->attrib.attach;
    header.crash = m->attrib.crash;
    header.rreq = m->attrib.rreq;
    header.areq = m->attrib.areq;
    header.rrcpt = m->attrib.rcpt;
    header.xx2 = m->attrib.direct;
    header.board = (char)CurArea.board;

    ts = localtime(&m->timestamp);
    header.posttime[0] = 5;
    sprintf(header.posttime + 1, "%02d:%02d", ts->tm_hour, ts->tm_min);
    header.postdate[0] = 8;
    sprintf(header.postdate + 1, "%02d-%02d-%02d", ts->tm_mon + 1,
      ts->tm_mday, (ts->tm_year % 100));

    if (m->isto == NULL)
    {
        header.whoto[0] = 0;
    }
    else
    {
        len_to = strlen(m->isto);
        header.whoto[0] = (char)min(len_to, sizeof(header.whoto) - 1);
        memcpy(header.whoto + 1, m->isto, header.whoto[0]);
    }

    if (m->isfrom == NULL)
    {
        header.whofrom[0] = 0;
    }
    else
    {
        len_from = strlen(m->isfrom);
        header.whofrom[0] = (char)min(len_from, sizeof(header.whofrom));
        memcpy(header.whofrom + 1, m->isfrom, header.whofrom[0]);
    }

    if (m->subj == NULL)
    {
        header.subject[0] = 0;
    }
    else
    {
        len_subj = strlen(m->subj);
        header.subject[0] = (char)min(len_subj, sizeof(header.subject) - 1);
        memcpy(header.subject + 1, m->subj, header.subject[0]);
    }

    fseek(hdrfp, (long)position * (long)QMSG_SIZE, SEEK_SET);
    write_qmsg(hdrfp, &header);

    fseek(infofp, 0L, SEEK_SET);
    write_qinfo(infofp, &info);

    index.number = header.number;
    index.board = (char)CurArea.board;
    fseek(idxfp, (long)position * (long)QIDX_SIZE, SEEK_SET);
    write_qidx(idxfp, &index);

    fflush(idxfp);
    fflush(infofp);
    fflush(hdrfp);

    strcpy(path, ST->quickbbs);
    strcat(path, "/msgtoidx.bbs");
    fp = fopen(path, "r+b");
    if (fp == NULL)
    {
        fp = fopen(path, "w+b");
        if (fp == NULL)
        {
            return TRUE;
        }
    }

    fseek(fp, (long)position * (long)sizeof(header.whoto), SEEK_SET);
    fwrite(header.whoto, sizeof(header.whoto), 1, fp);
    fclose(fp);

    return TRUE;
}

char *QuickMsgReadText(unsigned long n)
{
    static struct qtext text;

    static long int b = -1;
    static long int c = -1;

    static char *next = NULL;
    char *t, *t2, ch = '\0';

    static char *s = NULL;

    if ((long)n < 0)
    {
        b = c = -1;
        next = NULL;
        if (s)
        {
            xfree(s);
        }
        s = NULL;
        return NULL;
    }

    if (next == NULL && s != NULL)
    {
        xfree(s);
        b = c = -1;
        s = NULL;
        return NULL;
    }

    if (s == NULL)
    {

        if (b == -1)
        {
            b = (long)(start = (long)header.start);
            c = (long)(count = (long)header.count);
        }

        if (c < 1 || b < 0)
        {
            b = c = -1;
            return NULL;
        }

        s = xmalloc((size_t) count * sizeof text + 1);

        memset(s, 0, (size_t) c * sizeof text + 1);

        fseek(textfp, (long)(start * (long)QTEXT_SIZE), SEEK_SET);
        while (c)
        {
            memset(&text, 0, sizeof text);
            if (read_qtext(textfp, &text) == 1)
            {
                if (text.length > sizeof text.text) /* Always false!!! */
                {
                    text.length = sizeof text.text;
                }
                strncat(s, text.text, text.length);
            }
            c--;
        }
        normalize(s);
        next = s;
    }

    t = next;
    next = strchr(t, '\n');
    if (next)
    {
        ch = *(next + 1);
        *(next + 1) = '\0';
    }

    t2 = xstrdup(t);

    if (next)
    {
        *(next + 1) = ch;
        next++;
    }

    return t2;
}

int QuickAreaSetLast(AREA * a)
{
    FILE *fp;

    strcpy(path, ST->quickbbs);
    strcat(path, "/lastread.bbs");

    if (messages == NULL || a->current == 0)
    {
        qlast[a->board - 1] = 0;
    }
    else
    {
        qlast[a->board - 1] = msg2quick((short) a->lastread);
    }

    fp = fopen(path, "wb");
    if (fp != NULL)
    {
        write_qlast(fp, qlast);
        fclose(fp);
    }

    return TRUE;
}

static int quick_scan(AREA * a)
{
    struct qidx index;
    FILE *fp;
    int i, idx;

    position = 0;

    if (infofp != NULL)
    {
        fclose(infofp);
    }

    strcpy(path, ST->quickbbs);
    strcat(path, "/msginfo.bbs");

    infofp = fopen(path, "r+b");
    if (infofp == NULL)
    {
        infofp = fopen(path, "w+b");
        if (infofp == NULL)
        {
            return 0;
        }
    }

    if (idxfp != NULL)
    {
        fclose(idxfp);
    }

    strcpy(path, ST->quickbbs);
    strcat(path, "/msgidx.bbs");
    idxfp = fopen(path, "r+b");
    if (idxfp == NULL)
    {
        idxfp = fopen(path, "w+b");
        if (idxfp == NULL)
        {
            return 0;
        }
    }

    if (textfp != NULL)
    {
        fclose(textfp);
    }

    strcpy(path, ST->quickbbs);
    strcat(path, "/msgtxt.bbs");
    strcpy(msgtxt, path);
    textfp = fopen(path, "r+b");
    if (textfp == NULL)
    {
        textfp = fopen(path, "w+b");
        if (textfp == NULL)
        {
            return 0;
        }
    }

    if (hdrfp != NULL)
    {
        fclose(hdrfp);
    }

    strcpy(path, ST->quickbbs);
    strcat(path, "/msghdr.bbs");
    strcpy(msghdr, path);
    hdrfp = fopen(path, "r+b");
    if (hdrfp == NULL)
    {
        hdrfp = fopen(path, "w+b");
        if (hdrfp == NULL)
        {
            return 0;
        }
    }

    if (toidxfp != NULL)
    {
        fclose(toidxfp);
    }

    strcpy(path, ST->quickbbs);
    strcat(path, "/msgtoidx.bbs");
    toidxfp = fopen(path, "r+b");
    if (toidxfp == NULL)
    {
        toidxfp = fopen(path, "w+b");
        if (toidxfp == NULL)
        {
            return 0;
        }
    }

    if (messages != NULL)
    {
        xfree(messages);
    }

    messages = NULL;

    rewind(infofp);
    if (read_qinfo(infofp, &info) != 1)
    {
        memset(&info, 0, sizeof info);
    }

    i = 0;
    maxmsgs = 0;
    idx = 0;
    rewind(idxfp);
    while (read_qidx(idxfp, &index) == 1)
    {
        if (index.board == (char)a->board && index.number > 0)
        {
            if (i >= maxmsgs)
            {
                messages = xrealloc(messages, (maxmsgs += CHUNKSZ) * sizeof(short));
            }
            messages[i++] = (short)idx;
        }
        idx++;
    }

    a->first = 1;
    a->last = i;
    a->messages = i;

    strcpy(path, ST->quickbbs);
    strcat(path, "/lastread.bbs");
    fp = fopen(path, "rb");
    if (fp != NULL)
    {
        read_qlast(fp, qlast);
        a->lastread = qlast[a->board - 1];
        fclose(fp);
    }

    a->current = quick2msg((short) a->lastread);
    a->lastread = a->current;

    if (a->lastread > i || i <= 0)
    {
        a->lastread = 0;
    }
    if (a->current > i || i <= 0)
    {
        a->current = 0;
    }

    info.areas[a->board - 1] = (short)i;

    return i;
}

int QuickMsgClose(void)
{
    return 0;
}

long QuickMsgAreaOpen(AREA * a)
{
    long ret;
    ret = quick_scan(a);
    a->status = 1;
    a->scanned = 1;
    return ret;
}

int QuickMsgAreaClose(void)
{
    CurArea.status = 0;
    return TRUE;
}

unsigned long QuickMsgnToUid(unsigned long n)
{
    return n;
}

unsigned long QuickUidToMsgn(unsigned long n)
{
    return n;
}

int QuickMsgLock(void)
{
    return 0;
}

int QuickMsgUnlock(void)
{
    return 0;
}
