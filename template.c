/*
 *  TEMPLATE.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Handles the creation of messages from a template.
 */

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "quote.h"
#include "date.h"
#include "memextra.h"
#include "readmail.h"  /* GetOrigin */
#include "template.h"
#include "version.h"

#define TEXTLEN 250

static LINE* addline(LINE* ln,  char *l);

static char **parse_words_to_array(char *string, int nmembers)
{
    char *cp = strtok(string, " \t\r\n");
    int n=0;
    char **whereto = xmalloc(nmembers * sizeof(char **));

    while (cp != NULL && n < nmembers )
    {
        whereto[n++] = cp;
        cp = strtok(NULL, " \t\r\n");
    }

    if (n < nmembers)
    {
        free(whereto);
        return NULL;
    }
    if (cp != NULL)
    {
        return whereto;
    }
    return whereto;
}

int MakeTemplateMsg(msg * m, msg * oldmsg, int olda, int type)
{
    LINE *ln = NULL;
    FILE *fp;
    char buf[TEXTLEN], buf2[TEXTLEN];
    char *wdaybuf=NULL, *monthbuf=NULL;
    char **use_month = NULL;
    char **use_day   = NULL;
    char *l;
    int blankline;

    if (ST->template)
    {
        fp = fopen(ST->template, "r");
    }

    if (!ST->template || fp == NULL)
    {
        if (m->text && (type & MT_QUO))
        {
            ln = m->text;
            makequote(ln, oldmsg != NULL ? oldmsg->isfrom : m->isfrom);
        }
        return 0;
    }

    while (fgets(buf, TEXTLEN - 1, fp))
    {
        if (buf[0] == ';')
        {
            continue;
        }

        if (buf[0] == '@' && buf[1] && buf[1] != '@')
        {
            switch (tolower(buf[1]))
            {
            case 'w':
                release(wdaybuf);
                wdaybuf = xstrdup(buf+2);
                use_day = parse_words_to_array(wdaybuf, 7);
                continue;

            case 'o':
                release(monthbuf);
                monthbuf = xstrdup(buf+2);
                use_month = parse_words_to_array(monthbuf, 12);
                continue;
                
            case 'f':
                if (!(type & MT_FOR))
                {
                    continue;
                }
                break;

            case 'l':
                if (!(type & MT_FOL))
                {
                    continue;
                }
                break;

            case 'r':
                if (!(type & MT_RED))
                {
                    continue;
                }
                break;

            case 'a':
                if (!(type & MT_ARC))
                {
                    continue;
                }
                break;

            case 'q':
                if (!(type & MT_QUO))
                {
                    continue;
                }
                break;

            case 'n':
                if (!(type & MT_NEW))
                {
                    continue;
                }
                break;

            case 'm':
                if (ln)
                {
                    /* assign msg to follow */
                    ln->next = m->text;
                    if (ln->next)
                    {
                        ln->next->prev = ln;
                        ln = ln->next;
                    }
                }
                else
                {
                    ln = m->text;
                }
                if (!(type & MT_QUO))
                {
                    /* don't want a quote */
                    while (ln != NULL && ln->next != NULL)
                    {
                        ln = ln->next;
                    }
                }
                else
                {
                    /* we do want a quote */
                    makequote(ln, oldmsg != NULL ? oldmsg->isfrom : m->isfrom);

                    /* just in case we didn't get the beginning */
                    ln = m->text;
                    while (ln != NULL && ln->next != NULL)
                    {
                        ln = ln->next;
                    }
                }
                continue;

            default:
                /* break on things we don't know */
                break;
            }

            if (buf[2])
            {
                /* could be a blank line... */
                l = attrib_line(m, oldmsg, olda, buf + 2, use_day, use_month);
            }
            else
            {
                l = xstrdup("\n");
            }
        }
        else
        {
            l = attrib_line(m, oldmsg, olda, buf, use_day, use_month);
        }

        ln = addline(ln, l);

    }

    fclose(fp);

    /* add tearline and origin if appropriate and wished */

    if (CurArea.echomail && !(type & MT_RED))
    {
        if (ln != NULL)
        {
            blankline = ( *(ln->text) == '\n');
        }
        else
        {
            blankline = 0;
        }

        if (!(type & MT_FOR))
        {
            if (SW->usetearlines  && SW->useoriginlines  && !blankline &&
                (SW->edittearlines || SW->editoriginlines))
            {
                strcpy(buf, "\n");
                ln = addline(ln, xstrdup(buf));
            }
            
            if (SW->usetearlines && SW->edittearlines)
            {
                /* add the tearline */
                
                if (SW->usepid)
                {
                    sprintf(buf, "---\n");
                }
                else
                {
                    sprintf(buf, "--- %s %s\n", PROG, VERNUM VERPATCH);
                }
                ln = addline(ln, xstrdup(buf));
            }
            
            if (SW->useoriginlines && SW->editoriginlines)
            {
                /* add the origin line */
                
                GetOrigin(buf2);
                sprintf(buf, " * Origin: %s (%s)\n", buf2,
                        SW->domainorigin ? show_address(&(m->from)) :
                        show_4d(&(m->from)));
                
                ln = addline(ln, xstrdup(buf));
            }
        }
    }


    /* we've finished; assign it to the beginning of the msg */

    if (m->text == NULL && ln != NULL)
    {
        /* find beginning of msg and assign it */

        while (ln->prev)
        {
            ln = ln->prev;
        }
        m->text = ln;
    }

    ln = m->text;
    while (ln && ln->prev)
    {
        ln = ln->prev;
    }
    m->text = ln;

    release(monthbuf); release(wdaybuf);
    release(use_month); release(use_day);

    return 0;
}

static LINE* addline(LINE* ln,  char *l)
{
    LINE *n;

    n = xcalloc(1, sizeof *n);

    n->templt = 1;  /* mark as a template line */

    if (ln)
    {
        n->next = ln->next;
    }

    n->prev = ln;

    if (ln)
    {
        ln->next = n;
    }

    if (n->next)
    {
        n->next->prev = n;
    }

    n->text = l;
    ln = n;

    return ln;
}

