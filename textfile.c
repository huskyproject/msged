/*
 *  TEXTFILE.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Handles import and export of text files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(PACIFIC)
#include <unixio.h>
#elif defined(SASC)
#include <fcntl.h>
#elif defined(UNIX) || defined(__DJGPP__)
#include <unistd.h>
#else
#include <io.h>
#endif

#if defined(UNIX) || defined(__EMX__)
#define HAVE_POPEN
#endif

#include "addr.h"
#include "config.h"
#include "nedit.h"
#include "msged.h"
#include "winsys.h"
#include "menu.h"
#include "dialogs.h"
#include "memextra.h"
#include "strextra.h"
#include "wrap.h"
#include "date.h"
#include "nshow.h"
#include "quote.h"
#include "textfile.h"

#define TEXTLEN 2048

void import(LINE * l)
{
    char *fn = xmalloc(PATHLEN + 1);
    static char fname[PATHLEN];
    static char line[TEXTLEN];
    char *temp;
    FILE *fp;
    LINE *n;
    int ret;

    ret = GetString(" Import File ", "Name of file to import?", fn, PATHLEN);

    TTCurSet(1);

    if (!ret)
    {
        xfree(fn);
        return;
    }

    fn = shell_expand(fn);

    fp = fopen(fn, "r");
    if (fp != NULL)
    {
        if (SW->importfn)
        {
            if (strstr(fn, "\\") || strstr(fn, "/"))
            {
                temp = getfilename(fn);
            }
            else
            {
                temp = strupr(fn);
            }
            strcpy(fname, temp);
            sprintf(line, "   ----- %s begins -----\n", fname);
            l->text = strdup(line);
        }

        while (fgets(line, sizeof line, fp) != NULL)
        {
            if (l->text != NULL)
            {
                n = xcalloc(1, sizeof *n);
                n->prev = l;
                n->next = l->next;

                if (n->next != NULL)
                {
                    n->next->prev = n;
                }

                l->next = n;
                l = n;
            }
            else
            {
                n = l;
            }

            if (softcrxlat)
            {
                char *p;
                p = strchr(line, 0x8d);
                while (p != NULL)
                {
                    *p++ = softcrxlat;
                    p = strchr(p, 0x8d);
                }
            }

            n->text = strdup(line);
            if (strlen(n->text) > (size_t) SW->rm)
            {
                l = n->next;
                wrap(n, 1, maxy, SW->rm);

                if (!l)
                {
                    while (n->next)
                    {
                        n = n->next;
                    }
                }
                else
                {
                    n = l->prev;
                }

                l = n;
            }
        }

        if (SW->importfn)
        {
            if (l->text != NULL)
            {
                n = xcalloc(1, sizeof *n);
                n->prev = l;
                n->next = l->next;

                if (n->next != NULL)
                {
                    n->next->prev = n;
                }

                l->next = n;
                l = n;
            }
            sprintf(line, "   ----- %s ends -----\n", fname);
            l->text = strdup(line);
        }
        fclose(fp);
    }
    xfree(fn);
}

/*
 *  getfilename; Used to isolate the filename on an import so that
 *  it does not import directories into the message. Used only by
 *  import().
 */

char *getfilename(char *buf)
{
    int x, y;
    char tempch[2];
    static char filename[PATHLEN + 1] = "";

    for (x = 0; x <= strlen(buf); x++)
    {
        if (buf[strlen(buf) - x] == '\\' || buf[strlen(buf) - x] == '/')
        {
            break;
        }
    }

    for (y = strlen(buf) - x + 1; y <= strlen(buf); y++)
    {
        tempch[0] = buf[y];
        tempch[1] = '\0';
        if (y == strlen(buf) - x + 1)
        {
            strcpy(filename, tempch);
        }
        else
        {
            strcat(filename, tempch);
        }
    }

    return filename;
}

void export(LINE * f)
{
    FILE *fp;
    char *fn = xmalloc(PATHLEN + 1);
    int ret;
    int use_pclose = 0;

    if (ST->outfile)
    {
        strcpy(fn, ST->outfile);
    }
    else
    {
        strcpy(fn, "");
    }

    ret = GetString(" Export File ", "Name of file to export?", fn, PATHLEN);

    if (!ret)
    {
        xfree(fn);
        return;
    }

    release(ST->outfile);
    ST->outfile = strdup(fn);

    fn = shell_expand(fn);

    if (*fn == '+')
    {
        fp = fopen(fn + 1, "a");
    }
#ifdef HAVE_POPEN
    else if (*fn == '|')
    {
        fp = popen(fn + 1, "w");
        use_pclose = 1;
    }
#endif    
    else
    {
        fp = fopen(fn, "w");
    }

    if (fp == NULL)
    {
        ChoiceBox("", "WARNING: Error opening file", "  Ok  ", NULL,  NULL);
        xfree(fn);
        return;
    }

    while (f != NULL)
    {
        if (f->text && (*(f->text) != '\01' || SW->shownotes))
        {
            fputs(f->text, fp);
            if (strchr(f->text, '\n') == NULL)
            {
                fputc('\n', fp);
            }
        }
        f = f->next;
    }

    /* if output is to printer output a formfeed */
    if (isatty(fileno(fp)))
    {
        fputc(12, fp);
    }

#ifdef HAVE_POPEN
    if (use_pclose)
    {
        pclose(fp);
    }
    else
#endif        
    {
        fclose(fp);
    }

    xfree(fn);
    TTCurSet(1);
}

void writetxt(void)
{
    static char *modes[] = {"Text", "Quote", "Msged", NULL};
    static char *ovr[] = {"Append", "Replace", NULL};
    LINE *f = message->text;
    char *fn = xmalloc(PATHLEN + 1);
    static int mode = 0;
    int ret;
    FILE *fp;
    char *s;
    int use_pclose = 0;

    if (ST->outfile)
    {
        strcpy(fn, ST->outfile);
    }
    else
    {
        strcpy(fn, "");
    }

    ret = GetString(" Export File ", "Name of file to export?", fn, PATHLEN);

    if (!ret)
    {
        xfree(fn);
        return;
    }

    release(ST->outfile);
    ST->outfile = strdup(fn);

    fn = shell_expand(fn);

    s = strchr(fn, ',');
    if (s != NULL)
    {
        *s++ = '\0';
    }

    if (s && *s == 't')
    {
        mode = 0;
    }
    else if (s && *s == 'q')
    {
        mode = 1;
    }
    else if ((s && *s == 'm') || !s)
    {
        mode = 2;
    }

    if (*fn == '?')
    {
        mode = DoMenu(61, 2, 69, 4, modes, mode, SELBOX_WRTMODE, "");
        if (mode == -1)
        {
            mode = 0;
            xfree(fn);
            return;
        }
    }

    if (*fn == '+')
    {
        fp = fopen(fn + 1, "a");
    }
#ifdef HAVE_POPEN
    else if (*fn == '|')
    {
        fp = popen(fn + 1, "w");
        use_pclose = 1;
    }
#endif    
    else if (*fn == '?')
    {
        fp = fopen(fn + 1, "r");
        if (fp == NULL)
        {
            fp = fopen(fn + 1, "w");
        }
        else if (isatty(fileno(fp)))
        {
            fclose(fp);
            fp = fopen(fn + 1, "w");
            mode = 0;
        }
        else
        {
            ret = DoMenu(61, 2, 69, 3, ovr, 0, SELBOX_WRTOVER, "");
            if (ret == -1)
            {
                xfree(fn);
                return;
            }

            fclose(fp);
            if (ret)
            {
                fp = fopen(fn + 1, "w");
            }
            else
            {
                fp = fopen(fn + 1, "a");
            }
        }
    }
    else
    {
        fp = fopen(fn, "w");
    }

    if (fp == NULL)
    {
        ChoiceBox("", "WARNING: Error opening file", "  Ok  ", NULL,  NULL);
        xfree(fn);
        return;
    }

    if (mode == 0 || (s != NULL && strchr(s, 't') != NULL))
    {
        fprintf(fp, "Date:   %s", itime(message->timestamp));
        fprintf(fp, "\nFrom:   %s", message->isfrom ? message->isfrom : "");
        fprintf(fp, " of %s", show_address(&message->from));
        fprintf(fp, "\nTo:     %s", message->isto ? message->isto : "");

        if (CurArea.netmail)
        {
            fprintf(fp, " of %s", show_address(&message->to));
        }

        fprintf(fp, "\nSubj:   %s", message->subj ? message->subj : "");

        MakeMsgAttrs(fn, &message->attrib, message->scanned, message->times_read);

        fprintf(fp, "\nAttr:   %s", fn);
        fprintf(fp, "\nConf:   %-30s", CurArea.description);
        fprintf(fp, "\n\n");
    }

    if (mode == 1 || (s != NULL && strchr(s, 'q') != NULL))
    {
        makequote(message->text, message->isfrom);
        f = message->text;
    }

    while (f != NULL)
    {
        if (f->text && (*(f->text) != '\01' || SW->shownotes))
        {
            fputs(f->text, fp);
            if (!strchr(f->text, '\n') && (!mode || mode == 1))
            {
                fprintf(fp, "\n");
            }
        }
        f = f->next;
    }

    /* if output is to printer output a formfeed */
    if (isatty(fileno(fp)))
    {
        fputc(12, fp);
    }

#ifdef HAVE_POPEN
    if (use_pclose)
    {
        pclose(fp);
    }
    else
#endif        
    {
        fclose(fp);
    }
    if (mode == 1 || (s != NULL && strchr(s, 'q') != NULL))
    {
        /* reread the old message text */
        set_area(SW->area);
    }
    xfree(fn);
}






