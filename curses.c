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

int color;
int vrow, vcol;
int cur_start = 0;
int cur_end = 0;
static unsigned char *allowed_special_characters = NULL;

#ifdef UNIX
volatile
#endif
TERM term =
{
    80,
    24,
    0
};

#define EBUFSZ 100
static EVT EVent[EBUFSZ];	/* event circular queue */
static int ebufin = 0;		/* event in */
static int ebufout = 0;		/* event out */

static int kbhit(void);

static int norefresh = 0;

static void ttrefresh(void)
{
    if (!norefresh)
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
    if (norefresh)
    {
        norefresh--;
        ttrefresh();
    }
}
    

/* This maps PC colors to monochrome attributes for w/o color support */
static int mono_colors[128]= 
{
    A_NORMAL,    A_NORMAL,    A_NORMAL,   A_NORMAL,
    A_NORMAL,    A_NORMAL,    A_NORMAL,   A_NORMAL,
    A_BOLD,      A_BOLD,      A_BOLD,     A_BOLD,
    A_BOLD,      A_BOLD,      A_BOLD,     A_BOLD,
    
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,  
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,
    
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,  
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,
    
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,  
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,
    
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,  
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,
    
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,  
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,
    
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,  
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,
    
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,  
    A_REVERSE,   A_REVERSE,   A_REVERSE,  A_REVERSE,
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  
    A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,  A_BOLD | A_REVERSE,
};

int TTScolor(unsigned int newcolor)
{
    int attr = 0;

    color = newcolor;
    if (!has_colors())
    {
        attr = mono_colors[color & 0x7F];
    }
    else
    {
	if (color & 0x08)
            attr |= A_DIM | A_BOLD;
	if (color & 0x80)
            attr |= A_BLINK;
	attr |= COLOR_PAIR(((color & 0x07) | ((color & 0x70) >> 1)));
    }
        
    attrset(attr);
    bkgdset(attr);
    
    return 1;
}

int TTCurSet(int st)
{
    if (st)
        curs_set(1);
    else
        curs_set(0);
    return 0;
}

int TTgotoxy(int row, int col)
{
    move(vrow = row, vcol = col);
    ttrefresh();
    return 1;
}

int TTgetxy(int* row, int* col)
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

int TTWriteStr(unsigned short *b, int len, int row, int col)
{
    int oldcol = color;
    int cut = 0;

    move(row, col);
    for (; len; len--, b++)
    {
        int ch = *b & 0xff;
        int col = (*b & 0xff00) >> 8;
        int y, i;

        /* control chars are written in ^X notation */
        if (ch < ' ')
        {
            switch (ch)
            {
            case '\t': 
                getyx(stdscr, y, i);
                i = ((y >> 3) + 1) << 3;
                cut += i-y;
                len -= i-y;
                break;
			
            case '\n':
            case '\r':
                len += cut;
                cut = 0;
                break;

            case '\b':
                if (cut > 0)
                {
                    len++;
                    cut--;
                }
                break;
		
            default:
                len--;	
                cut++;
            }
        }

        if (color != col)
            TTScolor(col);
        addch(ch);
    }

    if (color != oldcol)
        TTScolor(oldcol);
    move(vrow, vcol);
    
    ttrefresh();
    
    return 1;
}

int TTStrWr(unsigned char *s, int row, int col)
{
    int cut = 0;
    size_t len;
    int y,i;
    
    if (s == NULL)
        return 1;
    
    move(row, col);
    for (len = strlen((const char*)s); len; len--, s++)
    {
        /* control chars are written in ^X notation */
        if (*s < ' ')
        {
            switch (*s)
            {
            case '\t': 
                getyx(stdscr, y, i);
                i = ((y >> 3) + 1) << 3;
                cut += i-y;
                len -= i-y;
                break;
		
            case '\n':
            case '\r':
                len += cut;
                cut = 0;
                break;

            case '\b':
                if (cut > 0)
                {
                    len++;
                    cut--;
                }
                break;

            default:
                len--;	
                cut++;
            }
        }
        
        addch(*s);
    }
    
    move(vrow, vcol);
    ttrefresh();
    return 1;
}

int TTReadStr(unsigned short *b, int len, int row, int col)
{
    while(len--)
    {
        int ch;
        unsigned short cell;
        
        ch = mvinch(row, col);
        cell = ch & A_CHARTEXT;
        if (ch & (A_DIM | A_BOLD))
            cell |= 0x0800;
        if (ch & A_BLINK)
            cell |= 0x8000;
        cell |= (ch & 0x0700) | ((ch & 0x7000) << 1);
        
        *b++ = cell;
        col++;
    }
    move(vrow, vcol);
    ttrefresh();
    return 1;
}

int TTScroll(int x1, int y1, int x2, int y2, int lines, int dir)
{
#if 0
    /* 
     *  XXX  If you can make this work - mail me
     *  XXX  (Seems that TTClear doesn't work properly too)
     */
    int height = y2-y1+1;
    WINDOW* win;
    
    win = subwin(stdscr, height, x2-x1+1, y1, x1);
    if (win == NULL)
        return 0;
    scrollok(win, TRUE);
    wscrl(win, dir ? lines : -lines);
    touchline(stdscr, y1, height);
    wttrefresh(win);
    delwin(win);
#else
    int y;
    int width = x2 - x1 + 1;
    unsigned short* buf;
    
    if (lines <= 0)
        return 0;
    
    buf = malloc(width*sizeof(unsigned short));
    if (buf == NULL)
        return 0;
    
    if (dir)
    {
        for (y = y1+lines; y <= y2; y++)
        {
            TTReadStr(buf, width, y, x1);
            TTWriteStr(buf, width, y-lines, x1);
        }
        
        /* fill new lines with spaces */
        TTClear(x1, y2-lines+1, x2, y2);
    } 
    else
    {
        for (y = y2-lines; y >= y1; y--)
        {
            TTReadStr(buf, width, y, x1);
            TTWriteStr(buf, width, y+lines, x1);
        }
        
        /* fill new lines with spaces */
        TTClear(x1, y1, x2, y1+lines-1);
    }
    free(buf);
    move(vrow, vcol);
    ttrefresh();
#endif
    return 1;
}

int TTClear(int x1, int y1, int x2, int y2)
{
#if 0
    int height = y2 - y1 + 1;
    WINDOW* win;

    win = subwin(stdscr, height, x2 - x1 + 1, y1, x1);
    if (win == NULL)
        return 0;
    werase(win);
    touchline(stdscr, y1, height);
    wttrefresh(win);
    delwin(win);
#else
    int y;
    
    for (y = y1; y <= y2; y++)
    {
        int x;
        move(y, x1);
        for (x = x1; x <= x2; x++)
            addch(' ');
    }
    move(vrow, vcol);
    ttrefresh();
#endif
    return 1;
}

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
    Key_A_A, Key_A_B, Key_A_C, Key_A_D, Key_A_E, Key_A_F, Key_A_G,
    Key_A_H, Key_A_I, Key_A_J, Key_A_K, Key_A_L, Key_A_M, Key_A_N,
    Key_A_O, Key_A_P, Key_A_Q, Key_A_R, Key_A_S, Key_A_T, Key_A_U,
    Key_A_V, Key_A_W, Key_A_X, Key_A_Y, Key_A_Z
};

static unsigned meta_digits[] =
{
    Key_A_0, Key_A_1, Key_A_2, Key_A_3, Key_A_4,
    Key_A_5, Key_A_6, Key_A_7, Key_A_8, Key_A_9
};

#define resize_code 9999

unsigned int TTGetKey(void)
{
    int ch;


redo:

    ch = getch();
    switch (ch)
    {
#ifdef KEY_RESIZE
    case KEY_RESIZE:
        return resize_code;
#endif
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
        switch (tolower(ch))
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
        }
    }

    if (ch >= 127)    /* Treat special characters */
    {
        int assume_meta_key = 1;
 
                      /* if the character has not been explicitly */
                      /* enabled by the user, we check if it is a */
                      /* Meta keystroke.                          */
         
        if (allowed_special_characters != NULL)
        {
            if (strchr((char*)allowed_special_characters, ch) != NULL)
            {
                assume_meta_key = 0;
            }
        }

        if (assume_meta_key)
        {
            if (ch == 127)
            {
                if (wnd_bs_127)
                {
                    ch = Key_BS;
                }
                else
                {
                    ch = Key_Del;
                }
            }
            else if (isalpha(ch - 128))
            {
                ch = meta_alphas[tolower(ch - 128) - 'a'];
            }
            else if (isdigit(ch - 128))
            {
                ch = meta_digits[ch - 128 - '0'];
            }
        }
    }

    return ch;
}

void TTSendMsg(unsigned int msg, int x, int y, unsigned int msgtype)
{
    if (((ebufin + 1) % EBUFSZ) != ebufout)
    {
        EVent[ebufin].msg = msg;
        EVent[ebufin].x = x;
        EVent[ebufin].y = y;
        EVent[ebufin].msgtype = msgtype;
        ebufin = (ebufin + 1) % EBUFSZ;
    }
}

int TTkopen(void)
{
    noecho();
    cbreak();
    nodelay(stdscr, FALSE);
    keypad(stdscr, TRUE);
    meta(stdscr, TRUE);
    intrflush(stdscr, FALSE);
    raw();
    
    return 0;
}

int TTkclose(void)
{
    return 0;
}

void MouseOFF(void)
{
}

void MouseON(void)
{
}

void MouseInit(void)
{
}

int GetMouInfo(int *x, int *y)
{
    unused(x);
    unused(y);
    return 0;
}

int TTGetMsg(EVT * e)
{
    e->msg = TTGetKey();
    e->x = 0;
    e->y = 0;
    e->msgtype = WND_WM_CHAR;
    e->id = 0;
    if (e->msg == resize_code)
    {
        e->msg = 1;
        e->msgtype = WND_WM_RESIZE;
        term.NRow = getmaxy(stdscr);
        term.NCol = getmaxx(stdscr);
    }
        
    return e->msg;
}

int TTPeekQue(void)
{
    return kbhit();
}

void TTClearQue(void)
{
    while (TTPeekQue())
        TTGetKey();
}

int TTGetChr(void)
{
    return TTGetKey();
}

static char ansi2curses[8] = {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE
};

int TTopen(void)
{
    int i;
    
    initscr();
    nonl();
    
    color = 0x07;
    vrow = vcol = 0;
    term.NRow = getmaxy(stdscr);
    term.NCol = getmaxx(stdscr);
    
    if (has_colors())
    {
        start_color();
        
        for (i = 0; i < COLOR_PAIRS; i++)
            init_pair(i, ansi2curses[i & 0x07],
                      ansi2curses[(i & 0x38) >> 3]);
        init_color(COLOR_RED, 0, 0, 1000);
        init_color(COLOR_BLUE, 1000, 0, 0);
    }

    TTkopen();
    return 1;
}

/* With an ANSI/VT100 console, there is no way to distinguish between
   a national special character and a Meta- (Alt-)
   Keycombination. Therefore, the user has to explicitly tell us which
   keystrokes should be interpreted as national special characters and
   which should be treated as Alt-Keycombinations. */

void TTEnableSCInput(char *special_characters)
{
    if (allowed_special_characters != NULL)
    {
        free(allowed_special_characters);
    }
    allowed_special_characters = (unsigned char *) strdup(special_characters);
}

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
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    select(FD_SETSIZE, &select_set, 0, 0, &timeout);
    return FD_ISSET(0, &select_set);
}

int dv_running(void)
{
    return 0;
}

#endif






