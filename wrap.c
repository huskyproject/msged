/*
 *  WRAP.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Editor for Msged.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "editmail.h"
#include "memextra.h"
#include "winsys.h"
#include "nshow.h"
#include "unused.h"
#include "readmail.h"
#include "textfile.h"
#include "menu.h"
#include "dialogs.h"
#include "main.h"
#include "specch.h"
#include "screen.h"
#include "version.h"
#include "makemsgn.h"
#include "keys.h"
#include "misc.h"
#include "help.h"
#include "dosmisc.h"
#include "wrap.h"
#include "mctype.h"

static LINE *current;           /* current line */
static LINE *pagetop;           /* top line of page */
static LINE *msgtop;            /* top line of file */
static LINE *udel;              /* deleted line buffer */
static LINE *clip;              /* for blocks of text */
static char line_buf[255];      /* buffer for the current line */
static int x = 1;               /* x coordinate, 1 based */
static int y = 1;               /* y coordinate, 1 based */
static int currline = 1;        /* current line of msg, 1 based */
static int ed_miny = 1;         /* logical min y value */
static int ed_maxy = 1;         /* logical max y value */
static int edmaxy;              /* real max y value */
static int edminy;              /* real min y value */
static int quote_len = 14;      /* length to look for quote */
static int done;                /* finished editing? */
static int insert = 1;          /* insert = on ? */
static int blocking;            /* block on? */
static msg *messg;              /* message being edited */

/* This function sets the current right and quote margins, taking
   into account both the actual window size and the settings that the
   user has desired. */

void adapt_margins(void)
{
    SW->rm = maxx - 1;
    if (SW->rm > SW->orgrm)
    {
        SW->rm = SW->orgrm;
    }
    
    SW->qm = SW->rm - strlen(ST->quotestr);
    if (SW->qm > SW->orgqm)
    {
        SW->qm = SW->orgqm;
    }
    if (SW->rm < 1)
    {
        SW->rm = 1;
    }
    if (SW->qm < 1)
    {
        SW->qm = 1;
    }
}        

static void zap_quotes(void)
{
    while (current->next != NULL && (current->quote || current->text[0] == 10))
    {
        delete_line();
    }
}

static void rotate(void)
{
    rot13 = (rot13 + 1) % 3;
}

static void nada(void)
{
}

char *e_getbind(unsigned int key)
{
    unsigned int i = 0;
    void (*action) (void);

    if (key & 0xff)
    {
        action = editckeys[key & 0xff];
    }
    else
    {
        action = editakeys[(key >> 8) & 0xff];
    }

    while ((editcmds[i].label != NULL) && (action != editcmds[i].action))
    {
        i++;
    }

    return editcmds[i].label;
}

char *e_getlabels(int i)
{
    return editcmds[i].label;
}

void e_assignkey(unsigned int key, char *label)
{
    unsigned int i = 0;

    while ((editcmds[i].label != NULL) && (strncmp(editcmds[i].label, label, strlen(editcmds[i].label)) != 0))
    {
        i++;
    }

    if (editcmds[i].label != NULL)
    {
        if (key & 0xff)
        {
            editckeys[key & 0xff] = editcmds[i].action;
        }
        else
        {
            editakeys[(key >> 8) & 0xff] = editcmds[i].action;
        }
    }
}

LINE *lineins(LINE * cl)
{
    LINE *nl;

    nl = xcalloc(1, sizeof *nl);

    if (cl == NULL)
    {
        return nl;
    }

    nl->next = cl;
    nl->prev = cl->prev;
    cl->prev = nl;

    if (nl->prev != NULL)
    {
        nl->prev->next = nl;
    }

    return nl;
}

/*
 *  Functions that are local to this file (excepting wrap and editmsg).
 */

/*
 *  EdPutLine(); Displays a line, translating the display type.
 */

static void EdPutLine(LINE * line, int y)
{
    line->quote = isquote(line->text);

    /* check line coords! */
    if (y <= ed_maxy)
    {
        PutLine(line, y + edminy);
    }
    else
    {
        ed_error("EdPutLine", "illegal coordinates - y = %d!", y);
    }
}

/*
 *  ScrollDown(); Scrolls down one line.
 */

void ScrollDown(int y1, int y2)
{
    if (y1 <= ed_maxy && y2 <= ed_maxy)
    {
        WndScroll(0, y1 + edminy, maxx - 1, y2 + edminy, 0);
    }
    else
    {
        ed_error("ScrollDown", "illegal coordinates - y1 = %d!, y2 = %d", y1, y2);
    }
}

/*
 *  ScrollUp(); Scrolls up one line.
 */

void ScrollUp(int y1, int y2)
{
    if (y1 <= ed_maxy && y2 <= ed_maxy)
    {
        WndScroll(0, y1 + edminy, maxx - 1, y2 + edminy, 1);
    }
    else
    {
        ed_error("ScrollUp", "illegal coordinates - y1 = %d!, y2 = %d", y1, y2);
    }
}

/*
 *  GotoXY(); Goes to a position on the screen.
 */

static void GotoXY(int x, int y)
{
    if (x >= 1 && x <= maxx && y <= ed_maxy && y >= ed_miny)
    {
        WndGotoXY(x - 1, y + edminy);
    }
}

/*
 *  RedrawPage(); Redraws the page, starting from the line passed and
 *  the y coord.
 */

void RedrawPage(LINE * start, int wherey)
{
    LINE *cur = start;
    static LINE blank = {"", 0, 0, 0, 0, 0, NULL, NULL};
    int cury = wherey;

    if (start != NULL && wherey <= ed_maxy)
    {
        while (cur != NULL && cury <= ed_maxy)
        {
            EdPutLine(cur, cury);
            cur = cur->next;
            cury++;
        }

        /*
         *  If we have lines left on the screen, clear them - simple to
         *  just write over them with blank lines.
         */

        if (cury <= ed_maxy)
        {
            while (cury <= ed_maxy)
            {
                EdPutLine(&blank, cury);
                cury++;
            }
        }
    }
}

/*
 *  InsertLine(); Inserts a line AFTER the current line.
 */

LINE *InsertLine(LINE * current)
{
    LINE *nl;

    nl = xcalloc(1, sizeof *nl);

    nl->next = current->next;
    nl->prev = current;
    current->next = nl;

    if (nl->next)
    {
        nl->next->prev = nl;
    }

    return nl;
}

/*
 *  UnmarkLineBuf(); Copies the line_buf to the current line structure,
 */

void UnmarkLineBuf(void)
{
    release(current->text);
    current->text = xstrdup(line_buf);
}

/*
 *  SetLineBuf(); Copies the current line text to the line_buf.
 */

void SetLineBuf(void)
{
    memset(line_buf, 0, sizeof line_buf);
    if (current->text)
    {
        strncpy(line_buf, current->text, sizeof line_buf);
        line_buf[(sizeof line_buf) - 1] = '\0';
    }
}

/*
 *  iswhspace(); Determines if this is a white space.
 */

int iswhspace(char c)
{
    if (c == ' ' || c == '\t')
    {
        return 1;
    }
    return 0;
}

/*
 *  trailspace(); Determines if the eol has a trailing space.
 */

int trailspace(char *text)
{
    if (text == NULL || strlen(text) == 0)
    {
        return 1;
    }

    if (*(text + strlen(text) - 1) == ' ' || *(text + strlen(text) - 1) == '\n')
    {
        return 1;
    }

    return 0;
}

/*
 *  isquote(); Determines if the line is a quote.
 */

int isquote(char *text)
{
    char *s;

    if (text == NULL || strlen(text) == 0)
    {
        return 0;
    }

    s = text;
    while (*s && (s - text) < quote_len)
    {
        if (*s == '>')
        {
            return 1;
        }

        if (*s == '<' || *s == '-' || *s == '=')
        {
            return 0;
        }
        s++;
    }

    return 0;
}

/*
 *  FindQuoteEnd(); Finds and returns the end of the quote_string on
 *  the past line of text.
 */

char *FindQuoteEnd(char *txt)
{
    char *s, *c;

    if (txt == NULL || strlen(txt) == 0)
    {
        return txt;
    }

    if (strlen(txt) <= quote_len)
    {
        s = txt + strlen(txt) - 1;
    }
    else
    {
        s = txt + quote_len;
    }

    if ((c = strchr(txt, '<')) != NULL)
    {
        /*
         *  Check for the special case of '<sigh>' or some such similar
         *  text stuffing up the quoting process.
         */

        /* mods by PE 1995-04-26 */
        if (c < s)
        {
            if (c > txt)
            {
                s = c - 1;
            }
            else if (c == txt)
            {
                return (txt);
            }
        }
    }

    while (s > txt && *s != '>')
    {
        s--;
    }

    if (s == txt && *s != '>')
    {
        return txt;
    }

    if (*s == '>' && *(s + 1))
    {
        /* go past the '>' character */

        /*
         *  We only want to increment two characters - this saves trouble
         *  when we encounter indents in the quotes.  We could strip
         *  spaces when finding indents, but why bother?
         */

        s++;
        if (*s == ' ' && *(s + 1))
        {
            s++;
        }
    }

    return s;
}

/*
 *  GetWrapPoint(); Finds the best point to break a line.
 */

char *GetWrapPoint(char *text, int rm)
{
    char *s, *sp;
    int slen;

    if (text == NULL || strlen(text) == 0)
    {
        return NULL;
    }

    /* find the point to look for a wrap and save the spot */

    slen = strlen(text);

    if (slen > rm)
    {
        sp = text + (rm - 1);
    }
    else
    {
        sp = text + (slen - 1);
    }

    s = sp;

    if (*s == '\0' || *s == '\n' || *s == '\r')
    {
        return NULL;
    }

    if (!iswhspace(*s))
    {
        /* search backward for beginning of word */

        while (*s && !iswhspace(*s))
        {
            if (s - text < 2)
            {
                /* Can't wrap any further, so split at EOL. */

                if (sp > text)
                {
                    s = sp - 1;
                }
                else
                {
                    s = sp;
                }
                break;
            }
            s--;
        }
        s++;
    }
    else
    {
        /* search forward for the beginning of next word */

        while (*s && iswhspace(*s))
        {
            if (s - text < rm / 2)
            {
                break;
            }
            if (s - text < rm)  /* prevent a line with too much whitespaces */
            {
                s++;
            }
            else
            {
                break;
            }
        }
        if (*s == '\0' || *s == '\n' || *s == '\r')
        {
            return NULL;
        }
    }

    return s;
}

/*
 *  wrap(); this is a very big procedure; wraps a line and following ones.
 */

int wrap(LINE * cl, int x, int y, int rm)
{
    LINE *l, *nl;
    char *s, *t, *tl, ch;
    int wrapped_line = 0;
    int slen, space;

    unused(x);
    unused(y);
    l = cl;

    /* stop when no more lines to process */
    while (l != NULL)
    {
        /* get the next line for later use */

        nl = l->next;

        if (l->text == NULL || strlen(l->text) < rm)
        {
            /* we may want to copy stuff from the next line to this
               one */

            if (nl == NULL || nl->text == NULL)
            {
                /* nothing we can do */
                return wrapped_line;
            }

            /*
             *  A '\n' terminates a para, so if we have one on this
             *  line, we want to stop wrapping and return.
             */

            if (l->text != NULL && strchr(l->text, '\n') != NULL)
            {
                return wrapped_line;
            }

            /* Get the length of the current line. */

            s = FindQuoteEnd(nl->text);

            if (l->text != NULL)
            {
                slen = strlen(l->text);
            }
            else
            {
                slen = 0;
            }

            /*
             *  If the next line can fit on this line, then we may
             *  as well simply copy the entire line and delete the
             *  next line, otherwise we copy only what we have space
             *  for.
             */

            space = rm - slen;
            if (space > strlen(s))
            {
                /* then we move the entire line up */

                tl = xcalloc(1, strlen(s) + slen + 2);

                /* Copy the text to the new memory. */

                if (l->text)
                {
                    strcpy(tl, l->text);
                }
                else
                {
                    strcpy(tl, "");
                }

                if (trailspace(tl) == 0 && !iswhspace(*s))
                {
                    if (*s != '\0' && *s != '\n')
                    {
                        strcat(tl, " ");
                    }
                }

                /* If the line that will be deleted had the cursor,
                   mark the line that it has been merged with as
                   having the cursor. */

                if (nl->cursor_position)
                {
                    if (nl->cursor_position > (s - nl->text))
                    {
                        l->cursor_position = strlen(tl) +
                            nl->cursor_position - (s - nl->text);
                    }
                    else
                    {
                        l->cursor_position = strlen(tl) + 1;
                    }
                }

                strcat(tl, s);

                /* Delete the old line. */

                l->next = nl->next;

                if (l->next)
                {
                    l->next->prev = l;
                }

                release(nl->text);
                release(nl);
                release(l->text);

                l->text = tl;
                wrapped_line = 1;
            }
            else
            {
                if (space == 0)
                {
                    return wrapped_line;
                }

                /* We want to copy some words up to this line; */

                t = s + space - 1;
                tl = t;

                /*
                 *  We start the wrap at the amount of space left on the
                 *  previous line.  If that spot is on a word, then we
                 *  move to the beginning of that word and wrap anything
                 *  before that. If the word goes to the beginning of the
                 *  line, then it is too big to wrap (we only wrap whole
                 *  words).
                 */

                if (!iswhspace(*t))
                {
                    while (t > s && !iswhspace(*t))
                    {
                        t--;
                    }
                    if (t == s)
                    {
                        /* word too big */
                        return wrapped_line;
                    }
                    else
                    {
                        t++;
                    }
                }
                else
                {
                    while (*t && iswhspace(*t))
                    {
                        t++;
                    }

                    if (*t == '\0' || *t == '\n')
                    {
                        t = tl;
                    }
                }

                /* Save current position. */

                ch = *t;
                *t = '\0';

                tl = xcalloc(1, strlen(s) + slen + 2);

                /* Copy stuff to be wrapped to the new memory. */

                if (l->text)
                {
                    strcpy(tl, l->text);
                }
                else
                {
                    strcpy(tl, "");
                }

                if (trailspace(tl) == 0 && !iswhspace(*s))
                {
                    strcat(tl, " ");
                }

                /* Adapt the cursor position */

                if (nl->cursor_position)
                {
                    if (nl->cursor_position > (s - nl->text))
                    {
                        if (nl->cursor_position - (s - nl->text) <=
                            strlen(s))
                        {
                            l->cursor_position = strlen(tl) +
                                nl->cursor_position - (s - nl->text);
                            nl->cursor_position = 0;
                        }
                        else
                        {
                            nl->cursor_position -= strlen(s);
                        }
                    }
                    else
                    {
                        l->cursor_position = strlen(tl) + 1;
                        nl->cursor_position = 0;
                    }
                }
                
                strcat(tl, s);

                /*
                 *  Assign new memory and move what is left on next line
                 *  to beginning of that line (we don't bother to
                 *  reallocate it).
                 */

                release(l->text);
                l->text = tl;
                wrapped_line = 1;
                *t = ch;
                memmove(s, t, strlen(t) + 1);
            }
        }
        else
        {
            /*
             *  We want to wrap stuff on current line to the next line
             *  because current line is overflowing.
             */

            t = GetWrapPoint(l->text, rm);
            if (t == NULL)
            {
                return wrapped_line;
            }

            if (nl != NULL && nl->text != NULL && strchr(l->text, '\n') == NULL && l->quote == nl->quote)
            {
                s = xcalloc(1, strlen(t) + strlen(nl->text) + 2);

                tl = FindQuoteEnd(nl->text);

                if (t == NULL)
                {
                    ed_error("wrap()", "Logic error wrapping to next line!");
                }

                ch = *tl;
                *tl = '\0';

                strcpy(s, nl->text);
                strcat(s, t);

                /* adapt the cursor position */
                if (l->cursor_position)
                {
                    if (l->cursor_position > (t - l->text))
                    {
                        nl->cursor_position = l->cursor_position -
                            (t-l->text) + strlen(nl->text);
                        l->cursor_position = 0;
                    }
                }
                else if (nl->cursor_position)
                {
                    if (nl->cursor_position > strlen(nl->text))
                    {
                        nl->cursor_position += strlen(t);
                    }
                }

                if (trailspace(s) == 0)
                {
                    strcat(s, " ");
                }

                *tl = ch;
                strcat(s, tl);
                release(nl->text);
                nl->text = s;
                *t = '\0';
                wrapped_line = 1;

            }
            else
            {
                nl = InsertLine(l);
                if (cl->quote)
                {
                    nl->quote = 1;
                    s = FindQuoteEnd(l->text);
                    ch = *s;
                    *s = '\0';

                    tl = xcalloc(1, strlen(t) + strlen(l->text) + 2);

                    strcpy(tl, l->text);
                    strcat(tl, t);
                    *s = ch;
                    nl->text = tl;
                }
                else
                {
                    nl->text = xstrdup(t);
                    s = l->text;
                }

                /* adapt the cursor position */
                if (l->cursor_position)
                {
                    if (l->cursor_position > (t - l->text))
                    {
                        nl->cursor_position = l->cursor_position -
                            (t-l->text) + (s - l->text);
                        l->cursor_position = 0;
                    }
                }


                *t = '\0';
                wrapped_line = 1;
            }
            l = l->next;
        }
    }
    return wrapped_line;
}

/*
 *  toggle_quote(); Toggles the quote status of the current line.
 */

static void toggle_quote(void)
{
    if (current == NULL)
    {
        return;
    }

    if (current->quote)
    {
        current->quote = 0;
    }
    else
    {
        current->quote = 1;
    }

    EdPutLine(current, y);
}

/*
 *  CheckXvalid(); Checks to see that the current X position is still
 *  on the line (ie. X is not past EOL).
 */

void CheckXvalid(void)
{
    SetLineBuf();
    if (current->text == NULL || strlen(current->text) < x)
    {
        go_eol();
    }
}

/*
 *  WordStart(); Finds the start of the current word at the current X
 *  position, and then returns the difference between that position and
 *  the current X position.
 */

int WordStart(void)
{
    char *s, *b;

    if (*line_buf == 0)
    {
        return 0;
    }

    s = line_buf + x - 1;
    b = s;

    if (iswhspace(*s))
    {
        return 0;
    }

    while (s > line_buf && !iswhspace(*s))
    {
        s--;
    }

    return (int) (b - s);
}

/*
 *  insert_char(); Inserts a char at the current position.
 */

static void insert_char(char ch)
{
    int slen, wlen;

#ifdef UNIX /* entering these characters would cause problems on Unix */
    if ((unsigned char)ch < 32 ||
        ((unsigned char)ch >= 128 && (unsigned char)ch < (128 + 32)))
    {
        return;
    }
#endif    

    if ((unsigned char)ch == 0x8d && softcrxlat)
    {
        ch = softcrxlat;
    }

    if (insert == 0 && line_buf[x - 1] != '\n')
    {
        line_buf[x - 1] = ch;
    }
    else
    {
        memmove(line_buf + x, line_buf + x - 1, strlen(line_buf + x - 1) + 1);
        line_buf[x - 1] = ch;
    }

    UnmarkLineBuf();

    current->templt = 0;
    wlen = WordStart();
    slen = strlen(current->text);

    if (wrap(current, 0, 0, SW->rm) == 1)
    {
        SetLineBuf();
        RedrawPage(current, y);
        if (strlen(current->text) < x)
        {
            if (wlen)
            {
                x = wlen;
                if (current->quote)
                {
                    char *s;

                    s = FindQuoteEnd(current->text);
                    if (s && s > current->text)
                    {
                        x += (int)(s - current->text);
                    }
                }
                if (x > strlen(current->next->text))
                {
                    x = strlen(current->next->text) - 1;
                }
            }
            else
            {
                x = slen - strlen(current->text) - 1;
            }

            go_down();
        }
    }
    else
    {
        EdPutLine(current, y);
    }

    x++;
    SetLineBuf();
}

/*
 *  delete_character(); Deletes the character at the current X position.
 */

static void delete_character(void)
{
    LINE *nl;

    current->templt = 0;

    if (*line_buf == 0 || line_buf[0] == '\n')
    {
        /* Current line is blank, so we delete it. */

        if (current->next == NULL)
        {
            return;
        }

        current->next->prev = current->prev;
        if (current->prev)
        {
            current->prev->next = current->next;
        }

        if (msgtop == current)
        {
            msgtop = current->next;
        }

        if (pagetop == current)
        {
            msgtop = current->next;
        }

        nl = current;
        current = nl->next;

        release(nl->text);
        release(nl);
        RedrawPage(current, y);
    }
    else
    {
        /* Else we just want to kill the char at x. */

        memmove(line_buf + x - 1, line_buf + x, strlen(line_buf + x) + 1);

        UnmarkLineBuf();

        if (wrap(current, 0, 0, SW->rm) == 1)
        {
            RedrawPage(current, y);
        }
        else
        {
            EdPutLine(current, y);
        }
    }
    SetLineBuf();
}

/*
 *  backspace; Deletes the char behind the current X pos and moves
 *  the cursor back one char.
 */

static void backspace(void)
{
    if (x == 1)
    {
        if (current->prev == NULL)
        {
            return;
        }
        UnmarkLineBuf();
        go_up();
        go_eol();
        delete_character();
    }
    else
    {
        x--;
        delete_character();
    }
    EdPutLine(current, y);
}

static void delword(void)
{
    char *s;

    s = line_buf + x - 1;

    while (*s && !m_isspace(*s))
    {
        s++;
    }

    while (*s && m_isspace(*s))
    {
        s++;
    }

    strcpy(line_buf + x - 1, s);

    UnmarkLineBuf();

    wrap(current, x, y, SW->rm);
    RedrawPage(current, y);

    SetLineBuf();
}

static void go_left(void)
{
    if (x == 1)
    {
        if (current->prev)
        {
            go_up();
            go_eol();
        }
    }
    else
    {
        x--;
    }
}

static void go_right(void)
{
    if (line_buf[x - 1] == '\0' || line_buf[x - 1] == '\n')
    {
        if (current->next)
        {
            go_down();
            go_bol();
        }
    }
    else
    {
        x++;
    }
}

static void go_up(void)
{
    UnmarkLineBuf();
    if (current->prev)
    {
        current = current->prev;
        currline--;
        if (y == ed_miny)
        {
            pagetop = current;
            ScrollDown(1, ed_maxy);
            EdPutLine(current, y);
        }
        else
        {
            y--;
        }
    }
    CheckXvalid();
}

static void go_down(void)
{
    UnmarkLineBuf();
    if (current->next)
    {
        current = current->next;
        currline++;
        if (y == ed_maxy)
        {
            ScrollUp(1, ed_maxy);
            EdPutLine(current, y);
        }
        else
        {
            y++;
        }
    }
    CheckXvalid();
}

static void go_bol(void)
{
    x = 1;
}

static void go_eol(void)
{
    x = strlen(line_buf);

    if (x > 0)
    {
        if (line_buf[x - 1] != '\n')
        {
            x++;
        }
    }

    x = min(max(1, x), SW->rm);
}

static void go_word_right(void)
{
    int fsm = 0, c;

    if (x >= strlen(line_buf))
    {
        if (current->next != NULL)
        {
            go_down();
            go_bol();
        }
        else
        {
            fsm = 3;
        }
    }

    while (fsm < 3)
    {
        c = m_isspace(*(line_buf + x - 1));
        switch (fsm)
        {
        case 0:
            if (!c)
            {
                fsm = 1;
            }
            else
            {
                fsm = 2;
            }
            break;

        case 1:
            if (c)
            {
                fsm = 2;
            }
            break;

        case 2:
            if (!c)
            {
                fsm = 3;
            }
            break;

        default:
            break;
        }

        if (fsm != 3)
        {
            if (x == strlen(line_buf))
            {
                if (current->next != NULL)
                {
                    go_down();
                    go_bol();
                }
                else
                {
                    fsm = 3;
                }
            }
            else
            {
                x++;
            }
        }
    }
}

static void go_word_left(void)
{
    int fsm = 0, c;

    if (x == 1)
    {
        if (current->prev != NULL)
        {
            go_up();
            go_eol();
        }
        else
        {
            fsm = 4;
        }
    }

    while (fsm < 4)
    {
        c = m_isspace(*(line_buf + x - 1));
        switch (fsm)
        {
        case 0:
            if (!c)
            {
                fsm = 1;
            }
            else
            {
                fsm = 3;
            }
            break;

        case 1:
            if (!c)
            {
                fsm = 2;
            }
            else
            {
                fsm = 3;
            }
            break;

        case 2:
            if (c)
            {
                fsm = 4;
                x++;
            }
            break;

        case 3:
            if (!c)
            {
                fsm = 2;
            }
            break;

        default:
            break;
        }

        if (fsm != 4)
        {
            if (x == 1)
            {
                if (current->prev != NULL)
                {
                    go_up();
                    go_eol();
                }
                else
                {
                    fsm = 4;
                }
            }
            else
            {
                x--;
            }
        }
        else if (x > strlen(line_buf))
        {
            if (current->next != NULL)
            {
                go_down();
                go_bol();
            }
            else
            {
                go_eol();
            }
        }
    }
}

static void go_pgup(void)
{
    LINE *l = current;
    int count = 1;

    UnmarkLineBuf();
    while (count < ed_maxy && current->prev)
    {
        count++;
        current = current->prev;
        currline--;
    }

    /* If we actually moved, redraw the page. */

    if (l != current)
    {
        y = 1;
        RedrawPage(current, 1);
    }
    CheckXvalid();
}

static void go_pgdown(void)
{
    LINE *l = current;
    int count = 1;

    UnmarkLineBuf();
    while (count < ed_maxy && current->next)
    {
        count++;
        current = current->next;
        currline++;
    }

    /* If we actually moved, redraw the page. */

    if (l != current)
    {
        y = 1;
        RedrawPage(current, 1);
    }
    CheckXvalid();
}

static void newline(void)
{
    LINE *nl;

    nl = InsertLine(current);
    if (nl == NULL)
    {
        return;
    }

    /*
     *  If the current line is a quote, then the break shouldn't cause a
     *  wrap from the previous line.  This is prevented by a hard CR
     *  (which the user can then kill if wanted).
     */

    if (current->quote && !strchr(line_buf, '\n'))
    {
        strcat(line_buf, "\n");
    }

    nl->text = xstrdup(line_buf + x - 1);
    line_buf[x - 1] = '\0';

    strcat(line_buf, "\n");

    if (current->block)
    {
        nl->block = 1;
    }

    if (current->templt)
    {
        if (x == 1)
        {
            nl->templt = 1;
            current->templt = 0;
        }
        else
        {
            current->templt = 0;
        }
    }
    go_down();
    go_bol();

    wrap(current, 0, 0, SW->rm);
    RedrawPage(current->prev, y - 1);
    SetLineBuf();
}

/*
 *  udel_add_q(); Adds l to the deleted line queue.
 */

static void udel_add_q(LINE * l)
{
    LINE *nl = udel;
    int num = 0;

    if (udel == NULL)
    {
        udel = l;
        l->next = NULL;
        l->prev = NULL;
        return;
    }

    /* find the last line, keeping count of lines * in the process. */

    while (nl->next != NULL)
    {
        nl = nl->next;
        num++;
    }

    /*
     *  If there are more than max num of lines, then we delete the
     *  oldest one (on the end of queue).
     */

    if (num >= 30)
    {
        nl->prev->next = NULL;
        release(nl->text);
        release(nl);
    }

    /* Add the latest deleted line to the beginning of the queue. */

    udel->prev = l;
    l->next = udel;
    l->prev = NULL;
    udel = l;
}

static void delete_line(void)
{
    LINE *nl;

    if (current->next == NULL)
    {
        if (current->prev)
        {
            current->prev->next = NULL;
            nl = current;
            current = current->prev;
            if (y > 1)
            {
                y--;
            }
        }
        else
        {
            nl = current;
            current = xmalloc(sizeof(LINE));
            memcpy(current, nl, sizeof(LINE));
            current->text = xstrdup("\n");
        }
        GotoXY(x, y);
    }
    else
    {
        current->next->prev = current->prev;
        if (current->prev)
        {
            current->prev->next = current->next;
        }
        nl = current;
        current = (nl->next != NULL) ? nl->next : nl->prev;
    }

    if (msgtop == nl)
    {
        msgtop = current;
    }

    /* Save the deleted line in a buffer. */

    udel_add_q(nl);

    /* Redraw the screen to reflect missing line. */

    RedrawPage(current, y);
    CheckXvalid();
}

/*
 *  udel_delete_q(); Removes and returns the first line in the
 *  undelete buffer.
 */

static LINE *udel_delete_q(void)
{
    LINE *nl = udel;

    if (udel == NULL)
    {
        return NULL;
    }

    if (udel->next)
    {
        udel->next->prev = NULL;
        udel = udel->next;
    }
    else
    {
        udel = NULL;
    }

    return nl;
}

/*
 *  undelete(); If there is a line to undelete, undeletes the last line
 *  deleted and inserts it before the current line.
 */

static void undelete(void)
{
    LINE *nl;

    /* Get last line deleted. */

    nl = udel_delete_q();

    if (nl == NULL)
    {
        return;
    }

    /* Make sure we don't split it (the current block) in half :-) */

    nl->templt = 0;

    if (blocking && current->block)
    {
        nl->block = 1;
    }
    else
    {
        if (current->block == 0)
        {
            nl->block = 0;
        }
    }

    /* Insert it ABOVE current line. */

    nl->prev = current->prev;
    current->prev = nl;
    nl->next = current;
    if (nl->prev)
    {
        nl->prev->next = nl;
    }
    if (msgtop == current)
    {
        msgtop = nl;
    }

    /* Make it the current line && redraw page to reflect new line. */

    UnmarkLineBuf();
    current = nl;
    RedrawPage(current, y);
    CheckXvalid();
}

/*
 *  ClearBlocks(); Clears all blocked lines in the message.
 */

static void ClearBlocks(void)
{
    LINE *nl = msgtop;

    while (nl != NULL)
    {
        if (nl->block)
        {
            nl->block = 0;
        }

        nl = nl->next;
    }
    blocking = 0;
}

/*
 *  CalculateBlocks(); Provides QEdit style blocking. Called everytime
 *  anchor is called (a new anchor has been laid).
 */

static void CalculateBlocks(void)
{
    LINE *nl = msgtop;
    int bl = 0;                 /* hit a block yet? */
    int cp = 0;                 /* gone past current line? */

    while (nl != NULL)
    {
        /*
         *  If we've passed the current line and hit the block and come
         *  out the other side, we want to stop.
         */

        if (cp && bl && !nl->block)
        {
            break;
        }

        /* If bl = FALSE and we get blocked line, turn ON. */

        if (!bl && nl->block)
        {
            bl = 1;
        }

        /* If we hit current line. */

        if (nl == current)
        {
            /*
             *  If anchor is hit in the middle of a block, then we want
             *  to recreate the block on that line.
             */

            if (nl->block)
            {
                ClearBlocks();
                bl = 0;
            }
            nl->block = 1;
            if (bl)
            {
                /*
                 *  If we're already blocking, then we want to stop at
                 *  the current line.
                 */
                break;
            }
            else
            {
                if (blocking == 0)
                {
                    /*
                     *  If we aren't blocking and global blocking hasn't
                     *  been set, then we want to stop, else start
                     *  blocking.
                     */
                    break;
                }
            }
            cp = 1;
        }

        /*
         *  If we've passed the current line and haven't hit a block yet,
         *  then we want to block all lines between.
         */

        if (cp && !bl)
        {
            nl->block = 1;
        }

        /*
         *  If we haven't hit the current line, but have hit the block,
         *  we want to mark stuff in between.
         */

        if (!cp && bl)
        {
            nl->block = 1;
        }

        nl = nl->next;
    }
    blocking = 1;
}

/*
 *  unblock(); Kills the current block.
 */

static void unblock(void)
{
    LINE *nl = current;
    int y2 = y;

    ClearBlocks();

    /* Find the top of the page and redraw it. */

    while (y2 != ed_miny)
    {
        nl = nl->prev;
        y2--;
    }
    RedrawPage(nl, ed_miny);
}

/*
 *  anchor(); Lays a block anchor at the current position.
 */

static void anchor(void)
{
    LINE *nl = current;
    int y2 = y;

    /* Calculate the new block configuration. */

    CalculateBlocks();

    /* Find the top of the page and redraw it. */

    while (y2 != ed_miny)
    {
        nl = nl->prev;
        y2--;
    }
    RedrawPage(nl, ed_miny);
}

/*
 *  cut(); Cuts the current block from the message and saves it.
 */

static void cut(void)
{
    LINE *nl, *begin, *end;
    int y1;

    if (!blocking)
    {
        return;
    }

    UnmarkLineBuf();

    if (clip != NULL)
    {
        /* Discard whatever was in there before. */
        clip = clearbuffer(clip);
    }

    nl = msgtop;
    begin = NULL;
    end = NULL;

    /* Find the begin and end of the block. */

    while (nl)
    {
        /* If we haven't hit a block yet, then this must be it. */

        if (nl->block && !begin)
        {
            begin = nl;
        }

        /* If we've hit a block & come out the other side, break. */

        if (begin && !nl->block && !end)
        {
            end = nl;
            break;
        }
        nl = nl->next;
    }

    if (!begin->prev)
    {
        if (!end)
        {
            /*
             *  Whole message has been selected - create a line to put the
             *  cursor on.
             */

            nl = xcalloc(1, sizeof *nl);
            nl->text = xstrdup("\n");
            msgtop = nl;
            current = nl;
        }
        else
        {
            /* There is some message left. */

            msgtop = end;
            current = end;
            if (end->prev)
            {
                end->prev->next = NULL;
            }

            end->prev = NULL;
        }
    }
    else
    {
        /* Join up the gap where the cut will be. */

        if (end)
        {
            begin->prev->next = end;
            end->prev->next = NULL;
            end->prev = begin->prev;
        }
        else
        {
            begin->prev->next = NULL;
        }

        current = begin->prev;
        begin->prev = NULL;
    }

    /* Save the cut text and redraw the page. */

    blocking = 0;
    clip = begin;

    /* Find beginning of screen & corresponding line. */

    y1 = y;
    nl = current;
    while (y1 > ed_miny && nl->prev != NULL)
    {
        nl = nl->prev;
        y1--;
    }

    /* Handle case where lines have to be moved up on screen. */

    if (y1 != ed_miny)
    {
        y -= (y1 - ed_miny);
    }
    RedrawPage(nl, 1);
    CheckXvalid();
}

/*
 *  paste(); Pastes the block on the clipboard into the page at the
 *  current cursor position.
 */

static void paste(void)
{
    LINE *nl = current;
    LINE *t = clip;
    LINE *t1;

    if (t == NULL)
    {
        return;
    }

    /* If a block was there, kill it. */

    ClearBlocks();

    /* Copy from the clipboard to AFTER the current position. */

    while (t != NULL)
    {
        t1 = InsertLine(nl);
        t1->quote = t->quote;
        t1->text = xstrdup(t->text);
        nl = t1;
        t = t->next;
    }

    /* Redraw the page. */

    RedrawPage(current, y);
}

static void tabit(void)
{
    if (!(x % SW->tabsize))
    {
        insert_char(' ');
    }

    while (x % SW->tabsize)
    {
        insert_char(' ');
    }

    insert_char(' ');
}

static void go_tos(void)
{
    UnmarkLineBuf();
    while (y > ed_miny && current->prev != NULL)
    {
        current = current->prev;
        currline--;
        y--;
    }
    CheckXvalid();
}

static void go_bos(void)
{
    UnmarkLineBuf();
    while (y < ed_maxy && current->next != NULL)
    {
        current = current->next;
        currline++;
        y++;
    }
    CheckXvalid();
}

static void go_tom()
{
    if (current == msgtop)
    {
        return;
    }

    UnmarkLineBuf();

    current = msgtop;
    currline = 1;
    y = 1;

    CheckXvalid();
    RedrawPage(current, y);
}

static void go_bom(void)
{
    if (current->next == NULL)
    {
        return;
    }

    UnmarkLineBuf();
    while (current->next)
    {
        current = current->next;
        currline++;
    }

    y = 1;

    CheckXvalid();
    RedrawPage(current, y);
}

static void emacskill(void)
{
    if (x == 1 && line_buf[x] == 0)
    {
        delete_line();
    }
    else
    {
        killeol();
    }
}

static void killeol(void)
{
    memset(line_buf + x - 1, 0, strlen(line_buf + x - 1));
    line_buf[x - 1] = '\n';
    line_buf[x] = '\0';
    UnmarkLineBuf();
    EdPutLine(current, y);
}

static void imptxt(void)
{
    UnmarkLineBuf();
    import(current);
    RedrawPage(current, y);
    SetLineBuf();
}

static void ExportBlock(void)
{
    LINE *l, *nl;

    if (!blocking)
    {
        return;
    }

    l = msgtop;
    while (l->next && !l->block)
    {
        l = l->next;
    }

    if (!l->block)
    {
        return;
    }

    nl = l;
    while (nl->next && nl->block)
    {
        nl = nl->next;
    }
    if (!nl->block)
    {
        if (nl->prev)
        {
            nl->prev->next = NULL;
        }
    }
    export(l);
    if (!nl->block)
    {
        if (nl->prev)
        {
            nl->prev->next = nl;
        }
    }
}

static void outtext(void)
{
    static char title[] = " Export ";
    static char msgtxt[] = "Export what?";
    int res;

    if (clip && blocking == 0)
    {
        res = ChoiceBox(title, msgtxt, "Clipboard", "Message", NULL);
        cursor(1);
        switch (res)
        {
        case ID_ONE:
            export(clip);
            break;

        case ID_TWO:
            export(msgtop);
            break;

        default:
            break;
        }
        return;
    }
    else if (clip == NULL && blocking)
    {
        res = ChoiceBox(title, msgtxt, "Block", "Message", NULL);
        cursor(1);
        switch (res)
        {
        case ID_ONE:
            ExportBlock();
            break;

        case ID_TWO:
            export(msgtop);
            break;

        default:
            break;
        }
        return;
    }
    else if (clip && blocking)
    {
        res = ChoiceBox(title, msgtxt, "Clipboard", "Block", "Message");
        cursor(1);
        switch (res)
        {
        case ID_ONE:
            export(clip);
            break;

        case ID_TWO:
            ExportBlock();
            break;

        case ID_THREE:
            export(msgtop);
            break;

        default:
            break;
        }
        return;
    }
    export(msgtop);
    cursor(1);
}

static void quit(void)
{
    release(current->text);
    current->text = strdup(line_buf);
    if (current->text == NULL)
    {
        WndWriteStr(0, 0, cm[CM_WTXT], "WARNING: Memory allocation failure - attempting to save...");
    }
    done = SAVE;
}

static void die(void)
{
    if (confirm("Cancel?"))
    {
        done = ABORT;
    }
    cursor(1);
}

void count_bytes(LINE *l, long *bytes, long *quotes)
{
    long count = 0;
    long quote_count = 0;
    size_t len;

    while (l != NULL)
    {
        if (l->text)
        {
            len = strlen(l->text);
            count += len;
            if (isquote(l->text))
            {
                quote_count += len;
            }
        }
        l = l->next;
    }

    if (bytes != NULL)
    {
        *bytes = count;
    }
    if (quotes != NULL)
    {
        *quotes = quote_count;
    }
}

static void bytecount(void)
{
    long count = 0;
    long quote_count = 0;
    char text[80];

    count_bytes(msgtop, &count, &quote_count);

    sprintf(text, "Message size is %ld characters (quote ratio: %ld%%)",
                  count, (quote_count * 100) / count);
    ChoiceBox(" Message Size ", text, "  Ok  ", NULL, NULL);
    cursor(1);
}

static void toggle_ins(void)
{
    static unsigned char buf[4] = {SC8, SC8, SC8, '\0'};
    insert = !insert;
    if (insert)
    {
        WndWriteStr(maxx - 5, 5, cm[CM_DTXT], "ins");
    }
    else
    {
        WndWriteStr(maxx - 5, 5, cm[CM_DTXT], (char *)buf);
    }
}

static void close_screen(void) /* used by shellos, and on WND_WM_RESIZE) */
{
    WndClose(hMnScr);
    KillHotSpots();
    TTgotoxy(term.NRow - 1, 0);
    TTclose();
    cursor(1);
}

static void reopen_screen(void) /* used by shellos, and on WND_WM_RESIZE */
{
    LINE *curr;
    int y2;
    int oldmaxx = maxx;

    /* Redraw the screen. */

    cursor(0);
    InitScreen();
    BuildHotSpots();
    DrawHeader();
    ShowNewArea();
    ShowMsgHeader(messg);

    edminy = 5;
    edmaxy = maxy - 1;
    ed_miny = 1;
    ed_maxy = edmaxy - edminy - 1;

    if (oldmaxx != maxx) /* terminal size has changed - rewrap! */
    {
        adapt_margins();

                                /* mark the cursor position */
        for (curr = msgtop; curr != NULL; curr = curr->next)
        {
            if (curr == current)
            {
                curr->cursor_position = x;
            }
            else
            {
                curr->cursor_position = 0;
            }
        }

                                /* rewrap the message */
        for (curr = msgtop; curr != NULL; curr = curr->next)
        {
            wrap (curr, 0, 0, SW->rm);
        }

                                /* search the cursor position */
        current = NULL;
        for (curr = msgtop, currline = 1; curr != NULL; curr =
                 curr->next, currline++)
        {
            if (curr->cursor_position)
            {
                current = curr;
                x = curr->cursor_position;
                break;
            }
        }
        if (current == NULL)
        {
            ed_error("reopen_screen", "lost track of cursor!");
        }
            
        SetLineBuf();
    }

    /* redraw the screen */

    if (y > ed_maxy)
    {
        y = ed_maxy;
    }
    y2 = y;
    curr = current;
    while (y2 > ed_miny && curr->prev)
    {
        curr = curr->prev;
            y2--;
    }
    y -= (y2 - ed_miny);
    RedrawPage(curr, ed_miny);

    cursor(1);
    GotoXY(x,y);
}

static void shellos(void)
{
    static char tmp[PATHLEN];

    mygetcwd(tmp, PATHLEN);
    setcwd(ST->home);

    close_screen();

    fputs("\nEnter the command \"EXIT\" to return to " PROG ".\n", stderr);
    shell_to_dos();

    reopen_screen();

    setcwd(tmp);
}

static void doscmd(void)
{
    WND *hCurr, *hWnd;
    static char curdir[PATHLEN];
    char cmd[64], tmp[40];
    int ret;

    mygetcwd(curdir, PATHLEN);
    memset(cmd, 0, sizeof cmd);

#if defined(MSDOS)
    if (!GetString(" System Command ", "Enter DOS command to execute:", cmd, 64))
    {
        return;
    }
#elif defined(OS2)
    if (!GetString(" System Command ", "Enter OS/2 command to execute:", cmd, 64))
    {
        return;
    }
#else
    if (!GetString(" System Command ", "Enter system command to execute:", cmd, 64))
    {
        return;
    }
#endif

    hCurr = WndTop();
    hWnd = WndOpen(0, 0, maxx - 1, maxy - 1, NBDR, 0, cm[CM_NTXT]);

    cursor(1);
    ret = system(cmd);
    cursor(0);

#if defined(MSDOS)
    sprintf(tmp, "DOS command returned %d", ret);
#elif defined(OS2)
    sprintf(tmp, "OS/2 command returned %d", ret);
#else
    sprintf(tmp, "System command returned %d", ret);
#endif
    ChoiceBox(" Info ", tmp, "  Ok  ", NULL, NULL);

    WndClose(hWnd);
    WndCurr(hCurr);
    setcwd(curdir);
    cursor(1);
}

/*
 *  editheader(); Lets the user edit the header.
 */

static void editheader(void)
{
    msg *save;
    int q;

    q = 0;
    save = duplicatemsg(messg);
    while (!q)
    {
        if (EditHeader(save) == Key_Esc)
        {
            if (confirm("Cancel?..."))
            {
                dispose(save);
                cursor(1);
                return;
            }
        }
        else
        {
            q = 1;
        }
    }

    /*
     *  We want to save the changes: release all allocated memory and
     *  make *messg = *save.
     */

    release(messg->isfrom);
    release(messg->isto);
    release(messg->subj);
    release(messg->msgid);
    release(messg->reply);
    release(messg->to.domain);
    release(messg->from.domain);

    *messg = *save;

    cursor(1);
}

/*
 *  setup(); Calls the setup dialog box and allows user to play with
 *  switches.
 */

static void setup(void)
{
    LINE *nl = current;
    int y2 = y;

    set_switch();
    while (y2 != ed_miny && nl->prev != NULL)
    {
        nl = nl->prev;
        y2--;
    }
    RedrawPage(nl, ed_miny);
    cursor(1);
}

static void do_help(void)
{
    if (ST->helpfile)
    {
        DoHelp(1);
    }
}

static void UpdateXY(void)
{
    static char line[50];

    if (SW->statbar)
    {
        sprintf(line, "%c X: %03d  Y: %03d", SC7, x, currline);
#if defined(MSDOS)
        WndPutsn(maxx - (17 + 16), maxy - 1, 16, cm[CM_ITXT], line);
#else
        WndPutsn(maxx - 17, maxy - 1, 16, cm[CM_ITXT], line);
#endif
    }
}

static void UpdateMem(void)
{
    if (SW->statbar)
    {
#if defined(MSDOS) && !defined(__FLAT__)
        long mem, oldmem = 0;
        char line[15];
        mem = corerem();
        if (mem != oldmem)
        {
            oldmem = mem;
            sprintf(line, "%c %3ldK ", SC7, (long)(corerem() / 1024));
            WndPutsn(maxx - 7, maxy - 1, 7, cm[CM_ITXT], line);
        }
#endif
    }
}

int editmsg(msg * m, int quote)
{
    EVT event;
    int editcrstate = 0;
    unsigned int ch;
    static unsigned char buf[4] = {SC8, SC8, SC8, '\0'};

    x = 1;
    y = 1;
    currline = 1;
    edminy = 5;
    edmaxy = maxy - 1;
    ed_miny = 1;
    ed_maxy = edmaxy - edminy - 1;
    messg = m;
    current = messg->text;
    msgtop = messg->text;

    if (insert)
    {
        WndWriteStr(maxx - 5, 5, cm[CM_DTXT], "ins");
    }

    if (msgtop == NULL)
    {
        current = calloc(1, sizeof *current);
        if (current == NULL)
        {
            WndWriteStr(0, 0, cm[CM_WTXT], "WARNING: Memory allocation error - aborting!");
            return ABORT;
        }
        msgtop = current;
        msgtop->text = xstrdup("\n");
        msgtop->prev = NULL;
    }
    if (SW->editcronly)
    {
        editcrstate = SW->showcr;
        SW->showcr = 1;
    }
    SetLineBuf();
    RedrawPage(current, y);

    done = FALSE;
    cursor(1);
    GotoXY(x, y);

    while (!done)
    {
        if (window_resized)
        {
            close_screen();
            reopen_screen();
            window_resized = 0; /* ack! */
        }
        UpdateMem();
        UpdateXY();
        GotoXY(x, y);
        ch = MnuGetMsg(&event, hMnScr->wid);
        switch (event.msgtype)
        {
        case WND_WM_CHAR:
            if (ch & 0xff)
            {
                if (editckeys[(ch & 0xff)] == NULL)
                {
                    insert_char((char)DOROT13((char)(ch & 0xff)));
                }
                else
                {
                    (*editckeys[(ch & 0xff)]) ();
                }
            }
            else if (editakeys[(ch >> 8)] != NULL)
            {
                (*editakeys[(ch >> 8)]) ();
            }
            break;

        default:
            break;
        }
    }

    messg->text = msgtop;

    if (SW->chopquote && quote)
    {
        LINE *ff, *lowest;

        ff = lowest = msgtop;

        /* Find the lowest line of text that isn't a template line. */

        while (ff->next)
        {
            if (!ff->quote && strlen(ff->text) != 0 && *(ff->text) != '\n' && !ff->templt)
            {
                lowest = ff;
            }

            ff = ff->next;
        }

        if (lowest && lowest != msgtop && lowest->next)
        {
            ff = lowest;

            /*
             *  We got a lowest line, now we find the template line that
             *  is next underneath it.
             */

            while (ff)
            {
                if (ff->templt)
                {
                    break;
                }

                ff = ff->next;
            }

            if (ff == NULL)
            {
                /* didn't find one */
                lowest->next = clearbuffer(lowest->next);
            }
            else
            {
                LINE *d;

                /*
                 *  We've found one, so delete all lines of text between
                 *  the lowest and the template line.
                 */

                ff = lowest->next;

                while (!ff->templt)
                {
                    d = ff;
                    ff = ff->next;
                    if (d->prev)
                    {
                        d->prev->next = ff;
                    }
                    if (d->next)
                    {
                        d->next->prev = d->prev;
                    }
                    release(d->text);
                    xfree(d);
                }
            }
        }
    }

    /* Clean up everything. */

    if (insert)
    {
        WndWriteStr(maxx - 5, 5, cm[CM_DTXT], (char *)buf);
    }

    if (SW->editcronly)
    {
        SW->showcr = editcrstate;
    }

    blocking = 0;
    cursor(0);

    return done;
}

int ed_error(char *fc, char *fmt,...)
{
    va_list params;
    static char line[255];
    va_start(params, fmt);
    vsprintf(line, fmt, params);
    fprintf(stderr, "\nError! function %s: %s", fc, line);
    exit(-1);
    return 0;
}
