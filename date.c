/*
 *  DATE.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Parse various string date formats into a UNIX style timestamp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "memextra.h"
#include "strextra.h"
#include "date.h"
#include "mctype.h"

static char *month[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char *day[] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static char *attr_tokens[] =
{
    "yms",  /* year of message creation */
    "yno",  /* current year */
    "mms",  /* month of message creation */
    "mno",  /* current month */
    "dms",  /* day of message creation */
    "dno",  /* current day */
    "wms",  /* weekday of message creation */
    "wno",  /* current weekday */
    "tnm",  /* (normal) time of message creation */
    "tnn",  /* (normal) current time */
    "tam",  /* (atime) time of message creation */
    "tan",  /* (atime) current time */

    "ofn",  /* orginal from name */
    "off",  /* original from first name */
    "otn",  /* original to name */
    "otf",  /* original to first name */
    "osu",  /* original subject */
    "ooa",  /* original originination address */
    "oda",  /* orginal destination address */

    "fna",  /* from name */
    "ffn",  /* from first name */
    "fad",  /* from address */
    "tna",  /* to name */
    "tfn",  /* to first name */
    "tad",  /* to address */
    "sub",  /* subject */

    "una",  /* user name */
    "ufn",  /* user first name */
    "uad",  /* user address */
    "ceh",  /* current conference name */
    "oeh",  /* original conference name */

    "ims",  /* (iso) date of message creation */
    "ino",  /* (iso) current date */
    "cms",  /* four-digit year of message creation */
    "cno",  /* current four-digit year */

    NULL
};

#define ATTR_TOK_YMS  0
#define ATTR_TOK_YMO  1
#define ATTR_TOK_MMS  2
#define ATTR_TOK_MNO  3
#define ATTR_TOK_DMS  4
#define ATTR_TOK_DNO  5
#define ATTR_TOK_WMS  6
#define ATTR_TOK_WNO  7
#define ATTR_TOK_TNM  8
#define ATTR_TOK_TNN  9
#define ATTR_TOK_TAM  10
#define ATTR_TOK_TAN  11
#define ATTR_TOK_OFN  12
#define ATTR_TOK_OFF  13
#define ATTR_TOK_OTN  14
#define ATTR_TOK_OTF  15
#define ATTR_TOK_OSU  16
#define ATTR_TOK_OOA  17
#define ATTR_TOK_ODA  18
#define ATTR_TOK_FNA  19
#define ATTR_TOK_FFN  20
#define ATTR_TOK_FAD  21
#define ATTR_TOK_TNA  22
#define ATTR_TOK_TFN  23
#define ATTR_TOK_TAD  24
#define ATTR_TOK_SUB  25
#define ATTR_TOK_UNA  26
#define ATTR_TOK_UFN  27
#define ATTR_TOK_UAD  28
#define ATTR_TOK_CEH  29
#define ATTR_TOK_OEH  30
#define ATTR_TOK_IMS  31
#define ATTR_TOK_INO  32
#define ATTR_TOK_CMS  33
#define ATTR_TOK_CNO  34

static int valid_date(struct tm *tms)
{
    return !(tms->tm_wday > 6 || tms->tm_wday < 0 || tms->tm_mon > 11 ||
      tms->tm_mon < 0 || tms->tm_mday > 31 || tms->tm_mday < 0 ||
      tms->tm_hour > 23 || tms->tm_hour < 0 || tms->tm_min > 59 ||
      tms->tm_min < 0 || tms->tm_sec > 59 || tms->tm_sec < 0);
}

time_t parsedate(char *ds)
{
    int t, absnow, absyear;
    struct tm tm, *now;
    char work[80], *s;
    time_t n;

    if (ds == NULL || *ds == '\0')
    {
        return 0;
    }

    memset(&tm, 0, sizeof tm);
    strcpy(work, ds);

    if (strchr(ds, '-') != NULL)
    {
        /* quickbbs style date */

        s = strtok(work, "-");
        if (s != NULL)
        {
            tm.tm_mon = atoi(s) - 1;
        }
        s = strtok(NULL, "-");
        if (s != NULL)
        {
            tm.tm_mday = atoi(s);
        }
        s = strtok(NULL, " ");
        if (s != NULL)
        {
            tm.tm_year = atoi(s);
        }
        s = strtok(NULL, ":");
        if (s != NULL)
        {
            while (m_isspace(*s))
            {
                s++;
            }
            tm.tm_hour = atoi(s);
        }
        s = strtok(NULL, " ");
        if (s != NULL)
        {
            tm.tm_min = atoi(s);
        }
        tm.tm_sec = 0;
    }
    else
    {
        /* fido style date */

        s = strtok(work, " ");

        if (s == NULL)
        {
            return 0;
        }

        t = atoi(s);
        if (t == 0)
        {
            /* a usenet date */
            s = strtok(NULL, " ");
            if (s == NULL)
            {
                return 0;
            }
            t = atoi(s);
        }
        tm.tm_mday = t;
        s = strtok(NULL, " ");
        if (s == NULL)
        {
            return 0;
        }
        for (t = 0; t < 12; t++)
        {
            if (stricmp(s, month[t]) == 0)
            {
                break;
            }
        }
        if (t == 12)
        {
            t = 1;
        }
        tm.tm_mon = t;
        s = strtok(NULL, " ");
        if (s == NULL)
        {
            return 0;
        }
        tm.tm_year = atoi(s);
        s = strtok(NULL, ":");
        if (s == NULL)
        {
            return 0;
        }
        while (m_isspace(*s))
        {
            s++;
        }
        tm.tm_hour = atoi(s);
        s = strtok(NULL, ": \0");
        if (s == NULL)
        {
            return 0;
        }
        tm.tm_min = atoi(s);
        s = strtok(NULL, " ");
        if (s != NULL)
        {
            tm.tm_sec = atoi(s);
        }
        tm.tm_isdst = -1;
    }

    /* Now try to find out which century we're in and fix the year */

    n = time(NULL);
    now = localtime(&n);

    absnow  = 1900 + now->tm_year;
    absyear = 1900 + tm.tm_year;
    while (absyear <= absnow-50)
      absyear += 100;
    while (absyear > absnow+50)
      absyear -= 100;

    tm.tm_year = absyear - 1900;

    return mktime(&tm);
}

char *itime(time_t now)
{
    struct tm *tm;
    static char tmp[40];

    tm = localtime(&now);

    if (!tm || !valid_date(tm))
    {
        sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d", 1970, 1, 1, 0, 0, 0);
    }
    else
    {
        sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900,
          tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    }

    return tmp;
}

char *atime(time_t now)
{
    struct tm *tm;
    static char tmp[40];

    tm = localtime(&now);

    if (!tm || !valid_date(tm))
    {
        sprintf(tmp, "%s %s %02d %04d %02d:%02d:%02d", day[4], month[0],
          1, 1970, 0, 0, 0);
    }
    else
    {
        sprintf(tmp, "%s %s %02d %04d %02d:%02d:%02d", day[tm->tm_wday],
          month[tm->tm_mon], tm->tm_mday, tm->tm_year + 1900, tm->tm_hour,
          tm->tm_min, tm->tm_sec);
    }

    return tmp;
}

char *mtime(time_t now)
{
    struct tm *tm;
    static char tmp[21];

    tm = localtime(&now);

    if (!tm || !valid_date(tm))
    {
        sprintf(tmp, "%02d %s %02d  %02d:%02d:%02d", 1, month[0], 70, 0, 0, 0);
    }
    else
    {
        sprintf(tmp, "%02d %s %02d  %02d:%02d:%02d", tm->tm_mday,
          month[tm->tm_mon], tm->tm_year % 100, tm->tm_hour, tm->tm_min,
          tm->tm_sec);
    }

    return tmp;
}

char *qtime(time_t now)
{
    struct tm *tm;
    static char tmp[20];

    tm = localtime(&now);

    if (!tm || !valid_date(tm))
    {
        sprintf(tmp, "%s %02d %02d:%02d", month[0], 1, 0, 0);
    }
    else
    {
        sprintf(tmp, "%s %02d %02d:%02d", month[tm->tm_mon], tm->tm_mday,
          tm->tm_hour, tm->tm_min);
    }

    return tmp;
}

/* find_token - returns the token number or -1 if not found */

static int find_token(char *token)
{
    int i;
    i = 0;
    while (attr_tokens[i] != NULL)
    {
        if (stricmp(attr_tokens[i], token) == 0)
        {
            return i;
        }
        i++;
    }
    return -1;
}

/* Returns a pointer to the first name. Note: uses static memory. */

char *firstname(char *name)
{
    char *s;
    static char work[40];

    memset(work, 0, sizeof work);
    if (name == NULL)
    {
	return work;
    }
    s = strchr(name, ' ');
    if (s == NULL)
    {
        sprintf(work, "%-.39s", name);
    }
    else
    {
        *s = '\0';
        sprintf(work, "%-.39s", name);
        *s = ' ';
    }
    return work;
}

/* Returns a pointer to the last name. Note: uses static memory. */

char *lastname(char *name)
{
    char *s;
    static char work[40];

    memset(work, 0, sizeof work);
    if (name == NULL)
    {
	return work;
    }

    s = strchr(name, ' ');
    if (s == NULL)
    {
        sprintf(work, "%-.39s", name);
    }
    else
    {
        sprintf(work, "%-.39s", s + 1);
    }
    return work;
}


/* attrib_line - builds an attribution line */

char *attrib_line(msg * m, msg * old, int olda, char *format,
                  char **days, char **months)
{
    struct tm now, *tm;
    char work[256], token[5], *t;
    time_t n;
    int num;

    if (days == NULL) days = day;
    if (months == NULL) months = month;

    if (format == NULL)
    {
        return NULL;
    }

    memset(work, 0, sizeof work);
    t = work;
    n = time(NULL);
    tm = localtime(&n);
    now = *tm;

    if (old)
    {
        tm = localtime(&(old->timestamp));
    }

    while (*format)
    {
        if (*format == '%')
        {
            format++;
            switch (*format)
            {
            case '%':
                *t = *format;
                break;

            case '_':
                *t = ' ';
                break;

            default:
                memset(token, 0, sizeof token);
                strncpy(token, format, 3);
                num = find_token(token);

                switch (num)
                {
                case ATTR_TOK_YMS:
                    if (old)
                    {
                        sprintf(t, "%02d", tm->tm_year % 100);
                    }
                    break;

                case ATTR_TOK_YMO:
                    sprintf(t, "%02d", now.tm_year % 100);
                    break;

                case ATTR_TOK_MMS:
                    if (old)
                    {
                        strcpy(t, months[tm->tm_mon]);
                    }
                    break;

                case ATTR_TOK_MNO:
                    strcpy(t, months[now.tm_mon]);
                    break;

                case ATTR_TOK_DMS:
                    if (old)
                    {
                        sprintf(t, "%02d", tm->tm_mday);
                    }
                    break;

                case ATTR_TOK_DNO:
                    sprintf(t, "%02d", now.tm_mday);
                    break;

                case ATTR_TOK_WMS:
                    if (old)
                    {
                        strcpy(t, days[tm->tm_wday]);
                    }
                    break;

                case ATTR_TOK_WNO:
                    strcpy(t, days[now.tm_wday]);
                    break;

                case ATTR_TOK_TNM:
                    if (old)
                    {
                        sprintf(t, "%02d:%02d", tm->tm_hour, tm->tm_min);
                    }
                    break;

                case ATTR_TOK_TNN:
                    sprintf(t, "%02d:%02d", now.tm_hour, now.tm_min);
                    break;

                case ATTR_TOK_TAM:
                    if (old)
                    {
                        strcpy(t, atime(old->timestamp));
                    }
                    break;

                case ATTR_TOK_TAN:
                    strcpy(t, atime(n));
                    break;

                case ATTR_TOK_OFN:
                    if (old && old->isfrom)
                    {
                        strcpy(t, old->isfrom);
                    }
                    break;

                case ATTR_TOK_OFF:
                    if (old && old->isfrom)
                    {
                        strcpy(t, firstname(old->isfrom));
                    }
                    break;

                case ATTR_TOK_OTN:
                    if (old && old->isto)
                    {
                        strcpy(t, old->isto);
                    }
                    break;

                case ATTR_TOK_OTF:
                    if (old && old->isto)
                    {
                        strcpy(t, firstname(old->isto));
                    }
                    break;

                case ATTR_TOK_OSU:
                    if (old && old->subj)
                    {
                        strcpy(t, old->subj);
                    }
                    break;

                case ATTR_TOK_OOA:
                    if (old)
                    {
                        strcpy(t, show_address(&old->from));
                    }
                    break;

                case ATTR_TOK_ODA:
                    if (old)
                    {
                        strcpy(t, show_address(&old->to));
                    }
                    break;

                case ATTR_TOK_FNA:
                    if (m->isfrom)
                    {
                        strcpy(t, m->isfrom);
                    }
                    break;

                case ATTR_TOK_FFN:
                    if (m->isfrom)
                    {
                        strcpy(t, firstname(m->isfrom));
                    }
                    break;

                case ATTR_TOK_FAD:
                    strcpy(t, show_address(&m->from));
                    break;

                case ATTR_TOK_TNA:
                    if (m->isto)
                    {
                        strcpy(t, m->isto);
                    }
                    break;

                case ATTR_TOK_TFN:
                    if (m->isto)
                    {
                        strcpy(t, firstname(m->isto));
                    }
                    break;

                case ATTR_TOK_TAD:
                    strcpy(t, show_address(&m->to));
                    break;

                case ATTR_TOK_SUB:
                    strcpy(t, m->subj);
                    break;

                case ATTR_TOK_UNA:
                    if (ST->username)
                    {
                        strcpy(t, ST->username);
                    }
                    break;

                case ATTR_TOK_UFN:
                    if (ST->username)
                    {
                        strcpy(t, firstname(ST->username));
                    }
                    break;

                case ATTR_TOK_UAD:
                    strcpy(t, show_address(&CurArea.addr));
                    break;

                case ATTR_TOK_CEH:
                    strcpy(t, CurArea.tag);
                    break;

                case ATTR_TOK_OEH:
                    if (olda != -1)
                    {
                        strcpy(t, arealist[olda].tag);
                    }
                    break;

                case ATTR_TOK_IMS:
                    if (old != NULL)
                    {
                        sprintf(t, "%04d-%02d-%02d", tm->tm_year + 1900,
                          tm->tm_mon + 1, tm->tm_mday);
                    }
                    break;

                case ATTR_TOK_INO:
                    sprintf(t, "%04d-%02d-%02d", now.tm_year + 1900,
                      now.tm_mon + 1, now.tm_mday);
                    break;

                case ATTR_TOK_CMS:
                    if (old != NULL)
                    {
                        sprintf(t, "%04d", tm->tm_year + 1900);
                    }
                    break;

                case ATTR_TOK_CNO:
                    sprintf(t, "%04d", now.tm_year + 1900);
                    break;

                default:
                    break;
                }
                break;
            }
            t = work + strlen(work);
            format += 3;
        }
        else
        {
            *t++ = *format++;
        }
    }
    return xstrdup(work);
}
