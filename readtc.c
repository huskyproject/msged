/* Routines for querying termcap information on UNIX.
   These routines are shared between the alternative screen
   modules curses.c and ansi.c
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "readtc.h"

/* the pseudographic character codes */
#define ASCII_SPECIALS "|-****|-*****><!*^v&*"
char *tt_specials = ASCII_SPECIALS;
char *tt_alternate_start = NULL;
char *tt_alternate_end = NULL;

/* cursor shape control */
char *tt_showcursor = NULL;
char *tt_hidecursor = NULL;

#ifdef UNIX

#define DEC_SPECIALS   "xqlkmjxqlkmja>< p^V&*"
#define LINUX_SPECIALS   "\225\232\226\234\223\231\205\212\206\214\203\211*><!*^V&*"
/* "\170\161\154\153\155\152\170\161\154\153\155\152\141><\160^^V&*" */


/* No chance to guess which UNIX uses term.h and which termcap.h, so we
   just have to use our own prototype. I know this is ugly ...
   #include <termcap.h>
   #include <term.h>
*/

int tgetent(char *, const char *);
char *tgetstr(char *, char **);   

static char termbuf[4096];
static char termresult[4096];

static char *transform_acs(char *template, char *terminfo)
{
    int i;
    static char buffer[22];
    char *p;

    for (i = 0; i < sizeof(buffer) && i < strlen(template); i++)
    {
        p = islower(template[i]) ? strchr(terminfo, template[i]) : NULL;
        if (p != NULL && p[0] && p[1])
        {
            buffer[i] = p[1];
        }
        else
        {
            buffer[i] = ASCII_SPECIALS[i];
        }
    }
    return buffer;
}

void query_termcap(int what)
{
    char *termname;
    char *area;
    char *acs;

    termname = getenv("TERM");

    /* READ INFORMATION ON PSEUDO GRAPHICS CHARACTERS */

    if ((termname != NULL) && (what & QUERY_ALTCHARSET))
    {
        /* The Linux people are idiots, can't even get their termcap right */
        if (!strcmp(termname, "linux") ||
            !strcmp(termname, "linux-lat") ||
            !strcmp(termname, "linux console"))
        {
            tt_specials = LINUX_SPECIALS;
            tt_alternate_start = "";
            tt_alternate_end = "";
            tt_showcursor = "\033[25h";
            tt_hidecursor = "\033[25l";
        }
        else if (tgetent(termbuf, termname) != 0)
        {
            area = termresult;
            tt_alternate_start = tgetstr("as", &area);
            tt_alternate_end = tgetstr("ae", &area);
            acs = tgetstr("ac", &area);
            tt_showcursor = tgetstr("ve", &area);
            tt_hidecursor = tgetstr("vi", &area);

            if (acs != NULL)
            {
                tt_specials = transform_acs(DEC_SPECIALS, acs);
                if (tt_alternate_start == NULL)
                {
                    tt_alternate_start = "";
                }
                if (tt_alternate_end == NULL)
                {
                    tt_alternate_end = "";
                }
            }
            else if (tt_alternate_start != NULL && tt_alternate_end != NULL)
            {
                tt_specials = DEC_SPECIALS;
            } 
            else
            {
                tt_alternate_start = NULL;
                tt_alternate_end = NULL;
            }
        }
    } else
    {
        tt_specials = ASCII_SPECIALS;
        tt_alternate_start = NULL;
        tt_alternate_end = NULL;
    }
}
#endif
