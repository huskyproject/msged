/*
 *  ANSI.C
 *
 *  Written by Paul Edwards et al and released to the public
 *  domain.
 *
 *  Screen definitions & routines using ANSI codes.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(MSDOS) || defined(OS2) || defined(WINNT)
#include <conio.h>
#endif

#ifdef HAVE_CURSES              /* curses are presently only used for */
#include <curses.h>             /* terminal size (cols, rows) detection */
#endif

#ifdef UNIX

#include <termios.h>

static struct termios oldtios;

void block_console(int min, int time)
{
    struct termios tios;

    tcgetattr(0, &tios);
    tios.c_cc[VMIN] = min;   
    tios.c_cc[VTIME] = time;
    tcsetattr(0, 0, &tios);
}

#endif


#include "winsys.h"
#include "unused.h"

#ifdef UNIX
#include <unistd.h>
static int waiting = -1;
#include "keys.h"

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
#endif

int vcol, vrow;                 /* cursor position         */
int color = 7;                  /* current color on screen */
int cur_start = 0;
int cur_end = 0;
static unsigned char *allowed_special_characters = NULL;

#ifdef SASC
int akbhit(void);
int agetch(void);
int agetchr(void);
int coninit(void);
void confin(void);
#endif

TERM term =
{
    80,
#ifdef SASC
    30,
#else
    24,
#endif
    0
};

static unsigned char *scrnbuf;
static unsigned char *colbuf;

#define EBUFSZ 100
static EVT EVent[EBUFSZ];       /* event circular queue */
static int ebufin = 0;          /* event in */
static int ebufout = 0;         /* event out */

static int mykbhit(void);
static int FullBuffer(void);


static int ansi_foreground_colors[8]=
{
    30,  /* black   */
    34,  /* blue    */
    32,  /* green   */
    36,  /* cyan    */
    31,  /* red     */
    35,  /* magenta */
    33,  /* brown  (or yellow)    */
    37   /* light gray (or white) */
};

static int ansi_background_colors[8]=
{
    40,  /* black   */
    44,  /* blue    */
    42,  /* green   */
    46,  /* cyan    */
    41,  /* red     */
    45,  /* magenta */
    43,  /* brown   */
    47   /* gray    */
};
    
  
int TTScolor(unsigned int Attr)
{
    color = Attr;
    printf ("\x1b\x5b;2m");  /* reset intensify and bright attributes */

    printf ("\x1b\x5b");
    if (Attr & 0x08)
    {
        printf ("1;");  /* intensified foreground */
    }
    if (Attr & 0x80)
    {
        printf ("5;");  /* intensified background */
    }
    printf("%d;%dm", ansi_foreground_colors[Attr&0x7],
                     ansi_background_colors[(Attr >> 4) & 0x7]);
    
    /* fflush(stdout);   this is not necessary as long as no output is done */
    return 1;
}

int TTCurSet(int st)
{
    unused(st);
    return 0;
}

static int TTgotoxy_noflush(int row, int col)
{
    vrow = row;
    vcol = col;
    printf("\x1b[%d;%dH", row + 1 , col + 1);
    return 1;
}

int TTgotoxy(int row, int col)
{    
    vrow = row;
    vcol = col;
    printf("\x1b[%d;%dH", row + 1, col + 1);
    fflush(stdout);
    return 1;
}

int TTgetxy(int *row, int *col)
{
    *row = vrow;
    *col = vcol;
    return 1;
}

int TTPutChr(unsigned int Ch)
{
    putchar(Ch & 0xff);
    scrnbuf[vrow * term.NCol + vcol] = (Ch & 0xff);
    colbuf[vrow * term.NCol + vcol] = color;
    TTgotoxy(vrow, vcol);
    /*
    if (++vcol >= term.NCol)
    {
       vcol = 0;
       if (++vrow >= term.NRow)
       {
           vrow = term.NRow - 1;
       }
    }*/
    return 1;
}

int TTWriteStr(unsigned short *b, int len, int row, int col)
{
    int x;
    int thiscol, oldcol = color, restorecol = 0;

    TTgotoxy_noflush(row, col);
    for (x = 0; x < len; x++)
    {
        scrnbuf[vrow * term.NCol + vcol + x] = (*b & 0xff);
        thiscol = (*b & 0xff00U) >> 8;
        colbuf[vrow * term.NCol + vcol + x] = thiscol;
        if (thiscol != color)
        {
            TTScolor(thiscol);
            restorecol = 1;
        }
        putchar(*b & 0xff);
        b++;
    }
    if (restorecol)
    {
        TTScolor(oldcol);
    }
    TTgotoxy(row, col + len);
    return 1;
}

int TTStrWr(unsigned char *s, int row, int col)
{
    size_t len;
    len = strlen((char *)s);
    TTgotoxy_noflush(row, col);
    memcpy(&scrnbuf[vrow * term.NCol + vcol], s, len);
    memset(&colbuf[vrow * term.NCol + vcol], color % 256, len);
    printf("%s", s);
    TTgotoxy(row, col + len);
    return 1;
}

int TTReadStr(unsigned short *b, int len, int row, int col)
{
    int x;
    for (x = 0; x < len; x++)
    {
        b[x] = scrnbuf[row * term.NCol + col + x];
        b[x] |= (colbuf[row * term.NCol + col + x] << 8);
    }
    return 1;
}

int TTScroll(int x1, int y1, int x2, int y2, int lines, int Dir)
{
    int y, x, x3, orgcolor;
    int diff = x2 - x1 + 1;

    if (Dir)
    {
        while (lines-- > 0)
        {
            for (y = y1; y < y2; y++)
            {
                memcpy(&scrnbuf[y * term.NCol + x1],
                       &scrnbuf[(y + 1) * term.NCol + x1], diff);
                memcpy(&colbuf[y * term.NCol + x1],
                       &colbuf[(y + 1) * term.NCol + x1], diff);
            }
            memset(&scrnbuf[y2 * term.NCol + x1], ' ', diff);
            memset(&colbuf[y2 * term.NCol + x1], color, diff);
        }
    }
    else
    {
        while (lines-- > 0)
        {
            for (y = y2; y > y1; y--)
            {
                memcpy(&scrnbuf[y * term.NCol + x1],
                       &scrnbuf[(y - 1) * term.NCol + x1], diff);
                memcpy(&colbuf[y * term.NCol + x1],
                       &colbuf[(y - 1) * term.NCol + x1], diff);
            }
            memset(&scrnbuf[y1 * term.NCol + x1], ' ', diff);
            memset(&colbuf[y1 * term.NCol + x1], color, diff);
        }
    }
    
    orgcolor = color;
    
    for (y = y1; y <= y2; y++)
    {
        TTgotoxy_noflush(y, x1);
        x = x3 = x1;
        TTScolor(colbuf[y * term.NCol + x1]);
       
        while (x3 <= x2)
        {
            if (x3 < x2)
            {
                if (colbuf[y * term.NCol + x3 + 1] == color)
                {
                    ++x3;
                    continue;
                }
            }
            fwrite(&scrnbuf[y * term.NCol + x], x3 - x + 1, 1, stdout);
            if (x3 < x2)
            {
                TTScolor(colbuf[y * term.NCol + x3 + 1]);
            }
            x = ++x3;
        }
    }

    TTScolor(orgcolor);
    TTgotoxy_noflush(y1, x1);
    fflush(stdout);
    return 1;
}

int TTClear(int x1, int y1, int x2, int y2)
{
    int x, y;
    for (y = y1; y <= y2; y++)
    {
        TTgotoxy_noflush(y, x1);
        if (x2 >= term.NCol - 1)
        {
            TTEeol();
        }
        else
        {
            for (x = x1; x <= x2; x++)
            {
                putchar(' ');
            }
        }
        for (x = x1; x <= x2; x++)
        {
            scrnbuf[y * term.NCol + x] = ' ';
             colbuf[y * term.NCol + x] = color;
        }
    }
    TTgotoxy(y1, x1);
    return 1;
}

int TTEeol(void)
{
    printf("\x1b[K"); 
    fflush(stdout);
    return 1;
}

int TTdelay(int mil)
{
    unused(mil);
    return 0;
}

unsigned int TTGetKey(void)
{
    int ch;

 skip:
#if defined(OS2) || defined(WINNT)
    ch = getch();
#elif defined(UNIX)
    if (waiting != -1)
    {
        ch = waiting;
        waiting = -1;
    }
    else
    {
        block_console(1,0);
        ch = getchar();
        block_console(0,0);
    }
#elif defined(SASC)
    ch = agetchr();
#else
    ch = getchar();
#endif

#ifdef UNIX
    if (ch >= 127)    /* Treat special characters */
    {
        int assume_meta_key = 1;
 
                      /* if the character has not been explicitly */
                      /* enabled by the user, we check if it is a */
                      /* Meta keystroke.                          */
         
        if (allowed_special_characters != NULL)
        {
            if (strchr(allowed_special_characters, ch) != NULL)
            {
                assume_meta_key = 0;
            }
        }

        if (assume_meta_key)
        {
            if (isalpha(ch - 128))
            {
                ch = meta_alphas[tolower(ch - 128) - 'a'];
            }
            else if (isdigit(ch - 128))
            {
                ch = meta_digits[ch - 128];
            }
            else if (ch == 127)
            {
                ch = Key_Del;
            }
        }
    }
    else if (ch == 0x1b)   /* interprete VT100 escape sequences */
    {
        block_console(0,2);
        ch = getchar();
        block_console(0,0);
            
        if (ch == EOF)
        {
            clearerr(stdin);
        }
        if (ch <= 0)
        {
            ch = 0x1b;
        }
        else
        {
            switch (ch)
            {
            case 'a':
                ch = Key_A_A;
                break;
            case 'b':
                ch = Key_A_B;
                break;
            case 'c':
                ch = Key_A_C;
                break;
            case 'd':
                ch = Key_A_D;
                break;
            case 'e':
                ch = Key_A_E;
                break;
            case 'f':
                ch = Key_A_F;
                break;
            case 'g':
                ch = Key_A_G;
                break;
            case 'h':
                ch = Key_A_H;
                break;
            case 'i':
                ch = Key_A_I;
                break;
            case 'j':
                ch = Key_A_J;
                break;
            case 'k':
                ch = Key_A_K;
                break;
            case 'l':
                ch = Key_A_L;
                break;
            case 'm':
                ch = Key_A_M;
                break;
            case 'n':
                ch = Key_A_N;
                break;
            case 'o':
                ch = Key_A_O;
                break;
            case 'p':
                ch = Key_A_P;
                break;
            case 'q':
                ch = Key_A_Q;
                break;
            case 'r':
                ch = Key_A_R;
                break;
            case 's':
                ch = Key_A_S;
                break;
            case 't':
                ch = Key_A_T;
                break;
            case 'u':
                ch = Key_A_U;
                break;
            case 'v':
                ch = Key_A_V;
                break;
            case 'w':
                ch = Key_A_W;
                break;
            case 'x':
                ch = Key_A_X;
                break;
            case 'y':
                ch = Key_A_Y;
                break;
            case 'z':
                ch = Key_A_Z;
                break;
            case 72:
                ch = Key_Up;
                break;
            case 80:
                ch = Key_Dwn;
                break;
            case 73:
                ch = Key_PgUp;
                break;
            case 81:
                ch = Key_PgDn;
                break;
            case 71:
                ch = Key_Home;
                break;
            case 83:
                ch = Key_Del;
                break;
            case 75:
                ch = Key_Lft;
                break;
            case 77:
                ch = Key_Rgt;
                break;
            case 'O':  /* OS/2 telnet Alt key combinations */
                block_console(0,2);
                ch = getchar();
                block_console(0,0);
                switch(ch)
                {
                case 92:
                    ch = Key_A_Q;
                    break;
                case 44:
                    ch = Key_A_W;
                    break;
                case 37:
                    ch = Key_A_E;
                    break;
                case 34:
                    ch = Key_A_R;
                    break;
                case 59:
                    ch = Key_A_T;
                    break;
                case 62:
                    ch = Key_A_Z;
                    break;
                case 58:
                    ch = Key_A_U;
                    break;
                case 40:
                    ch = Key_A_I;
                    break;
                case 93:
                    ch = Key_A_O;
                    break;
                case 125:
                    ch = Key_A_P;
                    break;
                case 33:
                    ch = Key_A_A;
                    break;
                case 124:
                    ch = Key_A_S;
                    break;
                case 36:
                    ch = Key_A_D;
                    break;
                case 94:
                    ch = Key_A_F;
                    break;
                case 38:
                    ch = Key_A_G;
                    break;
                case 42:
                    ch = Key_A_H;
                    break;
                case 41:
                    ch = Key_A_J;
                    break;
                case 95:
                    ch = Key_A_K;
                    break;
                case 43:
                    ch = Key_A_L;
                    break;
                case 46:
                    ch = Key_A_Y;
                    break;
                case 60:
                    ch = Key_A_X;
                    break;
                case 35:
                    ch = Key_A_C;
                    break;
                case 39:
                    ch = Key_A_V;
                    break;
                case 64:
                    ch = Key_A_B;
                    break;
                case 132:
                    ch = Key_A_N;
                    break;
                case 91:
                    ch = Key_A_M;
                    break;
                case 48:
                    ch = Key_A_0;
                    break;
                case 49:
                    ch = Key_A_1;
                    break;
                case 50:
                    ch = Key_A_2;
                    break;
                case 51:
                    ch = Key_A_3;
                    break;
                case 52:
                    ch = Key_A_4;
                    break;
                case 53:
                    ch = Key_A_5;
                    break;
                case 54:
                    ch = Key_A_6;
                    break;
                case 55:
                    ch = Key_A_7;
                    break;
                case 56:
                    ch = Key_A_8;
                    break;
                case 57:
                    ch = Key_A_9;
                    break;
                default:
                    ch = 0x1B;
                    break;
                }
                break;
            case '[': /* "ansi" cursor key codes */
                block_console(0,2);
                ch = getchar();
                block_console(0,0);
                switch (ch)
                {
                case 'A':
                    ch = Key_Up;
                    break;
                case 'B':
                    ch = Key_Dwn;
                    break;
                case 'C':
                    ch = Key_Rgt;
                    break;
                case 'D':
                    ch = Key_Lft;
                    break;
                case 'F':
                    ch = Key_End;
                    break;
                case 'G':
                    ch = Key_PgDn;
                    break;
                case 'H':
                    ch = Key_Home;
                    break;
                case 'I':
                    ch = Key_PgUp;
                    break;
                case 'L':
                    ch = Key_Ins;
                    break;
                case '1':
                    block_console(0,2);
                    ch = getchar();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_Home; /* xterm Home ... */
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case '2':
                    block_console(0,2);
                    ch = getchar();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_Ins; /* xterm Insert ... */
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case '3':
                    block_console(0,2);
                    ch = getchar();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_Del; /* xterm Delete ... */
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case '4':
                    block_console(0,2);
                    ch = getchar();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_End; /* xterm End ... */
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case '5':
                    block_console(0,2);
                    ch = getchar();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_PgUp; /* xterm PgUp ... */
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case '6':
                    block_console(0,5);
                    ch = getchar();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_PgDn; /* xterm PgDn ... */
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case EOF:
                    clearerr(stdin);
                    ch = 0x1b;
                    waiting = '[';
                    break;
                default:
                    ch = 0x1b;
                    break;
                }
                break;
            default:
                goto skip;
            }
        }
    }
#endif

    if (ch == '\n')
    {
        ch = '\r';
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


int collect_events(int block)
{
    int msg;

    if (mykbhit() || block)
    {
        msg = TTGetKey();
        TTSendMsg(msg, 0, 0, WND_WM_CHAR);
    }

    return 0;
}

int TTkopen(void)
{
#ifdef UNIX
    struct termios tios;
#endif

#ifdef HAVE_CURSES
    WINDOW *scr = initscr();

    term.NRow = scr->_maxy;
    term.NCol = scr->_maxx;
    endwin();  /* close curses, we do not need them any more */
#endif    

#ifdef SASC
    coninit();
#endif

#ifdef UNIX
    tcgetattr(0, &tios);
    oldtios = tios;
    tios.c_lflag &= ~(ICANON | ISIG);
    tios.c_lflag &= ~ECHO;
    tios.c_cc[VMIN] = 0;     /* block_console(0); */
    tios.c_cc[VTIME] = 0;
    tcsetattr(0, 0, &tios);
    setbuf(stdin, NULL);
#endif

    scrnbuf = malloc(term.NRow * term.NCol);
    colbuf = malloc(term.NRow * term.NCol);
    memset(scrnbuf, ' ', term.NRow * term.NCol);
    memset(colbuf, 11, term.NRow * term.NCol);
    return 0;
}

int TTkclose(void)
{
#ifdef SASC
    confin();
    printf("\x1b[31m");
#else
    printf("\x1b[0m\x1b[1;1H\x1b[J"); 
#endif
#ifdef UNIX
    tcsetattr(0, 0, &oldtios);
#endif
    fflush(stdout);
    free(scrnbuf);
    free(colbuf);
    if (allowed_special_characters != NULL)
    {
        free(allowed_special_characters);
    }
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
    while (ebufin == ebufout)
    {
        collect_events(1);
    }
    e->msg = EVent[ebufout].msg;
    e->x = EVent[ebufout].x;
    e->y = EVent[ebufout].y;
    e->msgtype = EVent[ebufout].msgtype;
    e->id = 0;
    ebufout = (ebufout + 1) % EBUFSZ;
    return e->msg;
}

int TTPeekQue(void)
{
    collect_events(0);
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

int TTopen(void)
{
    vcol = vrow = 0;
    color = 0x07;
    TTkopen();
    return 1;
}

int TTclose(void)
{
    TTkclose();
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
    allowed_special_characters = strdup(special_characters);
}


static int mykbhit(void)
{
    int ret;
    if (FullBuffer())
    {
        return 0;
    }
#ifdef SASC
    ret = akbhit();
#elif defined UNIX
    if (waiting == -1)
    {
        waiting = getchar();
        if (waiting == EOF)
        {
            waiting = -1;
            clearerr(stdin);
        }
    }
    ret = (waiting != -1);
#else
    ret = kbhit();
#endif
    return ret;
}

int dv_running(void)
{
    return 0;
}

static int FullBuffer(void)
{
    if (((ebufin + 1) % EBUFSZ) != ebufout)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
