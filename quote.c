/*
 *  QUOTE.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Contains routines relevant to detection of quotes and quoting.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "addr.h"
#include "nedit.h"
#include "memextra.h"
#include "strextra.h"
#include "mctype.h"
#include "msged.h"
#include "wrap.h"
#include "quote.h"

#define TEXTLEN 128
#define INPLEN 60

/* is the passed line a quote? */

int is_quote(char *text)
{
    char *s = text, *c = text;

    while (*s && s && s - text < 12)
    {
        if (*s == '>')
        {
            break;
        }
        s++;
    }

    if (*s != '>')
    {
        return FALSE;
    }

    while (c && *c && c < s)
    {
        switch (*c)
        {
        case '<':
            return FALSE;

        case ' ':
        case ':':
        case '-':
        case '@':
            c++;
            continue;

        default:
            if (!m_isalnum(*c))
            {
                return FALSE;
            }
            c++;
            break;
        }
    }

    return TRUE;
}

int is_same_quote(LINE * l, LINE * o)
{
    char *s, *c;
    int lenl, leno;

    lenl = strlen(l->text);
    leno = strlen(o->text);

    if (l->quote && o->quote)
    {
        if (lenl >= 12)
        {
            s = l->text + 11;
        }
        else
        {
            s = l->text + lenl - 1;
        }

        if (leno >= 12)
        {
            c = o->text + 11;
        }
        else
        {
            c = o->text + leno - 1;
        }

        while (*s && *s != '>')
        {
            s--;
        }

        while (*c && *c != '>')
        {
            c--;
        }

        if (*s == '>' && *(s + 1))
        {
            s++;
        }

        if (*c == '>' && *(c + 1))
        {
            c++;
        }

        if (!strncmpi(l->text, o->text, (size_t) max((s - l->text),
          c - o->text)))
        {
            return TRUE;
        }

        return FALSE;
    }

    return FALSE;
}

int is_blank(LINE * l)
{
    char *s;
    int len;

    if (!l || !l->text || *l->text == '\n' || *l->text == '\0')
    {
        return TRUE;
    }

    len = strlen(l->text);
    if (l->quote)
    {
        if (len >= 12)
        {
            s = l->text + 11;
        }
        else
        {
            s = l->text + len - 1;
        }

        while (*s && *s != '>')
        {
            s--;
        }

        if (*s == '>' && *(s + 1))
        {
            s++;
        }

        while (*s && m_isspace(*s))
        {
            s++;
        }

        if (*s == '\0')
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        s = l->text;
        while (*s && m_isspace(*s))
        {
            s++;
        }

        if (*s == '\0')
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

char *replace_noise(char *text)
{
    char *s = text;
    char *c;

    /* replace nasty bits */

    if (*text == '\01')
    {
        *text = '@';
    }

    if (!SW->soteot)
    {
        if (!strncmp(text, " * Ori", 6))
        {
            *(text + 1) = '+';
        }

        if (!strncmp(text, "---", 3) && strncmp(text, "----", 4))
        {
            *(text + 1) = '+';
        }

        if (!strncmp(text, "SEEN-BY:", 8))
        {
            *(text + 4) = '+';
        }
    }

    /* strip leading spaces unless hard-quoting */

    if (!SW->hardquote)
    {
        while (*s && m_isspace(*s))
        {
            s++;
        }
    }

    if (!*s)
    {
        return text;
    }

    c = xstrdup(s);
    release(text);

    return c;
}


LINE *makequote(LINE * l, char *isfrom)
{
    int i;
    char *qs;
    char *s, c;
    LINE *t, *o;
    char initial[10];
    char line2[256];

    if (l == NULL)
    {
        return l;
    }

    i = 0;
    s = isfrom;

    while (s && *s && i < 10)
    {
        while (*s && m_isspace(*s))
        {
            s++;
        }
	if (!m_isalnum(*s)) s++; /* mtt */
        initial[i++] = *s;
        while (*s && !m_isspace(*s))
        {
            s++;
        }
    }

    initial[i] = '\0';

    s = strchr(ST->quotestr, '&');
    if (s == NULL)
    {
        qs = xstrdup(ST->quotestr);
    }
    else
    {
        qs = xmalloc(strlen(ST->quotestr) + strlen(initial) + 1);
        *s = '\0';
        strcpy(qs, ST->quotestr);
        strcat(qs, initial);
        strcat(qs, s + 1);
        *s = '&';
    }

    s = qs;
    s = strchr(s, '^');
    while (s != NULL)
    {
        if (initial[0])
        {
            *s = initial[0];
        }
        else
        {
            strdel(s, 1);
        }
        s = strchr(s, '^');
    }

    s = qs;
    s = strchr(s, '*');
    while (s != NULL)
    {
        if (initial[1])
        {
            *s = initial[1];
        }
        else
        {
            strdel(s, 1);
        }
        s = strchr(s, '*');
    }

    t = l;

    while (t)
    {
        t->hide = t->block = 0;
        if (!t->text || strlen(t->text) == 0)
        {
            release(t->text);
            t->text = xstrdup("\n");
            t = t->next;
            continue;
        }
        if (!t->quote)
        {
            if (SW->hardquote)
            {
                wrap(t, 1, maxy, SW->qm - strlen(qs));
            }
            if (strchr(t->text, '\n') != NULL)
            {
                /* don't quote a blank line */
                if (*t->text == ' ')
                {
                    char *p;

                    p = t->text;
                    while (*p == ' ')
                    {
                        p++;
                    }
                    if (*p == '\n' && *(p + 1) == '\0')
                    {
                        strcpy(t->text, "\n");
                    }
                }
                if (*t->text != '\n' && *t->text)
                {
                    t->hide = t->block = 0;
                    t->text = replace_noise(t->text);
                    sprintf(line2, "%s%s", qs, t->text);
                    release(t->text);
                    t->text = xstrdup(line2);
                    t->quote = 1;
                }
            }
            else
            {
                if (!SW->hardquote)
                {
                    wrap(t, 1, maxy, SW->qm - strlen(qs));
                }
                while (t != NULL && strchr(t->text, '\n') == NULL)
                {
                    t->hide = t->block = 0;
                    t->text = replace_noise(t->text);
                    sprintf(line2, "%s%s\n", qs, t->text);
                    release(t->text);
                    t->text = xstrdup(line2);
                    t->quote = 1;
                    t = t->next;
                }
                continue;
            }
        }
        else
        {
            if (SW->qquote)
            {
                s = strchr(t->text, '>');
                if (s)
                {
                    if (s - t->text <= 11)
                    {
                        c = *s;
                        *s = '\0';
                        strcpy(line2, t->text);
                        strcat(line2, ">");
                        *s = c;
                        strcat(line2, s);
                    }
                    else
                    {
                        strcpy(line2, t->text);
                    }
                }
                else
                {
                    strcpy(line2, t->text);
                }
            }
            else
            {
                if (t->text)
                {
                    strcpy(line2, t->text);
                }
                else
                {
                    strcpy(line2, "");
                }
            }
            release(t->text);

            if (!strchr(line2, '\n'))
            {
                strcat(line2, "\n");
            }

            if (*line2 == ' ')
            {
                t->text = xstrdup(line2 + 1);
            }
            else
            {
                t->text = xstrdup(line2);
            }
        }
        t = t->next;
    }

    xfree(qs);

    t = l;  /* returns the last line of the msg */
    while (t->next != NULL)
    {
        if (strlen(t->text) > SW->qm)
        {
            wrap(t, 1, maxy, SW->qm);
            t = t->next;
        }
        else
        {
            t = t->next;
        }
    }

    t = l;

    while (t)
    {
        if (strchr(t->text, '\n') && t->prev && !strchr(t->prev->text, '\n'))
        {
            if (t->next && !is_blank(t->next) && is_same_quote(t, t->next))
            {
                *((char *)strchr(t->text, '\n')) = '\0';
            }

            wrap(t, 1, maxy, SW->qm);
            o = t;

            while (t && !strchr(t->text, '\n'))
            {
                t = t->next;
            }

            if (t == o)
            {
                t = t->next;
            }
        }
        else
        {
            t = t->next;
        }
    }

    t = l;

    /* make sure the quotes are all terminated with '\n' */

    while (t->next)
    {
        if (t->text && strlen(t->text) > 1 && *ST->quotestr == ' ' && *(t->text) != ' ')
        {
            strcpy(line2, " ");
            strcat(line2, t->text);
            xfree(t->text);
            t->text = xstrdup(line2);
        }
        if (t->text && !strchr(t->text, '\n') && t->quote)
        {
            strcpy(line2, t->text);
            strcat(line2, "\n");
            xfree(t->text);
            t->text = xstrdup(line2);
        }
        t = t->next;
    }

    return t;  /* last line of message */
}
