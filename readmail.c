/*
 *  READMAIL.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Handles high-level message I/O.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#if defined(__MSC__) || defined(OS216)
#include <sys/types.h>
#include <sys/timeb.h>
#include <direct.h>
#endif

#if defined(MSDOS) && defined(__TURBOC__)
#include <dir.h>
#endif

#ifdef __WATCOMC__
#if defined(MSDOS) || defined(WINNT)
#include <direct.h>
#endif
#endif

#ifdef PACIFIC
#include <sys.h>
int bdos(int func, unsigned reg_dx, unsigned char reg_al);
#endif

#if defined(MSDOS) || (defined(WINNT) && !defined(__MINGW32__))
#include <dos.h>
#ifdef __TURBOC__
#include <dir.h>
#endif
#endif

#ifdef OS2
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#if defined(UNIX) || defined(__DJGPP__)
#include <unistd.h>
#endif

#ifdef __MINGW32__
#define chdir _chdir
#endif

#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "flags.h"
#include "memextra.h"
#include "strextra.h"
#include "config.h"
#include "quote.h"
#include "wrap.h"
#include "fecfg145.h"
#include "winsys.h"
#include "menu.h"
#include "dialogs.h"
#include "version.h"
#include "readmail.h"
#include "charset.h"
#include "date.h"
#include "echotoss.h"
#include "mctype.h"
#include "help.h"
#include "template.h"

#define rand_number(num) ((int) (((long) rand()) % (num)))

#define TEXTLEN 96

static void deleteCrapLine(LINE * crap);
static int is_sameaddr(ADDRESS * msg);

extern int set_rcvd;      /* located in msged.c */

int read_verbatim = 0;    /* see readmail.h for explanation */

#ifdef OS2
static unsigned long setDefaultDisk(unsigned short x);
#endif

static int changeDir(char *path);

char *do_softcrxlat(char *ptr)
{
    char *p;

    if (softcrxlat && ptr != NULL)
    {
        p = strchr(ptr, 0x8d);
        while (p != NULL)
        {
            *p++ = softcrxlat;
            p = strchr(p, 0x8d);
        }
    }

    return ptr;
}

LINE *clearbuffer(LINE * buffer)
{
    LINE *curline;

    curline = buffer;
    if (curline != NULL)
    {
	while (curline->next != NULL)
	{
	    curline = curline->next;
	    if (curline->prev == NULL)
	    {
		continue;
	    }
	    if (curline->prev->text != NULL)
	    {
		xfree(curline->prev->text);
	    }
	    curline->prev->next = NULL;
	    xfree(curline->prev);
	    curline->prev = NULL;
	}
	if (curline != NULL)
	{
	    if (curline->text)
	    {
		xfree(curline->text);
	    }
	    curline->text = NULL;
	    xfree(curline);
	}
    }
    return NULL;
}

static void KillTrailingLF(char *text)
{
    char *s;

    if (text == NULL)
    {
	return;
    }

    s = strchr(text, '\n');
    if (s != NULL)
    {
	*s = '\0';
    }
}

msg *readmsg(unsigned long n)
{
    ADDRESS a;
    LINE *l;
    char *tokens[10];
    int headerfin = 0, lastwasfromto = 0;
    int afound = 0;
    msg *m;
    char *text;
    char *t, *s;
    char *ptmp;
    char tmp[128];
    int goteot = 0;
    int gotsot = 0;
    int had_codepage = 0;
    LOOKUPTABLE *ltable = NULL;
    int fmpt, topt;
    int clevel = 0;

    l = NULL;

    memset(&a, 0, sizeof a);

    m = MsgReadHeader(n, RD_ALL);
    if (m == NULL)
    {
	read_verbatim = 0;
	return NULL;
    }

    stripSoft = 1;
    m->replyarea = NULL;
    if (ST->enforced_charset != NULL || ST->input_charset != NULL)
    {
        memset(tokens, 0, sizeof(tokens));
        if (ST->enforced_charset != NULL)
        {
            text = xstrdup(ST->enforced_charset);
        }
        else
        {
            text = xstrdup(ST->input_charset);
        }
        parse_tokens(text, tokens, 2);
        if (tokens[1])
        {
            m->charset_level = atoi(tokens[1]);
        }
        else
        {
            m->charset_level = 2;
        }
        m->charset_name = xstrdup(tokens[0]);
        ltable = get_readtable(m->charset_name, m->charset_level);
        free(text);
    }
    else
    {
	m->charset_name = NULL;
	m->charset_level = 0;
	ltable = get_readtable("ASCII", 2);
    }

    while ((text = MsgReadText(n)) != NULL)
    {
	if (*text == '\n' || *text == '\0' || !stricmp(text, "Lines:"))
	{
	    headerfin = 1;  /* want to stop looking unix header info */

	    if (lastwasfromto && (!SW->shownotes) && (!read_verbatim))
	    {
		 release(text); /* skip the blank line after from: / to: */
		 lastwasfromto = 0;
		 continue;
	    }
	}

	lastwasfromto = 0;

	if (*text == '\01')
	{
	    switch (*(text + 1))
	    {
	    case 'A':
		if (strncmp(text + 1, "AREA:", 5) != 0)
		{
		    break;
		}
		s = text + 6;

		while (m_isspace(*s))
		{
		    s++;
		}

		release(m->replyarea);
		m->replyarea = xstrdup(s);
		KillTrailingLF(m->replyarea);
		break;

	    case 'C':
                if (ST->enforced_charset != NULL)
                {
                    /* CHRS kludge is ignored if a special input charset
                       is enforced */
                    break;
                }

		if (strncmp(text + 1, "CHRS:", 5) == 0)
		{
                    if (had_codepage)
                        break;  /* @CODEPAGE is definitive override */
                    s = text + 6;
                    strcpy(tmp, s + 1);
                }
                else if (strncmp(text + 1, "CHARSET:", 8) == 0)
                {
                    if (had_codepage)
                        break;  /* @CODEPAGE is definitive override */
                    s = text + 9;
                    strcpy(tmp, s + 1);
                }
                else if (strncmp(text + 1, "CODEPAGE:", 9) == 0)
                {
                    had_codepage = 1;
                    s = text + 10;
                    sprintf(tmp, "CP%s 2", s+1);
                }
                else
                {
                    break;
                }

		memset(tokens, 0, sizeof(tokens));
		parse_tokens(tmp, tokens, 2);
		if (!tokens[1] || !tokens[1][0])
		{
                    clevel = 2;
                       /* Assume level 2 as default. The CHARSET kludge has no
                          level at all, and some broken message readers omit
                          the level also for the CHRS kludge. */
		}
                else
                {
                    clevel = atoi(tokens[1]);
                }

                if ( have_readtable(tokens[0], clevel) ||
                     ST->input_charset == NULL /* user wants no assumptions */)
                {
                    release(m->charset_name);
                    m->charset_name  = xstrdup(tokens[0]);
                    m->charset_level = clevel;
                    ltable = get_readtable(m->charset_name,
                                           m->charset_level);
                }
		break;

	    case 'M':
		if (strncmp(text + 1, "MSGID:", 6) != 0)
		{
		    break;
		}
		s = text + 7;

		while (m_isspace(*s))
		{
		    s++;
		}

		release(m->msgid);
		m->msgid = xstrdup(s);
		KillTrailingLF(m->msgid);
		break;

	    case 'R':
		if (strncmp(text + 1, "REPLY:", 6) != 0)
		{
		    break;
		}
		s = text + 7;

		while (m_isspace(*s))
		{
		    s++;
		}

		if (arealist[SW->area].msgtype != QUICK)
		{
		    release(m->reply);
		    m->reply = xstrdup(s);
		    KillTrailingLF(m->reply);
		}
		break;

	    case 'E':
		if (strncmp(text + 1, "EOT:", 4) == 0)
		{
		    goteot = 1;
		    if (gotsot && !(SW->showseenbys ||
				    SW->shownotes   || read_verbatim))
		    {
			m->soteot = 1;
		    }
		}
		break;

	    case 'S':
		if (strncmp(text + 1, "SOT:", 4) == 0)
		{
		    gotsot = 1;
		    stripSoft = 0;
		}
		break;

	    case 'F':
		if (strncmp(text + 1, "FMPT", 4) == 0)
		{
                    s = text + 5;
                    m->from.point = atoi(s + 1);
                }
                else if (strncmp(text + 1, "FLAGS", 5) == 0)
                {
                    parseflags(text + 7, m);
                }

                break;

	    case 'T':
		if (strncmp(text + 1, "TOPT", 4) == 0)
                {
                    s = text + 5;
                    m->to.point = atoi(s + 1);
                }
                else if (strncmp(text + 1, "TZUTC:", 6) == 0)
                {
                    s = text + 7;

                    while (m_isspace(*s))
                    {
                        s++;
                    }

                    m->timezone = (atoi(s) % 100) + ((atoi(s) / 100) * 60);
                    m->has_timezone = 1;
                }
                break;

	    case 'D':
		if (strncmp(text + 1, "DOMAIN", 6) != 0)
		{
		    break;
		}

		s = text + 7;
		strcpy(tmp, s);
		memset(tokens, 0, sizeof(tokens));
		parse_tokens(tmp, tokens, 4);

		if (!tokens[3])
		{
		    break;
		}

		memset(&a, 0, sizeof a);
		a = parsenode(tokens[1]);
		if (a.fidonet)
		{
		    release(m->to.domain);
		    m->to = a;
		    m->to.domain = xstrdup(tokens[0]);
		}
		memset(&a, 0, sizeof a);
		a = parsenode(tokens[3]);
		if (a.fidonet)
		{
		    release(m->from.domain);
		    m->from = a;
		    m->from.domain = xstrdup(tokens[2]);
		}
		break;

	    case 'I':
		fmpt = m->from.point; /* INTL kludge is not 4D */
		topt = m->to.point;

		if (strncmp(text + 1, "INTL", 4) != 0)
		{
		    break;
		}

		s = text + 5;
		strcpy(tmp, s + 1);
		memset(tokens, 0, sizeof(tokens));
		parse_tokens(tmp, tokens, 2);

		if (!tokens[1])
		{
		    break;
		}

		memset(&a, 0, sizeof a);
		a = parsenode(tokens[0]);
		if (a.fidonet)
		{
		    release(m->to.domain);
		    m->to = a;
		}
		memset(&a, 0, sizeof a);
		a = parsenode(tokens[1]);
		if (a.fidonet)
		{
		    release(m->from.domain);
		    m->from = a;
		}

		m->to.point = topt;    /* restore the point numbers */
		m->from.point = fmpt;

		break;
	    }

	    if ((!SW->shownotes) && (!read_verbatim))
	    {
		release(text);
		continue;
	    }
	}

	if (*text == 'S')
	{
	    if (strncmp(text, "SEEN-BY:", 8) == 0 &&
		!(SW->showseenbys || SW->shownotes || read_verbatim) &&
		(!gotsot || goteot))
	    {
		release(text);
		continue;
	    }
	}

	if (goteot && *text == '\n' &&
	    !(SW->showseenbys || SW->shownotes || read_verbatim))
	{
	    release(text);
	    continue;
	}

	/* from Roland Gautschi */

	if (SW->tabexpand && strchr(text, '\t') != NULL)
	{
	    do
	    {
		ptmp = xstrdup(text);
		release(text);

		text = xmalloc(strlen(ptmp) + SW->tabsize);
		t = strchr(ptmp, '\t');

		/* characters before \t */

		strncpy(text, ptmp, (size_t) (t - ptmp));

		/* replace \t with spaces */

		memset(text + (size_t) (t - ptmp), ' ', SW->tabsize);

		/* copy the rest */

		strcpy(text + (size_t) (t - ptmp) + SW->tabsize, t + 1);
		xfree(ptmp);
	    }
	    while (strchr(text, '\t') != NULL);
	}

	if (CurArea.echomail)
	{
	    if (afound == 0 && strlen(text) > 10 && *(text + 1) == '*')
	    {
		if (!strncmp(text, " * Origin:", 10))
		{
		    /* probably the origin line */

		    s = strrchr(text, '(');
		    if (s != NULL)
		    {
			while (*s && !m_isdigit(*s) && *s != ')')
			{
			    s++;
			}

			if (m_isdigit(*s))
			{
			    m->from = parsenode(s);
			}
		    }
		    else
		    {
			m->from.notfound = 1;
		    }
		}
	    }
	}

	if (strncmp(text, "---", 3) == 0 && strncmp(text, "----", 4) != 0 &&
	  !(SW->showtearlines || SW->shownotes || read_verbatim) &&
	    (!gotsot || goteot))
	{
	    release(text);
	    continue;
	}

	if (strncmp(text, " * Origin:", 10) == 0 &&
	  !(SW->showorigins || SW->shownotes || read_verbatim) &&
	  (!gotsot || goteot))
	{
	    release(text);
	    continue;
	}

	if ((CurArea.uucp || CurArea.news) && headerfin == 0)
	{
	    char *s;

	    if (CurArea.uucp)
	    {
		if (strncmpi(text, "To: ", 4) == 0)
		{
		    char *cpdomain, *cpname;

		    s = strchr(text, ' ') + 1;
		    parse_internet_address(s, &cpdomain, &cpname);
		    m->to.fidonet = 0;
		    m->to.internet = 0;
		    m->to.bangpath = 0;
		    m->to.notfound = 0;
		    if (strchr(cpdomain, '@') != NULL)
		    {
			m->to.internet = 1;
		    }
		    else
		    {
			m->to.bangpath = 1;
		    }
		    release(m->to.domain);
		    m->to.domain = cpdomain;
		    release (m->isto);
		    m->isto = cpname;
		    lastwasfromto = 1;
		    if (!(SW->shownotes || read_verbatim))
		    {
			release(text);
			continue;
		    }
		}
	    }

	    if ( (strncmpi(text, "From: ", 6) == 0) ||
		 (strncmpi(text, "Reply-To: ", 10) == 0) )  /* TE 03/01/98 */
	    {
		char *cpname, *cpdomain;

		s = strchr(text,' ') + 1;

		parse_internet_address(s, &cpdomain, &cpname);

		/* do not adopt a probably non-existent Reply-To User Name */
		if (strncmpi(text, "Reply-To: ", 10) != 0)
		{
		    release(m->isfrom);
		    m->isfrom = cpname;
		}
		m->from.fidonet = 0;
		m->from.internet = 0;
		m->from.bangpath = 0;
		m->from.notfound = 0;
		afound = 1;
		if (strchr(cpdomain, '@') != NULL)
		{
		    m->from.internet = 1;
		}
		else
		{
		    m->from.bangpath = 1;
		}
		release(m->from.domain);
		m->from.domain = cpdomain;
		lastwasfromto = 1;
		if (!(SW->shownotes || read_verbatim))
		{
		    release(text);
		    continue;
		}
	    }
	}

	if (*text != '\01' || SW->shownotes || read_verbatim)
	{
	    if (l == NULL)
	    {
		l = xcalloc(1, sizeof(LINE));
		m->text = l;
		l->next = l->prev = NULL;
	    }
	    else
	    {
		l->next = xcalloc(1, sizeof(LINE));
		if (l->next == NULL)
		{
		    xfree(text);
		    break;
		}
		l->next->next = NULL;
		l->next->prev = l;
		l = l->next;
	    }

	    l->block = 0;
	    if (!read_verbatim)
	    {
		/* character set translation: import to local  charset */
		l->text = translate_text(text, ltable);
		strip_control_chars(l->text);
	    }
	    else
	    {
		l->text = xstrdup(text);
	    }
	    release(text); text = l->text;
	    l->hide = (*text == '\01');

	    if (is_quote(text))
	    {
		l->quote = 1;
	    }
	    else
	    {
		l->quote = 0;
	    }

	    if (l->quote)
	    {
		if (strlen(l->text) > maxx)
		{
		    wrap(l, 1, maxy, maxx);
		    while (l->next)
		    {
			l = l->next;
		    }
		}
	    }
	    else
	    {
		if (*text != '\01' && *text != '\n' && strlen(text) > maxx)
		{
		    wrap(l, 1, maxy, maxx);
		    while (l->next)
		    {
			l = l->next;
		    }
		}
	    }
	}
	else
	{
	    release(text);
	}
    }

    MsgClose();

    if (!read_verbatim) /* recode the header, important for Russian users */
    {
        char *tmp;

        tmp = translate_text(m->isfrom, ltable);
        release(m->isfrom); m->isfrom = tmp;
        strip_control_chars(m->isfrom);

        tmp = translate_text(m->isto, ltable);
        release(m->isto); m->isto = tmp;
        strip_control_chars(m->isto);

        tmp = translate_text(m->subj, ltable);
        release(m->subj); m->subj = tmp;
        strip_control_chars(m->subj);
    }

    if (set_rcvd)
    {
	checkrcvd(m, n);
    }
    read_verbatim = 0;
    return m;
}

/*
 *  checkrcvd(); Checks to see if a message has been recieved. If so,
 *  reads the message header again (avoiding translations done in the
 *  original readinf process) and then writes the header to the message
 *  base.
 */

void checkrcvd(msg * m, unsigned long n)
{
    msg *mn;
    int set_received = 0, j;

    if (m->attrib.rcvd)
    {
	return;
    }

    m->times_read++;

    if (is_sameaddr(&m->to))
    {
        if (SW->receiveallnames)
        {
            for (j = 0; j < MAXUSERS && (!set_received); j++)
            {
                if (user_list[j].name != NULL &&
                    stricmp(user_list[j].name, m->isto) == 0)
                {
                    set_received = 1;
                }
            }
        }
        else
        {
            if (stricmp(ST->username, m->isto) == 0)
            {
                set_received = 1;
            }
        }
    }

    if (set_received)
    {
	mn = MsgReadHeader(n, RD_HEADER);
	if (mn == NULL)
	{
	    return;
	}
	mn->attrib.rcvd = 1;
	m->attrib.rcvd = 1;
	m->newrcvd = 1;
	mn->times_read++;
	MsgWriteHeader(mn, WR_HEADER);
	dispose(mn);
    }
}

static int is_sameaddr(ADDRESS * msg)
{
    int j;

    if (!CurArea.netmail)
    {
	/* we only care about the address in netmail areas */
	return 1;
    }

    if (!SW->receivealladdr)
    {
        if (msg->zone != CurArea.addr.zone ||
            msg->net != CurArea.addr.net   ||
            msg->node != CurArea.addr.node ||
            msg->point != CurArea.addr.point)
        {
            return 0;
        }
        return 1;
    }
    else
    {
        for (j = 0; j < SW->aliascount; j++)
        {
            if (alias[j].zone  == msg->zone  &&
                alias[j].net   == msg->net   &&
                alias[j].node  == msg->node  &&
                alias[j].point == msg->point)
            {
                return 1;
            }
        }
        return 0;
    }
}

/*
 *  Clears a message only - wipes the slate clean.
 */

void clearmsg(msg * m)
{
    if (m == NULL)
    {
	return;
    }

    /* kill the header stuff */

    release(m->reply);
    release(m->msgid);
    release(m->isfrom);
    release(m->isto);
    release(m->subj);
    release(m->to.domain);
    release(m->from.domain);
    release(m->replyarea);
    release(m->charset_name);

    if (m->text)
    {
	/* kill the text */
	m->text = clearbuffer(m->text);
    }

    /* clear the whole lot */
    memset(m, 0, sizeof *m);

    /* set the defaults */
    m->attrib.priv = CurArea.priv;
    m->attrib.crash = CurArea.crash;
    m->attrib.hold = CurArea.hold;
    m->attrib.direct = CurArea.direct;
    m->attrib.killsent = CurArea.killsent;
    m->attrib.local = 1;
}

int setcwd(char *path)
{
    char *p;

    p = strchr(path, ':');
    if (p == NULL)
    {
	p = path;
    }

    if (*p == ':')
    {
	p++;
#if defined(OS2)
	setDefaultDisk((unsigned short)(toupper(*path) - 'A' + 1));
#elif defined(MSDOS) && !defined(__FLAT__)
	bdos(14, toupper(*path) - 'A', 0);
#elif defined(__FLAT__) && defined(MSDOS)
	{
	    unsigned dummy;
	    _dos_setdrive(toupper(*path) - 'A' + 1, &dummy);
	}
#endif
    }

    return changeDir(p);
}

int isleap(int year)
{
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

unsigned long unixtime(const struct tm *tm)
{
    int year, i;
    unsigned long result;

    result = 0UL;
    year = tm->tm_year + 1900;

    /* traverse through each year */

    for (i = 1970; i < year; i++)
    {
	result += 31536000UL;  /* 60s * 60m * 24h * 365d = 31536000s */
	if (isleap(i))
	{
	    /* it was a leap year; add a day's worth of seconds */
	    result += 86400UL;  /* 60s * 60m * 24h = 86400s */
	}
    }

    /*
     *  Traverse through each day of the year, adding a day's worth
     *  of seconds each time.
     */

    for (i = 0; i < tm->tm_yday; i++)
    {
	result += 86400UL;  /* 60s * 60m * 24h = 86400s */
    }

    /* now add the number of seconds remaining */

    result += 3600UL * tm->tm_hour;
    result += 60UL * tm->tm_min;
    result += (unsigned long) tm->tm_sec;

    return result;
}

unsigned long sec_time(void)
{
    time_t now;
    struct tm *tm;
    now = time(NULL);
    tm = localtime(&now);
    return unixtime(tm);
}

/*
 *  Inserts a line after the passed line and returns a pointer to it.
 */

LINE *InsertAfter(LINE * l, char *text)
{
    LINE *nl;

    nl = xcalloc(1, sizeof(LINE));
    nl->text = xstrdup(text);

    if (l == NULL)
    {
	return nl;
    }

    nl->next = l->next;
    nl->prev = l;
    l->next = nl;
    if (nl->next)
    {
	nl->next->prev = nl;
    }

    return nl;
}


/* Translate a token for the origin line */

static void trans_token(char *line, int maxlen, msg *m)
{

    char *old, *cp, *cp2, *atm;
    int linelen = 0, digits;
    struct tm tm;
    long count = -1, quote_count = -1, temp;

    if (line == NULL || maxlen < 1)
    {
	return;
    }

    cp2 = cp = old = xstrdup(line);
    *line = 0;

    while (cp != NULL && maxlen - 1 > linelen)
    {
	cp = strchr(cp2, '@');
	if (cp != NULL)
	{
	    *cp = '\0';
	}
	linelen += sprintf(line + linelen, "%-.*s",
			   maxlen - 1 - linelen, cp2);
	if (cp == NULL)
	{
	    break;
	}
	cp++;
	switch (toupper(*cp))
	{
	case '@':                      /* escaped @ character */
	    strcat(line + (linelen), "@");
	    linelen++;
	    break;
	case 'N':                      /* insert full name */
	    if (m->isto != NULL)
	    {
		linelen += sprintf(line + linelen, "%-.*s",
				   maxlen - 1 - linelen,
				   m->isto);
	    }
	    break;
	case 'A':                      /* insert reply area name */
	    linelen += sprintf(line + linelen, "%-.*s",
			       maxlen - 1 - linelen,
			       CurArea.tag);
	    break;
	case 'S':                      /* insert subject */
	    if (m->subj != NULL)
	    {
		linelen += sprintf(line + linelen, "%-.*s",
				   maxlen - 1 - linelen,
				   m->subj);
	    }
	    break;
	case 'Y':                      /* insert user's full name */
	    linelen += sprintf(line + linelen, "%-.*s",
			       maxlen - 1 - linelen,
			       m->isfrom);
	    break;
	case 'I':                      /* insert message size */
	    if (count == -1)
	    {
		count_bytes(m->text, &count, &quote_count);
	    }
	    for (digits = 0, temp = count; temp !=0; digits++)
	    {
		temp = temp / 10;
	    }
	    if (!digits)
	    {
		digits++;
	    }
	    if (maxlen - 1 - linelen >= digits)
	    {
		linelen += sprintf(line + linelen, "%ld", count);
	    }
	    else
	    {
		linelen = maxlen;
	    }
	    break;

	case 'Q':                      /* in sert quote % */
	    if (count == -1)
	    {
		count_bytes(m->text, &count, &quote_count);
	    }
	    if (maxlen - 1 - linelen >= ((quote_count == count) ? 4 : 3))
	    {
		linelen += sprintf(line + linelen, "%02ld%%",
				   quote_count * 100 / count);
	    }
	    else
	    {
		linelen = maxlen;
	    }
	    break;
	case 'D':                      /* insert date */
	    tm = *localtime(&m->timestamp);
	    switch (toupper(*(++cp)))
	    {
	    case 'D':                /* day */
		if (maxlen - 1 - linelen >= 2)
		{
		    linelen += sprintf(line + linelen, "%2.2d",
				       tm.tm_mday);
		}
		else
		{
		    linelen = maxlen;
		}
		break;
	    case 'W':                /* week day (mon, tue, ...) */
		if (maxlen - 1 - linelen >= 3)
		{
		    atm = atime(m->timestamp);
		    linelen += sprintf(line + linelen, "%3.3s", atm);
		}
		else
		{
		    linelen = maxlen;
		}
		break;
	    case 'M':                /* month (jan, feb, .....) */
		if (maxlen - 1 - linelen >= 3)
		{
		    atm = atime(m->timestamp);
		    linelen += sprintf(line + linelen, "%3.3s",
				       atm + 4);
		}
		else
		{
		    linelen = maxlen;
		}
		break;
	    case 'Y':                /* two digit year */
		if (maxlen - 1 - linelen >= 2)
		{
		    linelen += sprintf(line + linelen, "%2.2d",
				       tm.tm_year % 100);
		}
		else
		{
		    linelen = maxlen;
		}
		break;
	    case 'C':                /* century: 1900 - 1999: 20, etc. */
		if (maxlen - 1 - linelen >= 2)
		{
		    linelen += sprintf(line + linelen, "%2.2d",
				       (tm.tm_year / 100) + 20);
		}
		else
		{
		    linelen = maxlen;
		}
		break;
	    case '4':                /* four digit year */
		if (maxlen - 1 - linelen >= 4)
		{
		    linelen += sprintf(line + linelen, "%4.4d",
				       tm.tm_year + 1900);
		}
		else
		{
		    linelen = maxlen;
		}
		break;
	    default:                 /* date in format DD Mon YY */
		cp--;
		if (maxlen - 1 - linelen >= 9)
		{
		    linelen += sprintf(line + linelen, "%-9.9s",
				       mtime(m->timestamp));
		}
		else
		{
		    linelen = maxlen;
		}
		break;
	    }
	    break;
	case 'T':                      /* insert time hh:mm:ss */
	    if (maxlen - 1 - linelen >= 8)
	    {
		tm = *localtime(&m->timestamp);
		linelen += sprintf(line + linelen, "%2.2d:%2.2d:%2.2d",
				   tm.tm_hour, tm.tm_min, tm.tm_sec);
	    }
	    else
	    {
		linelen = maxlen;
	    }
	    break;
	case 'F':    /* First Name in TO: Field of new message */
	    linelen += sprintf(line + linelen, "%-.*s",
			       maxlen - linelen - 1, firstname(m->isto));
	    break;
	case 'L':     /* Last Name in TO: Field of new message */
	    linelen += sprintf(line + linelen, "%-.*s",
			       maxlen - linelen - 1, lastname(m->isto));
	    break;
	}                                  /* end switch */
	cp2 = ++cp;
    }                                      /* end while */
    xfree(old);
}


/*
 *  Gets the origin line to use.
 */

static char *GetFastEchoOrigin(int board)
{
    FILE *fp;
    CONFIG feconfig;
    Area fearea;
    ExtensionHeader feexthdr;
    static OriginLines feorigin;
    int cnt, found;

    fp = fopen(ST->fecfgpath, "rb");
    if (fp == NULL)
    {
	return NULL;
    }

    if (fread(&feconfig, sizeof feconfig, 1, fp) != 1)
    {
	return NULL;
    }

    fseek(fp, sizeof feconfig + feconfig.offset + feconfig.NodeCnt *
      feconfig.NodeRecSize, SEEK_SET);

    do
    {
	if (fread(&fearea, sizeof fearea, 1, fp) != 1)
	{
	    return NULL;
	}
    }
    while (board != fearea.board);

    cnt = 0;
    found = 0;

    fseek(fp, sizeof feconfig, SEEK_SET);
    while (!found)
    {
	fread(&feexthdr, sizeof feexthdr, 1, fp);
	if (feexthdr.type == EH_ORIGINS)
	{
	    do
	    {
		if (fread(&feorigin, sizeof feorigin, 1, fp) != 1)
		{
		    return NULL;
		}
		cnt++;
	    }
	    while (cnt <= (fearea.flags.origin));
	    found = 1;
	}
	else
	{
	    fseek(fp, feexthdr.offset, SEEK_CUR);
	}
    }

    fclose(fp);

    if (feorigin.line && *feorigin.line != '\0')
    {
	return feorigin.line;
    }
    else
    {
	return NULL;
    }
}

void GetOrigin(char *origin)
{
    FILE *fp;
    char path[255];

    if (n_origins > 1)          /* origin shuffling */
    {
	release(ST->origin);
	ST->origin = xstrdup(origins[rand_number(n_origins)]);
    }

    if (!SW->override)
    {
	if (CurArea.msgtype == SQUISH)
	{
	    sprintf(path, "%s.sqo", CurArea.path);
	}
	else
	{
	    sprintf(path, "%s\\origin", CurArea.path);
	}

	fp = fopen(path, "r");
	if (fp != NULL)
	{
	    int i, orgnum, orgcnt;

	    i = 0;
	    orgnum = 0;
	    orgcnt = 0;

	    do
	    {
		*origin = '\0';
		fgets(origin, 65, fp);
		if (*origin)
		{
		    orgcnt++;
		}
	    }
	    while (!feof(fp));

	    orgnum = rand_number(orgcnt) + 1;

	    rewind(fp);

	    do
	    {
		*origin = '\0';
		fgets(origin, 65, fp);
		if (*origin)
		{
		    i++;
		}
		if (i == orgnum)
		{
		    break;
		}
	    }
	    while (!feof(fp));

	    fclose(fp);
	}
	else
	{
	    if (ST->origin != NULL)
	    {
		sprintf(origin, "%s", ST->origin);
	    }
	    else
	    {
		sprintf(origin, "%s", ST->username);
	    }
	    if (CurArea.msgtype == QUICK && areas_type == FASTECHO)
	    {
		char *feorigin;
		feorigin = GetFastEchoOrigin(CurArea.board);
		if (feorigin != NULL)
		{
		    sprintf(origin, "%s", feorigin);
		}
	    }
	}
    }
    else
    {
	if (ST->origin != NULL)
	{
	    sprintf(origin, "%s", ST->origin);
	}
	else
	{
	    sprintf(origin, "%s", ST->username);
	}
    }
    striptwhite(origin);
}

static void InvalidateKludges(msg * m)
{
    LINE *line;

    line = m->text;
    while (line != NULL)
    {
	char *p;
	p = line->text;
	if (p != NULL)
	{
	    if (*p == '\01')
	    {
		*p = '@';
	    }
	    else if (strstr(p, "SEEN-BY: ") == p)
	    {
		*(p + 4) = '+';
	    }
	    else if (strstr(p, "---\n") == p || strstr(p, "--- ") == p ||
	      strstr(p, " * Origin: ") == p)
	    {
		*(p + 1) = '+';
	    }
	}
	line = line->next;
    }
}

static void StripKludges(msg * m, int *got_originline, char *origin,
				  int *got_tearline,   char *tearline)
{
    LINE *line;
    int got_origin, got_tear, got_text;

    /* traverse forwards through linked list until the end */

    line = m->text;
    while (line->next != NULL)
    {
	line = line->next;
    }

    /* now go backwards, removing any unwanted stuff */

    got_origin = 0;
    got_tear = 0;
    got_text = 0;

    while (line != NULL)
    {
	char *p;

	p = line->text;
	if (p != NULL)
	{
	    if (strstr(p, "\01MSGID: ") == p || strstr(p, "\01REPLY: ") == p ||
	      strstr(p, "\01FLAGS ") == p || strstr(p, "\01PID: ") == p ||
	      strstr(p, "\01SOT:") == p || strstr(p, "\01EOT:") == p ||
	      strstr(p, "\01CHRS: ") == p || strstr(p, "\01TZUTC:") == p ||
              strstr(p, "\01CODEPAGE:") == p)
	    {
		*p = '\0';
	    }
	    else if (CurArea.netmail)
	    {
		if (strstr(p, "\01INTL ") == p || strstr(p, "\01TOPT ") == p ||
		  strstr(p, "\01FMPT ") == p || strstr(p, "\01DOMAIN ") == p ||
		  strstr(p, "\01Via ") == p)
		{
		    *p = '\0';
		}
	    }
	    else if (!CurArea.netmail)
	    {
		if (strstr(p, "SEEN-BY: ") == p || strstr(p, "\01PATH: ") == p)
		{
		    *p = '\0';
		}
		else if (!got_text && !got_origin && strstr(p, " * Origin: ") == p)
		{
		    *p = '\0';
		    got_origin = 1;

		    if (origin != NULL)  /* preserve an user-edited origin */
		    {
			strncpy (origin, p + 11, 79);
			origin[79] = 0;
			p = strrchr(origin, '(');
			if (p != NULL)
			{
			    *p = '\0';
			}
			striptwhite(origin);
		    }
		}
		else if (!got_text && !got_tear && (strstr(p, "---\n") == p ||
		  strstr(p, "--- ") == p))
		{
		    /* preserve an user-edited tearline */
		    if (tearline != NULL)
		    {
			strncpy (tearline, p+4, SW->xxltearline ? 75 : 31);
			tearline[SW->xxltearline ? 75 : 31] = 0;
			striptwhite (tearline);
		    }
		    *p = '\0';
		    got_tear = 1;
		}
		else if (!got_text && (*p == '\n' || *p == '\r'))
		{
		    *p = '\0';
		}
		else if (!got_text && *p != '\0' && *p != '\n' && *p != '\r')
		{
		    got_text = 1;
		}
	    }
	}

	line = line->prev;
    }

    *got_originline = got_origin;
    *got_tearline   = got_tear;
}

char *space_after_period(LINE *ln)
{
    int snt_count, spc_count;
    snt_count = 0;
    spc_count = 0;
    while (ln != NULL)
    {
	char *p;
	p = ln->text;
	while (*p != '\0')
	{
	    if (*p == '.' && *(p + 1) == ' ')
	    {
		snt_count++;
		spc_count++;
		if (*(p + 2) == ' ')
		{
		    spc_count++;
		}
	    }
	    p++;
	}
	ln = ln->prev;
    }
    if (snt_count != 0 && spc_count / snt_count == 2)
    {
	return "  ";
    }
    else
    {
	return " ";
    }
}

static int askgate(char *which)
{
    char str[80];
    int rc;

    sprintf(str, "Send this message via %s gate?", which);

    while (1)
    {
        rc = ChoiceBox("", str, "Yes", "No",
                       strcmp(which, "zone") ? NULL : "Help");

        switch(rc)
        {
        case ID_ONE:
            return 1;
        case ID_TWO:
            return 0;
        case ID_THREE:
            if (!strcmp(which, "zone"))
            {
                DoHelp(6);
            }
            break;
        }
    }
}


/*
 *  Sequence of events:
 *
 *  Original address is saved (what is displayed);
 *  Domain gates are checked for and address is modified if one found;
 *  if no domain gates, search for UUCP gate && mod address if found.
 *  check INTL
 *  check MSGID
 *  check REPLY
 *  do PID if one,
 *  If we found domain gate, do DOMAIN with original saved addresses.
 *  if we found UUCP, then do a "to:" kludge.
 */

/*
 *  Writes a message to disk.
 */

int writemsg(msg * m)
{
    LINE *curr, *l, *ufrom, *uto, *ureplyto, *xblank,
         *xtear, *xorigin, *ublank;
    ADDRESS to;
    ADDRESS from;
    unsigned long n;            /* UMSGID msgnum */
    unsigned long length;       /* length in bytes of the message */
    char text[255];             /* buffer useage */
    char origin[255];           /* out origin line */
    char tearline[76];          /* user-defined output tearline */
    char *uucp_from;            /* saved UUCP from address */
    char *uucp_to;              /* saved UUCP to address */
    int domain_gated, uucp_gated;
    int do_zonegate = 0;
    char *s;
    int i, abortWrite, got_origin, got_tear, ctrl;
    static unsigned long now = 0L;
    LOOKUPTABLE *ltable = NULL;
    int write_chrs_kludge;
    const int tear_max = SW->xxltearline ? 79 : 35;

    char *temptext;

    domain_gated = 0;
    uucp_gated = 0;
    length = 0;
    ctrl = 1;
    n = m->msgnum;
    curr = NULL;
    uto = NULL;
    ureplyto = NULL;
    ufrom = NULL;
    uucp_from = NULL;
    uucp_to = NULL;
    xorigin = NULL;
    xtear = NULL;
    xblank = NULL;
    ublank = NULL;
#if 0
    while (MsgLockArea() == -1)          /* Lock the Msg area for writing */
    {
	int ret;

	ret = ChoiceBox(" Error! ", "Could not write message!",
			"Retry", "Cancel", NULL);

	if (ret == ID_TWO)
	{
	    return FALSE;
	}
    }
#endif
    if (now == 0L)
    {
	now = sec_time();
    }

    if (m->invkludges)
    {
	InvalidateKludges(m);
	got_origin = got_tear = 0;
    }
    else if (!m->rawcopy)
    {
	StripKludges(m, &got_origin, origin, &got_tear, tearline);
    }

    ltable = get_writetable(CurArea.eightbits ? ST->output_charset : "ASCII",
	       &write_chrs_kludge);

    if (!m->rawcopy)
    {
	/* save the original address */

	to = m->to;
	from = m->from;

	/* do domain gating */

        if (SW->domains &&
            m->to.domain &&
            m->from.domain &&
            stricmp(m->from.domain, m->to.domain) &&
            (SW->gate == GDOMAINS || SW->gate == BOTH ||
             (SW->gate == GASK && askgate("domain"))) &&
            !m->to.internet /* in this case, "domain" string is e-mail addr */
           )
	{
	    /*
	     *  If we have two domains and they're different, then we want to
	     *  gate. If we have a to: domain and no from: domain, then we
	     *  still may want to gate.  If we don't have a to: domain, then
	     *  we don't want to gate the message (we assume it's destined to
	     *  our own network).
	     */

            for (i = 0; i < SW->domains; i++)
            {
                if (!stricmp(domain_list[i].domain, m->to.domain))
                {
                    domain_gated = 1;

                    if (m->attrib.crash || m->attrib.direct ||
                        m->attrib.hold  || m->attrib.immediate)
                    {
                        int ret;

                        ret = ChoiceBox(" Direct Message ",
                                        " Direct Message "
                                        "(crash, dir, imm or hold) to?",
                                        "Domain Gate", "Destination Node", NULL);

                        if (ret == ID_ONE)
                        {
                            m->to = domain_list[i];
                            if (domain_list[i].domain)
                            {
                                m->to.domain = xstrdup(domain_list[i].domain);
                            }
                        }
                    }
                    else
                    {
                        m->to = domain_list[i];
                        if (domain_list[i].domain)
                        {
                            m->to.domain = xstrdup(domain_list[i].domain);
                        }
                    }
                    break;
                }
            }
        }


	/* do uucp gating */

	if (m->to.internet || m->to.bangpath)
	{
	    uucp_gated = 1;
	    uucp_to = m->to.domain;
	    if (!CurArea.echomail)              /* don't change adresses */
	    {                                   /* in echomail areas     */
		m->to = uucp_gate;
	    }
	    if (uucp_gate.domain)
	    {
		m->to.domain = xstrdup(uucp_gate.domain);
	    }

            if (CurArea.netmail)
            {
                /* AKA-Matching for UUCP gated messages */
                akamatch(&(m->from), &(m->to));
            }
	}

	if (m->from.internet || m->from.bangpath)
	{
	    uucp_gated = 1;
	    uucp_from = xstrdup(m->from.domain);
	    if (!CurArea.echomail)              /* don't change adresses */
	    {                                   /* in echomail areas     */
		m->from = uucp_gate;
	    }
	    if (uucp_gate.domain)
	    {
		m->from.domain = xstrdup(uucp_gate.domain);
	    }
	}

	/* do the netmail stuff */

	    /* uccp is an ADD ON flag for netmail, not a replacement,
	       so I commented it out below, because it lead to INTL
	       kludges in echomail areas and the like */
	if (CurArea.netmail /* || CurArea.uucp */ )
	{

            /* print INTL kludge */
 	    if (m->from.zone != m->to.zone || m->from.zone != thisnode.zone)
	    {
		sprintf(text, "\01INTL %d:%d/%d %d:%d/%d\r", m->to.zone,
		  m->to.net, m->to.node, m->from.zone, m->from.net, m->from.node);
		curr = InsertAfter(curr, text);
	    }

            /* print FMPT / TOPT kludges */
	    if (m->to.point)
	    {
		sprintf(text, "\01TOPT %d\r", m->to.point);
		curr = InsertAfter(curr, text);
	    }

            if (! (!m->attrib.direct && !m->attrib.crash && m->from.point &&
                   SW->pointnet != 0 && CurArea.netmail))
            {
                /* privatenet / fakenet is not used */
                if (m->from.point)
                {
                    sprintf(text, "\01FMPT %d\r", m->from.point);
                    curr = InsertAfter(curr, text);
                }
            }

            /* see if we want to zonegate and set the ZON flag if
               necessary. The actual readressing proceudre is done later. */

            if (m->from.zone != m->to.zone && !m->attrib.direct &&
                !m->attrib.crash && CurArea.netmail &&
                (SW->gate == GZONES || SW->gate == BOTH ||
                 m->attrib.zon ||
                 (SW->gate == GASK && askgate("zone"))
                ))
            {
                do_zonegate = 1;
                m->attrib.zon = 1;
            }

        }

	/* these babies go everywhere */

	if (m->new)
	{
	    if (SW->msgids)
	    {
		sprintf(text, "\01MSGID: %s %08lx\r",
                        SW->domainmsgid ? show_address(&from) : show_4d(&from),
                        now);

		now++;
		curr = InsertAfter(curr, text);
	    }
	}
	else
	{
	    if (m->msgid)
	    {
		sprintf(text, "\01MSGID: %s\r", m->msgid);
		curr = InsertAfter(curr, text);
	    }
	}

        if (m->has_timezone)
        {
            sprintf(text, "\01TZUTC: %.4li\r",
                    (long)(((m->timezone / 60) * 100) + (m->timezone % 60)));
            curr = InsertAfter(curr, text);
        }

	if (m->reply && (!m->new || SW->msgids))
	{
	    sprintf(text, "\01REPLY: %s\r", m->reply);
	    curr = InsertAfter(curr, text);
	}

	if ((SW->echoflags && CurArea.echomail) || (CurArea.netmail))
	{
	    char flags[255];

            printflags(flags, m, CurArea.msgtype, CurArea.echomail);

            if (*flags)
            {
                sprintf(text, "\01FLAGS %s\r", flags);
                curr = InsertAfter(curr, text);
            }
	}

	if (!SW->usetearlines || SW->usepid || CurArea.netmail)
	{
	    sprintf(text, "\01PID: %s %s\r", PROG, VERNUM VERPATCH);
	    curr = InsertAfter(curr, text);
	}

	if (CurArea.eightbits && write_chrs_kludge)
	{
	    sprintf(text, "\01CHRS: %s 2\r", ST->output_charset);
	    curr = InsertAfter(curr, text);
            if (ST->output_charset[0] != 'C' || ST->output_charset[1] != 'P')
            {
                int cp;

                /* codepage of this kludge is non-obvious,
                   add CODEPAGE kludge */

                cp = get_codepage_number(ST->output_charset);
                if (cp != 0)
                {
                    sprintf(text, "\01CODEPAGE: %d\r", cp);
                    curr = InsertAfter(curr, text);
                }
            }
	}


	/* domain gating */

	if (domain_gated)
	{
	    sprintf(text, "\01DOMAIN %s %d:%d/%d.%d %s %d:%d/%d.%d\r",
	      to.domain, to.zone, to.net, to.node, to.point, from.domain,
	      from.zone, from.net, from.node, from.point);
	    curr = InsertAfter(curr, text);
	}

	if (SW->soteot)
	{
	    strcpy(text, "\01SOT:\r");
	    curr = InsertAfter(curr, text);
	}

	if (uucp_gated)
	{
	    int cr = 0;  /* we want to insert a \n after header info */

	    if (uucp_from)
	    {
		sprintf(text, "From: %s\r", uucp_from);
		curr = InsertAfter(curr, text);
		ufrom = curr;
		cr = 1;
	    }

	    if (uucp_to)
	    {
		sprintf(text, "To: %s\r", uucp_to);
		curr = InsertAfter(curr, text);
		uto = curr;
		cr = 1;

                if (ST->uucpreplyto != NULL)
                {
                    sprintf(text, "Reply-To: %s\r", ST->uucpreplyto);
                    curr = InsertAfter(curr, text);
                    ureplyto = curr;
                }
	    }

	    if (cr == 1)
	    {
		strcpy(text, "\r");
		curr = InsertAfter(curr, text);
		ublank = curr;
	    }
	}

	/*
	 *  Actually assign the kludges we just created to the message body
	 *  (before the text).
	 */

	if (curr != NULL)
	{
	    curr->next = m->text;
	    if (m->text != NULL)
	    {
		m->text->prev = curr;
	    }

	    while (curr->prev != NULL)
	    {
		curr = curr->prev;
	    }

	    m->text = curr;
	}
    }

    l = m->text;
    while (l)
    {
	if (l->text == NULL)
	{
	    l->text = xstrdup("\r");
	}

	if (*l->text)
	{
	    s = strrchr(l->text, '\n');
	    if (s != NULL)
	    {
		*s = '\r';
	    }
	    else
	    {
		char *text;

		text = xmalloc(strlen(l->text) + 5);
		strcpy(text, l->text);
		if (l->quote)
		{
		    strcat(text, "\r");
		}
		else
		{
		    if ((l->next == NULL || *(l->next->text) == '\0') &&
		      text[strlen(text) - 1] != '\r')
		    {
			strcat(text, "\r");
		    }
		    else if (l->next != NULL && *(l->next->text) == '\n' &&
		      text[strlen(text) - 1] != '\r')
		    {
			strcat(text, "\r");
		    }
		    else if (!m_isspace(*(text + strlen(text) - 1)))
		    {
			if (*(text + strlen(text) - 1) == '.')
			{
			    strcat(text, space_after_period(l));
			}
			else
			{
			    strcat(text, " ");
			}
		    }
		}
		release(l->text);
		l->text = text;
	    }
	}
	l = l->next;
    }

    /* find the end of the message */

    curr = m->text;
    while (curr->next != NULL)
    {
	curr = curr->next;
    }

    if (!m->rawcopy)
    {
	if (SW->usetearlines && SW->useoriginlines &&
	    *curr->text != '\r' && CurArea.echomail)
	{
	    strcpy(text, "\r");
	    curr = InsertAfter(curr, text);
	    xblank = curr;
	}

	if (SW->soteot)
	{
	    strcpy(text, "\01EOT:\r");
	    curr = InsertAfter(curr, text);
	}

	if (CurArea.netmail && SW->netmailvia)
	{
	    time_t td;
	    char time_str[80];

	    td = time(NULL);
	    strftime(time_str, 80, "%a %b %d %Y at %H:%M UTC", gmtime(&td));
	    sprintf(text, "\01Via " PROG " " VERNUM VERPATCH " %s, %s\r",
	      show_address(&from), time_str);
	    curr = InsertAfter(curr, text);
	}

	if (CurArea.echomail)
	{
	    if (SW->usetearlines)
	    {
		/* add the tearline */

		if (got_tear)  /* preserve user-defined tear line */
		{
		    sprintf(text, "--- %s\r", tearline);
		}
                else
                {
                    make_tearline(text);
                    strcat(text, "\r");
		}

		/* make sure it is not longer than 35 characters + \r*/
		if (strlen(text) > tear_max + 1) /* +1 for \r */
		{
		    text[tear_max + 1] = '\0';
		    text[tear_max] = '\r';
		}

		curr = InsertAfter(curr, text);
		xtear = curr;
	    }

	    if (SW->useoriginlines)
	    {
		/* add the origin line */

		if (!got_origin)
		{
		    GetOrigin(origin);
		}
		trans_token(origin, 79, m);
		sprintf(text, " * Origin: %s (%s)\r", origin,
		  SW->domainorigin ? show_address(&from) : show_4d(&from));

		/* make sure it is not longer than 79 characters + \r */
		i = strlen(text) - 80;
		if (i > 0)
		{
		    s = strstr(text, " (");
		    *s = 0;
		    if (i > strlen(text))  /* assure data integrity */
		    {
			i = strlen(text);
		    }
		    *s = ' ';
		    memmove (s - i, s, strlen(s) + 1);
		}

		curr = InsertAfter(curr, text);
		xorigin = curr;
	    }
	}
    }

    /* we've made the new message up, now return it's length */

    l = m->text;
    while (l)
    {
        if (*(l->text) != '\01')
	{
	    ctrl = 0;
	}
	if (!ctrl)
	{
            temptext = translate_text(l->text, ltable);
            if (temptext != NULL)
                length += strlen(temptext);
            release(temptext);
	}
	l = l->next;
    }

    length++; /* account for the \0 byte */

    if (!m->rawcopy)
    {
	/* remap point originated crashmail */

        /* if you change this if condition, don't forget to change the same
           conditions some lines above in the generation of FMPT also! */
	if (!m->attrib.direct && !m->attrib.crash && m->from.point &&
	  SW->pointnet != 0 && CurArea.netmail)
	{
	    m->from.net = SW->pointnet;
	    m->from.node = m->from.point;
	    m->from.point = 0;
	}

	/* do any required zone gating */
        if (do_zonegate)
	{
	    m->to.node = m->to.zone;
	    m->to.zone = m->from.zone;
	    m->to.net = m->from.zone;
	}
    }

    if (!m->rawcopy) /* recode the header, important for Russian users */
    {
        char *tmp;

        tmp = translate_text(m->isfrom, ltable);
        do_softcrxlat(tmp);
        release(m->isfrom); m->isfrom = tmp;

        tmp = translate_text(m->isto, ltable);
        do_softcrxlat(tmp);
        release(m->isto); m->isto = tmp;

        tmp = translate_text(m->subj, ltable);
        do_softcrxlat(tmp);
        release(m->subj); m->subj = tmp;
    }

    abortWrite = 0;
    while (MsgWriteHeader(m, WR_ALL) == ERR_OPEN_MSG && !abortWrite)
    {
	int ret;

	ret = ChoiceBox(" Error! ", "Could not write message!",
	  "Retry", "Cancel", NULL);

	if (ret == ID_TWO)
	{
	    abortWrite = 1;
	}
    }

    if (!abortWrite)
    {
	l = m->text;
	while (l && !abortWrite)
	{
	    if (l->text && *l->text != '\0')
	    {
		if (!m->rawcopy)
		{
		    temptext = translate_text(l->text, ltable);
                    do_softcrxlat(temptext);
		    /* output CHRS translation */
		}
		else
		{
		    temptext = xstrdup(l->text);
		}
		while ((!abortWrite) &&
		       (MsgWriteText(temptext, n, length) == FALSE))
		{
		    int ret;

		    ret = ChoiceBox(" Error! ", "Could not write message!",
				    "Retry", "Cancel", NULL);
		    if (ret == ID_TWO)
		    {
			abortWrite = 1;
		    }
		}

		release(temptext);
	    }

	    /*
	     *  The \r's have to be turned back into \n's because the
	     *  message might be a CC: and so be needed again.
	     */

	    s = strrchr(l->text, '\r');
	    if (s != NULL)
	    {
		*s = '\n';
	    }

	    l = l->next;
	}

	MsgWriteText(NULL, n, length);
	MsgClose();
    }

    CurArea.new = 1;
    echotoss_add(&CurArea);

    /*
     *  Clean up. If this message is a CC, then we don't want this
     *  temporary information to remain.
     */

    if ((uucp_from || uucp_to) && (ublank != NULL))
    {
	deleteCrapLine(ublank);
    }

    if (uucp_from)
    {
	release(uucp_from);
	deleteCrapLine(ufrom);
    }

    if (uucp_to)
    {
	release(uucp_to);
	deleteCrapLine(uto);
        if (ureplyto != NULL)
        {
            deleteCrapLine(ureplyto);
        }
    }

    deleteCrapLine(xblank);
    deleteCrapLine(xtear);
    deleteCrapLine(xorigin);

#if 0
    MsgUnlockArea();
#endif

    if (abortWrite)
    {
	return FALSE;
    }
    else
    {
	return TRUE;
    }
}

static void deleteCrapLine(LINE * crap)
{
    if (crap != NULL)
    {
	if (crap->prev != NULL)
	{
	    crap->prev->next = crap->next;
	}
	if (crap->next != NULL)
	{
	    crap->next->prev = crap->prev;
	}
	release(crap->text);
	release(crap);
    }
}

#ifdef OS2

#include <os2.h>

static unsigned long setDefaultDisk(unsigned short x)
{
#ifdef OS216
    return DosSelectDisk(x);
#else
    return DosSetDefaultDisk(x);
#endif
}

static int changeDir(char *path)
{
#ifdef OS216
    return chdir(path);
#else
    return DosSetCurrentDir((PSZ) path);
#endif
}

void mygetcwd(char *buf, int len)
{
#ifdef OS216
    getcwd(buf, len);
#else
    unsigned long ulen;
    ulen = len;
    DosQueryCurrentDir(0, (PSZ) buf, &ulen);
#endif
}

#else

static int changeDir(char *path)
{
    return chdir(path);
}

void mygetcwd(char *buf, int len)
{
#ifndef PACIFIC
    getcwd(buf, len);
#else
    getcwd(buf);
#endif
}

#endif
