/*
 *  TEXTFILE.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Changes by Frank Adams (dialog boxes etc.).
 *  Released to the public domain.
 *
 *  Handles import and export of text files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#if defined(PACIFIC)
#include <unixio.h>
#elif defined(SASC)
#include <fcntl.h>
#elif defined(UNIX) || defined(__DJGPP__)
#include <unistd.h>
#else
#include <io.h>
#endif

#if defined(UNIX) || defined(__EMX__) || defined(__DJGPP__)
#define HAVE_POPEN
#endif

#ifdef __MINGW32__
#define isatty _isatty
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
#include "filedlg.h"

#define TEXTLEN 2048

void import(LINE * l)
{
    char *fn;
    static char fname[PATHLEN];
    static char line[TEXTLEN];
    char *temp;
    FILE *fp;
    LINE *n;
    int ret;

    fn  = xmalloc(PATHLEN + 1);

    if (ST->infile)
    {
        strcpy(fn, ST->infile);
    }
    else
    {
        fn[0] = '\0';
    }

    ret = FileDialog(fn, "Select a File to Import");

    release(ST->infile);
    ST->infile = xstrdup(fn);

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


void export_text(msg *mesg, LINE *line)
{
    LINE *l;
    FILE *f;
    int destination = 0, mode = 0, ret = 0, x1, x2;
    char fn[2048];
    int (*closefunc)(FILE *) = NULL;

    static char *destinations[] = {
        "Write to File",
        "Print",
        "Pipe into external Program",
        "Cancel",
        NULL
    };
    static char *modes[] = {
        "As Plain Text",
/*        "As Quoted Text", */
        "Binary (for re-importing into Msged)",
        "Cancel",
        NULL
    };

    /* Find out if we want to print a message or only a piece of text */
    if (mesg != NULL)
    {
        l = mesg->text;
    }
    else
    {
        l = line;
    }

    /* Select the export destination */
    
    x1 = maxx/2 - 19; if (x1 < 0) x1 = 0;
    x2 = x1 + 38;
    destination = DoMenu(x1, 10, x2, 13, destinations, 0,
                         SELBOX_WRTMODE, "Export Destination");

    switch (destination)
    {
    case 0:                     /* as file */
        if (ST->outfile)
        {
            strcpy(fn, ST->outfile);
        }
        else
        {
            strcpy(fn, "");
        }
        if ((ret = FileDialog(fn, "Select File to Write to")) <= 0)
        {
            return;
        }
        xfree(ST->outfile);
        ST->outfile = strdup(fn);
        break;

    case 1:                     /* print */
#ifdef UNIX
        destination = 2;
        if (ST->printer != NULL)
        {
            sprintf(fn, "lpr %s -", ST->printer);
        }
        else
        {
            sprintf(fn, "lpr -");
        }
        break;
#else
        if (ST->printer != NULL)
        {
            strcpy(fn, ST->printer);
        }
        else
        {
            strcpy(fn, "PRN");
        }
#endif
        break;

    case 2:                     /* pipe */
#ifdef HAVE_POPEN
        if (!GetString("Pipe Text To External Program", "Command Line", fn,
                       2047))
        {
            return;
        }
#else
        ChoiceBox("Not Implemented",
                  "This version of Msged does not support piping.",
                  "OK", NULL, NULL);
        return;
#endif        
        break;

    default:                     /* cancel */
        return;
    }


    /* select the output mode, if a whole msg is exported */

    if (mesg != NULL && destination != 1)
    {
        mode = DoMenu(x1, 10, x2, 13, modes, 0,
                      SELBOX_WRTMODE, "Export Mode");
        if (mode < 0 || mode > 1)
        {
            return;
        }
    }


    /* Try to open the output medium */

    switch (destination)
    {
    case 0:                     /* file */
    case 1:
        closefunc = fclose;
        if (((f = fopen(fn, "r")) != NULL) && (!isatty(fileno(f))))
        {
            ret = ChoiceBox("Attention", "File already exists!",
                            "Append", "Overwrite", "Cancel");
                
            switch(ret)
            {
            case ID_ONE:             /* append */
                fclose(f);
                f = fopen(fn, "a+");
                if (f != NULL) fseek(f, 0, SEEK_END);
                break;
            case ID_TWO:
                fclose(f);
                f = fopen(fn, "w");
                break;
            case ID_THREE:
                fclose(f);
                return;
            default:
                abort();
            }
        }
        else
        {
            f = fopen(fn, "w");
        }

        if (f == NULL)
        {
            ChoiceBox("Error", "Cannot write to file!", "OK", NULL, NULL);
            return;
        }
        break;

    case 2:
#ifdef HAVE_POPEN        
        closefunc = pclose;
        f = popen(fn, "w");
        if (f == NULL)
        {
            ChoiceBox("Error", "Cannot execute program!", "OK", NULL, NULL);
            return;
        }
#endif
        break;
    }

    /* TODO: Quote Mode */

    /* write a header, if possible and desired */

    if (mode == 0 && mesg != NULL)
    {
        fprintf(f, "Date:   %s", itime(mesg->timestamp));
        fprintf(f, "\nFrom:   %s", mesg->isfrom ? mesg->isfrom : "");
        fprintf(f, " of %s", show_address(&mesg->from));
        fprintf(f, "\nTo:     %s", mesg->isto ? mesg->isto : "");

        if (CurArea.netmail)
        {
            fprintf(f, " of %s", show_address(&mesg->to));
        }

        fprintf(f, "\nSubj:   %s", mesg->subj ? mesg->subj : "");

        MakeMsgAttrs(fn, &mesg->attrib, mesg->scanned, mesg->times_read);

        fprintf(f, "\nAttr:   %s", fn);
        fprintf(f, "\nConf:   %-30s", CurArea.description);
        fprintf(f, "\n\n");
    }

    /* write the message text */

    while (l != NULL)
    {
        if (l->text && (*(l->text) != '\01' || SW->shownotes))
        {
            fputs(l->text, f);
            if (!strchr(l->text, '\n') && (mode != 1))
            {
                fprintf(f, "\n");
            }
        }
        l = l->next;
    }

    /* if output is to printer output a formfeed */
    if (isatty(fileno(f)))
    {
        fputc(12, f);
    }

    (*closefunc)(f);
}

/* called when exporting an anchor block from the editor */
void export(LINE * f)
{

    export_text(NULL, f);
}

/* called when exporting while in reading mode */
void writetxt(void)
{
    export_text(message, NULL);
}
