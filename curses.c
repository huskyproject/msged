/*
 *  curses.c
 *
 *  Written by Max Khon <fjoe@iclub.nsu.ru> et al and released to the public
 *  domain.
 *
 *  Screen definitions & routines using [n]curses.
 */

#ifdef USE_CURSES

#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "winsys.h"
#include "unused.h"
#include "keys.h"
#include "readtc.h"
#include "specch.h"
#include "free.h"

int color;
int vrow, vcol;
int cur_start = 0;
int cur_end   = 0;
static unsigned char * allowed_special_characters = NULL;

#ifdef UNIX
volatile
#endif
TERM term =
{
    80, 24, 0
};
static int tcflags = 0;     /* what we want to extract from termcap */

#define EBUFSZ 100
static EVT EVent[EBUFSZ];   /* event circular queue */
static int ebufin  = 0;     /* event in */
static int ebufout = 0;     /* event out */
static int kbhit(void);

static int norefresh = 0;
static void ttrefresh(void)
{
    if(!norefresh)
    {
        refresh();
    }
}

void TTBeginOutput(void)
{
    norefresh++;
}

void TTEndOutput(void)
{
    if(norefresh)
    {
        norefresh--;
        ttrefresh();
    }
}

/* This maps PC colors to monochrome attributes for w/o color support */
static int mono_colors[128] =
{
    A_NORMAL,           A_NORMAL,              A_NORMAL,                       A_NORMAL,
    A_NORMAL,
    A_NORMAL,           A_NORMAL,              A_NORMAL,                       A_BOLD,
    A_BOLD,
    A_BOLD,             A_BOLD,                A_BOLD,                         A_BOLD,
    A_BOLD,
    A_BOLD,             A_REVERSE,             A_REVERSE,                      A_REVERSE,
    A_REVERSE,
    A_REVERSE,          A_REVERSE,             A_REVERSE,                      A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,    A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,    A_REVERSE,                      A_REVERSE,
    A_REVERSE,
    A_REVERSE,          A_REVERSE,
    A_REVERSE,          A_REVERSE,             A_REVERSE,                      A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,    A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_REVERSE,             A_REVERSE,                      A_REVERSE,
    A_REVERSE,
    A_REVERSE,          A_REVERSE,
    A_REVERSE,          A_REVERSE,             A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,    A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_REVERSE,             A_REVERSE,                      A_REVERSE,
    A_REVERSE,
    A_REVERSE,          A_REVERSE,
    A_REVERSE,          A_REVERSE,             A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,    A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_REVERSE,             A_REVERSE,                      A_REVERSE,
    A_REVERSE,
    A_REVERSE,          A_REVERSE,
    A_REVERSE,          A_REVERSE,             A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,    A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_REVERSE,             A_REVERSE,                      A_REVERSE,
    A_REVERSE,
    A_REVERSE,          A_REVERSE,
    A_REVERSE,          A_REVERSE,             A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,    A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_REVERSE,             A_REVERSE,                      A_REVERSE,
    A_REVERSE,
    A_REVERSE,          A_REVERSE,
    A_REVERSE,          A_REVERSE,             A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE, A_BOLD | A_REVERSE,    A_BOLD | A_REVERSE,             A_BOLD | A_REVERSE,
    A_BOLD | A_REVERSE,
};
int TTScolor(unsigned int newcolor)
{
    int attr = 0;

    color = newcolor;

    if(!has_colors())
    {
        attr = mono_colors[color & 0x7F];
    }
    else
    {
        if(color & 0x08)
        {
            attr |= A_DIM | A_BOLD;
        }

        if(color & 0x80)
        {
            attr |= A_BLINK;
        }

        attr |= COLOR_PAIR(((color & 0x07) | ((color & 0x70) >> 1)));
    }

#ifdef A_ALTCHARSET

    if(color & F_ALTERNATE)
    {
        attr |= A_ALTCHARSET;
    }

#endif

    attrset(attr);
    bkgdset(attr & (~A_ALTCHARSET));
    return 1;
} /* TTScolor */

int TTCurSet(int st)
{
    if(st)
    {
        curs_set(1);
    }
    else
    {
        curs_set(0);
    }

    return 0;
}

int TTgotoxy(int row, int col)
{
    move(vrow = row, vcol = col);
    ttrefresh();
    return 1;
}

int TTgetxy(int * row, int * col)
{
    *row = vrow;
    *col = vcol;
    return 1;
}

int TTPutChr(unsigned int ch)
{
    move(vrow, vcol);
    addch(ch);
    ttrefresh();
    return 1;
}

int TTWriteStr(unsigned long * b, int len, int row, int col)
{
    int oldcol = color;
    int cut    = 0;

    move(row, col);

    for( ; len; len--, b++)
    {
        int ch = (int)(*b & 0x000000ffUL);
        int col = (int)((*b & 0xffff0000UL) >> 16);
        int y, i;

        /* control chars are written in ^X notation */
        if(ch < ' ')
        {
            switch(ch)
            {
                case '\t':
                    getyx(stdscr, y, i);
                    i    = ((y >> 3) + 1) << 3;
                    cut += i - y;
                    len -= i - y;
                    break;

                case '\n':
                case '\r':
                    len += cut;
                    cut  = 0;
                    break;

                case '\b':

                    if(cut > 0)
                    {
                        len++;
                        cut--;
                    }

                    break;

                default:
                    len--;
                    cut++;
            } /* switch */
        }

        if(color != col)
        {
            TTScolor(col);
        }

        addch(ch);
    }

    if(color != oldcol)
    {
        TTScolor(oldcol);
    }

    move(vrow, vcol);
    ttrefresh();
    return 1;
} /* TTWriteStr */

int TTStrWr(unsigned char * s, int row, int col, int len)
{
    int cut = 0;
    int y, i;

    if(s == NULL)
    {
        return 1;
    }

    if(len < 0)
    {
        len = strlen((const char *)s);
    }

    if(len == 0)
    {
        return 1;
    }

    move(row, col);

    for( ; len; len--, s++)
    {
        /* control chars are written in ^X notation */
        if(*s < ' ')
        {
            switch(*s)
            {
                case '\t':
                    getyx(stdscr, y, i);
                    i    = ((y >> 3) + 1) << 3;
                    cut += i - y;
                    len -= i - y;
                    break;

                case '\n':
                case '\r':
                    len += cut;
                    cut  = 0;
                    break;

                case '\b':

                    if(cut > 0)
                    {
                        len++;
                        cut--;
                    }

                    break;

                default:
                    len--;
                    cut++;
            } /* switch */
        }

        addch(*s);
    }
    move(vrow, vcol);
    ttrefresh();
    return 1;
} /* TTStrWr */

int TTReadStr(unsigned long * b, int len, int row, int col)
{
    while(len--)
    {
        int ch;
        unsigned long cell;
        ch   = mvinch(row, col);
        cell = ch & A_CHARTEXT;

        if(ch & (A_DIM | A_BOLD))
        {
            cell |= 0x080000;
        }

        if(ch & A_BLINK)
        {
            cell |= 0x800000;
        }

        if(ch & A_ALTCHARSET)
        {
            cell |= (((unsigned long)F_ALTERNATE) << 16);
        }

        cell |= (((ch & 0x0700) | ((ch & 0x7000) << 1))) << 8;
        *b++  = cell;
        col++;
    }
    move(vrow, vcol);
    ttrefresh();
    return 1;
} /* TTReadStr */

int TTScroll(int x1, int y1, int x2, int y2, int lines, int dir)
{
#if 0
    /*
     *  XXX  If you can make this work - mail me
     *  XXX  (Seems that TTClear doesn't work properly too)
     */
    int height = y2 - y1 + 1;
    WINDOW * win;
    win = subwin(stdscr, height, x2 - x1 + 1, y1, x1);

    if(win == NULL)
    {
        return 0;
    }

    scrollok(win, TRUE);
    wscrl(win, dir ? lines : -lines);
    touchline(stdscr, y1, height);
    wttrefresh(win);
    delwin(win);
#else
    int y;
    int width = x2 - x1 + 1;
    unsigned long * buf;

    if(lines <= 0)
    {
        return 0;
    }

    buf = malloc(width * sizeof(unsigned long));

    if(buf == NULL)
    {
        return 0;
    }

    if(dir)
    {
        for(y = y1 + lines; y <= y2; y++)
        {
            TTReadStr(buf, width, y, x1);
            TTWriteStr(buf, width, y - lines, x1);
        }
        /* fill new lines with spaces */
        TTClear(x1, y2 - lines + 1, x2, y2);
    }
    else
    {
        for(y = y2 - lines; y >= y1; y--)
        {
            TTReadStr(buf, width, y, x1);
            TTWriteStr(buf, width, y + lines, x1);
        }
        /* fill new lines with spaces */
        TTClear(x1, y1, x2, y1 + lines - 1);
    }

    nfree(buf);
    move(vrow, vcol);
    ttrefresh();
#endif /* if 0 */
    return 1;
} /* TTScroll */

int TTClear(int x1, int y1, int x2, int y2)
{
#if 0
    int height = y2 - y1 + 1;
    WINDOW * win;
    win = subwin(stdscr, height, x2 - x1 + 1, y1, x1);

    if(win == NULL)
    {
        return 0;
    }

    werase(win);
    touchline(stdscr, y1, height);
    wttrefresh(win);
    delwin(win);
#else
    int y;

    for(y = y1; y <= y2; y++)
    {
        int x;
        move(y, x1);

        for(x = x1; x <= x2; x++)
        {
            addch(' ');
        }
    }
    move(vrow, vcol);
    ttrefresh();
#endif /* if 0 */
    return 1;
} /* TTClear */

int TTEeol(void)
{
    move(vrow, vcol);
    clrtoeol();
    move(vrow, vcol);
    ttrefresh();
    return 1;
}

int TTdelay(int mil)
{
    unused(mil);
    return 0;
}

static unsigned meta_alphas[] =
{
    Key_A_A, Key_A_B, Key_A_C, Key_A_D, Key_A_E, Key_A_F, Key_A_G, Key_A_H, Key_A_I, Key_A_J,
    Key_A_K, Key_A_L, Key_A_M, Key_A_N, Key_A_O, Key_A_P, Key_A_Q, Key_A_R, Key_A_S, Key_A_T,
    Key_A_U, Key_A_V, Key_A_W, Key_A_X, Key_A_Y, Key_A_Z
};
static unsigned meta_digits[] =
{
    Key_A_0, Key_A_1, Key_A_2, Key_A_3, Key_A_4, Key_A_5, Key_A_6, Key_A_7, Key_A_8, Key_A_9
};
void TTSendMsg(int msg, int x, int y, int msgtype);

unsigned int TTGetKey(void)
{
    int ch;

    ch = getch();

    switch(ch)
    {
        case KEY_RESIZE:
            term.NRow = getmaxy(stdscr);
            term.NCol = getmaxx(stdscr);
            TTSendMsg(1, 0, 0, WND_WM_RESIZE);
            return -1;

        case KEY_LEFT:
            return Key_Lft;

        case KEY_RIGHT:
            return Key_Rgt;

        case KEY_UP:
            return Key_Up;

        case KEY_DOWN:
            return Key_Dwn;

        case KEY_BACKSPACE:
            return Key_BS;

        case KEY_NPAGE:
            return Key_PgDn;

        case KEY_PPAGE:
            return Key_PgUp;

        case KEY_HOME:
            return Key_Home;

        case KEY_END:
            return Key_End;

        case KEY_F(1):
            return Key_F1;

        case KEY_F(2):
            return Key_F2;

        case KEY_F(3):
            return Key_F3;

        case KEY_F(4):
            return Key_F4;

        case KEY_F(5):
            return Key_F5;

        case KEY_F(6):
            return Key_F6;

        case KEY_F(7):
            return Key_F7;

        case KEY_F(8):
            return Key_F8;

        case KEY_F(9):
            return Key_F9;

        case KEY_F(10):
            return Key_F10;

        case KEY_F(11):
            return 0x1b; /* DEC mode ... <grin> */

        case 0x1b:
            halfdelay(1);
            ch = getch();
            nocbreak();
            cbreak();

            switch(tolower(ch))
            {
                case ERR:
                case 0x1b:
                    return 0x1b;

                case 'a':
                    return Key_A_A;

                case 'b':
                    return Key_A_B;

                case 'c':
                    return Key_A_C;

                case 'd':
                    return Key_A_D;

                case 'e':
                    return Key_A_E;

                case 'f':
                    return Key_A_F;

                case 'g':
                    return Key_A_G;

                case 'h':
                    return Key_A_H;

                case 'i':
                    return Key_A_I;

                case 'j':
                    return Key_A_J;

                case 'k':
                    return Key_A_K;

                case 'l':
                    return Key_A_L;

                case 'm':
                    return Key_A_M;

                case 'n':
                    return Key_A_N;

                case 'o':
                    return Key_A_O;

                case 'p':
                    return Key_A_P;

                case 'q':
                    return Key_A_Q;

                case 'r':
                    return Key_A_R;

                case 's':
                    return Key_A_S;

                case 't':
                    return Key_A_T;

                case 'u':
                    return Key_A_U;

                case 'v':
                    return Key_A_V;

                case 'w':
                    return Key_A_W;

                case 'x':
                    return Key_A_X;

                case 'y':
                    return Key_A_Y;

                case 'z':
                    return Key_A_Z;

                case '1':
                    return Key_F1;

                case '2':
                    return Key_F2;

                case '3':
                    return Key_F3;

                case '4':
                    return Key_F4;

                case '5':
                    return Key_F5;

                case '6':
                    return Key_F6;

                case '7':
                    return Key_F7;

                case '8':
                    return Key_F8;

                case '9':
                    return Key_F9;

                case '0':
                    return Key_F10;
            } /* switch */
            break;
    } /* switch */

    if(ch >= 127)     /* Treat special characters */
    {
        int assume_meta_key = 1;

        /* if the character has not been explicitly */
        /* enabled by the user, we check if it is a */
        /* Meta keystroke.                          */
        if(allowed_special_characters != NULL)
        {
            if(strchr((char *)allowed_special_characters, ch) != NULL)
            {
                assume_meta_key = 0;
            }
        }

        if(assume_meta_key)
        {
            if(ch == 127)
            {
                if(wnd_bs_127)
                {
                    ch = Key_BS;
                }
                else
                {
                    ch = Key_Del;
                }
            }
            else if(isalpha(ch - 128))
            {
                ch = meta_alphas[tolower(ch - 128) - 'a'];
            }
            else if(isdigit(ch - 128))
            {
                ch = meta_digits[ch - 128 - '0'];
            }
        }
    }

    return ch;
} /* TTGetKey */

void TTSendMsg(int msg, int x, int y, int msgtype)
{
    if(((ebufin + 1) % EBUFSZ) != ebufout)
    {
        EVent[ebufin].msg     = msg;
        EVent[ebufin].x       = x;
        EVent[ebufin].y       = y;
        EVent[ebufin].msgtype = msgtype;
        ebufin = (ebufin + 1) % EBUFSZ;
    }
}

int TTkopen(void)
{
    nonl();
    noecho();
    cbreak();
    nodelay(stdscr, FALSE);
    keypad(stdscr, TRUE);
    meta(stdscr, TRUE);
    intrflush(stdscr, FALSE);
    raw();
    query_termcap(tcflags);
    return 0;
}

int TTkclose(void)
{
    return 0;
}

void MouseOFF(void)
{}

void MouseON(void)
{}

void MouseInit(void)
{}

int GetMouInfo(int * x, int * y)
{
    unused(x);
    unused(y);
    return 0;
}

static void collect_events(void)
{
    int ch = TTGetKey();

    if(ch < 0)
    {
        return;
    }

    TTSendMsg(ch, 0, 0, WND_WM_CHAR);
}

int TTGetMsg(EVT * e)
{
    while(ebufin == ebufout)
    {
        collect_events();
    }
    e->msg     = EVent[ebufout].msg;
    e->x       = EVent[ebufout].x;
    e->y       = EVent[ebufout].y;
    e->msgtype = EVent[ebufout].msgtype;
    e->id      = 0;
    ebufout    = (ebufout + 1) % EBUFSZ;
    return e->msg;
}

int TTPeekQue(void)
{
    if(kbhit())
    {
        collect_events();
    }

    return ebufin != ebufout;
}

void TTClearQue(void)
{
    ebufin = ebufout;
}

int TTGetChr(void)
{
    EVT e;

    TTGetMsg(&e);
    return e.msg;
}

static char ansi2curses[8] =
{
    COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW,
    COLOR_WHITE
};
int TTopen(void)
{
    int i;

    initscr();
    color     = 0x07;
    vrow      = vcol = 0;
    term.NRow = getmaxy(stdscr);
    term.NCol = getmaxx(stdscr);

    if(has_colors())
    {
        start_color();

        for(i = 0; i < COLOR_PAIRS; i++)
        {
            init_pair(i, ansi2curses[i & 0x07], ansi2curses[(i & 0x38) >> 3]);
        }
        init_color(COLOR_RED, 0, 0, 1000);
        init_color(COLOR_BLUE, 1000, 0, 0);
    }

    TTkopen();
    return 1;
}

/*
 * Configure the terminal. Configuration is retained even after TTclose.
 *
 * The ANSI/VT100 terminal accepts the following configuration keywords:
 *
 * keyword        possible values
 *
 * highascii      A string of high ascii bytes that, if read from the key-
 *                board shall be reported verbatim to the calling application
 *                instead of being interpreted as combination of the Meta key
 *                with a low ASCII key. You will need to enable high ascii
 *                alphabet characters (like umlauts or cyrillics) with this.
 *                An empty string is the deafault.
 *
 * pseudographics Not processed here - currently to psg support for curses.
 *
 */
int TTconfigure(const char * keyword, const char * value)
{
    size_t l;

    if(!strcmp(keyword, "highascii"))
    {
        if(allowed_special_characters != NULL)
        {
            nfree(allowed_special_characters);
        }

        allowed_special_characters = (unsigned char *)malloc(l = (strlen(value) + 1));
        memcpy(allowed_special_characters, value, l);
    }
    else if(!strcmp(keyword, "pseudographics"))
    {
        if(atoi(value))
        {
            tcflags |= QUERY_ALTCHARSET;
        }
        else
        {
            tcflags &= ~QUERY_ALTCHARSET;
        }

        query_termcap(tcflags);
    }
    else
    {
        return 0;
    }

    return 1;
} /* TTconfigure */

int TTclose(void)
{
    TTkclose();
    endwin();
    return 1;
}

static int kbhit(void)
{
    fd_set select_set;
    struct timeval timeout;

    FD_ZERO(&select_set);
    FD_SET(0, &select_set);
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;
    select(FD_SETSIZE, &select_set, 0, 0, &timeout);
    return FD_ISSET(0, &select_set);
}

int dv_running(void)
{
    return 0;
}

#endif /* ifdef USE_CURSES */
