/*
 *  ANSI.C
 *
 *  Written by Paul Edwards et al and released to the public
 *  domain.
 *
 *  Adapted to the peculiarities of UNIX consoles by Tobias Ernst.
 *
 *  Screen definitions & routines using ANSI codes.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* The following variables define the minimum terminal
   size. Theoretically, the minimum terminal size should be
   1x1. However, large parts of Msged cannot cope with small terminal
   sizes (they will create bus errors and all such things), therefore,
   the routines in this module always report a minimum terminal
   size. Of course, if the terminal is smaller than this minimum size,
   the output will be garbage, but at least the program will not crash
   ... */

#define MINTERMX 10  /* 10 x 8 is the absolute minimum */
#define MINTERMY 8


#if defined(MSDOS) || defined(OS2) || defined(WINNT)
#include <conio.h>
#endif

#ifdef UNIX

#include <termios.h>            /* struct winsize */
#include <sys/ioctl.h>          /* ioctl.h        */
#include <stdio.h>              /* fileno         */
#include <unistd.h>             /* sleep          */
#include <signal.h>             /* signal ...     */
#include <setjmp.h>             /* longjmp        */

#ifdef sun
#include <curses.h>
#endif

static volatile int resize_pending = 0;
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>          /* used to differentiate BSD from SYSV  */
#endif                                

static struct termios oldtios;

FILE *fDebug = stderr;

void block_console(int min, int time)
{
    struct termios tios;

/*  if (min == 0 && time == 0)
    {
        time = 1;
    } */
    
    tcgetattr(0, &tios);
    tios.c_cc[VMIN] = min;   
    tios.c_cc[VTIME] = time;
    tcsetattr(0, 0, &tios);
}

#endif


#include "winsys.h"
#include "unused.h"

#ifdef UNIX
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

#ifdef UNIX
volatile
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

static unsigned char *scrnbuf, *colbuf;


#define EBUFSZ 100
static EVT EVent[EBUFSZ];       /* event circular queue */
static int ebufin = 0;          /* event in */
static int ebufout = 0;         /* event out */

static int mykbhit(void);
static int FullBuffer(void);

static char *mono_colors[128]=
{
  ";8", "", "", "", "", "", "","", 
  ";8", ";1", ";1", ";1", ";1", ";1", ";1", ";1",
  ";7", ";7;8", ";7", ";7", ";7", ";7", ";7", ";7",
  ";7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7",
  ";7", ";7", ";7;8", ";7", ";7", ";7", ";7", ";7",
  ";7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7",
  ";7", ";7", ";7", ";7;8", ";7", ";7", ";7", ";7",
  ";7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7",
  ";7", ";7", ";7", ";7", ";7;8", ";7", ";7", ";7",
  ";7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7",
  ";7", ";7", ";7", ";7", ";7", ";7;8", ";7", ";7",
  ";7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7",
  ";7", ";7", ";7", ";7", ";7", ";7", ";7;8", ";7",
  ";7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7",
  ";7", ";7", ";7", ";7", ";7", ";7", ";7", ";7;8",
  ";7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7", ";1;7",
};

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

    if (wnd_force_monochrome)
    {
        printf ("%c%c0%sm",0x1b,0x5b,mono_colors[Attr&0x7F]);
        return 1;
    }
    else
    {
        printf ("%c%c0m",0x1b,0x5b);  /* reset attributes */

        fputs ("\033[", stdout);
        if (Attr & 0x08)
        {
            fputs ("1;", stdout);  /* intensified foreground */
        }
        if (Attr & 0x80)
        {
            fputs ("5;", stdout);  /* intensified background */
        }
        printf("%d;%dm", ansi_foreground_colors[Attr&0x7],
               ansi_background_colors[(Attr >> 4) & 0x7]);
    
        return 1;
    }
}

int TTCurSet(int st)
{
    unused(st);
    return 0;
}

static int TTgotoxy_noflush(int row, int col)
{
    if (row >= term.NRow)
    {
        row = term.NRow - 1;
    }
    if (col >= term.NCol)
    {
        col = term.NCol - 1;
    }
    vrow = row;
    vcol = col;
    printf("\033[%d;%dH", row + 1 , col + 1);
    return 1;
}

int TTgotoxy(int row, int col)
{    
    if (row >= term.NRow)
    {
        row = term.NRow - 1;
    }
    if (col >= term.NCol)
    {
        col = term.NCol - 1;
    }
    vrow = row;
    vcol = col;
    printf("\033[%d;%dH", row + 1, col + 1);
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
        if (vrow >= term.NRow || vcol + x >= term.NCol)
        {
            break;  /* don't write out of area */
        }
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
    TTgotoxy(row, col + x);
    return 1;
}

int TTStrWr(unsigned char *s, int row, int col)
{
    size_t len;
    len = strlen((char *)s);

    if (row >= term.NRow || col >= term.NCol)
    {
        return 0;
    }
    if (col + len > term.NCol)
    {
        if ((len = term.NCol - col)<1)
        {
                return 0;
        }
    }
    
    TTgotoxy_noflush(row, col);
    memcpy(&scrnbuf[vrow * term.NCol + vcol], s, len);
    memset(&colbuf[vrow * term.NCol + vcol], color % 256, len);
    fputs(s, stdout);
    TTgotoxy(row, col + len);
    return 1;
}

int TTReadStr(unsigned short *b, int len, int row, int col)
{
    int x;
    if (row >= term.NRow || col >= term.NCol)
    {
        return 0;
    }
    if (col + len > term.NCol)
    {
        if ((len = term.NCol - col) < 1)
        {
            return 0;
        }
    }
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


    if (x1 < 0 || y1 <0 || x2 >= term.NCol || y2 >= term.NRow || lines<1)
    {
            abort();
        return 0;
    }

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
            if (y < term.NRow && x < term.NCol)
            {
                scrnbuf[y * term.NCol + x] = ' ';
                 colbuf[y * term.NCol + x] = color;
            }
        }
    }
    TTgotoxy(y1, x1);
    return 1;
}

int TTEeol(void)
{
    fputs("\033[K", stdout); 
    fflush(stdout);
    return 1;
}

int TTdelay(int mil)
{
    unused(mil);
    return 0;
}

static void TTRepaint();

int getkey()
{
#ifdef sun
  return getch();
#else
  return getchar();
#endif
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
        ch = getkey();
        block_console(0,0);
    }
#elif defined(SASC)
    ch = agetchr();
#else
    ch = getkey();
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
    else if (ch == 12)
    {
        TTRepaint();
        goto skip;
    }
    else if (ch == 0x1b)   /* interprete VT100 escape sequences */
    {
        block_console(0,2);
        ch = getkey();
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
            case 0x1B:  /* double escape */
                ch = Key_Esc;
                break;
            case ':':
                ch = Key_F1;
                break;
            case ';':
                ch = Key_F2;
                break;
            case '<':
                ch = Key_F3;
                break;
            case '=':
                ch = Key_F4;
                break;
            case '>':
                ch = Key_F5;
                break;
            case '?':
                ch = Key_F6;
                break;
            case '@':
                ch = Key_F7;
                break;
            case 'A':
                ch = Key_F8;
                break;
            case 'B':
                ch = Key_F9;
                break;
            case 'C':
                ch = Key_F10;
                break;
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
            case 'O':  /* VT100 and ANSI Alt- and Function keys */
                block_console(0,2);
                ch = getkey();
                block_console(0,0);
                switch(ch)
                {
                case 33:
                    ch = Key_A_A;
                    break;
                case 34:
                    ch = Key_A_R;
                    break;
                case 35:
                    ch = Key_A_C;
                    break;
                case 36:
                    ch = Key_A_D;
                    break;
                case 37:
                    ch = Key_A_E;
                    break;
                case 38:
                    ch = Key_A_G;
                    break;
                case 39:
                    ch = Key_A_V;
                    break;
                case 40:
                    ch = Key_A_I;
                    break;
                case 41:
                    ch = Key_A_J;
                    break;
                case 42:
                    ch = Key_A_H;
                    break;
                case 43:
                    ch = Key_A_L;
                    break;
                case 44:
                    ch = Key_A_W;
                    break;
                case 46:
                    ch = Key_A_Y;
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
                case 58:
                    ch = Key_A_U;
                    break;
                case 59:
                    ch = Key_A_T;
                    break;
                case 60:
                    ch = Key_A_X;
                    break;
                case 62:
                    ch = Key_A_Z;
                    break;
                case 64:
                    ch = Key_A_B;
                    break;
                case 69:
                    ch = Key_A_F5;
                    break;
                case 70:
                    ch = Key_A_F6;
                    break;
                case 71:
                    ch = Key_A_F7;
                    break;
                case 72:
                    ch = Key_A_F8;
                    break;
                case 73:
                    ch = Key_A_F9;
                    break;
                case 74:
                    ch = Key_A_F10;
                    break;
                case 75:
                    ch = Key_A_F1;
                    break;
                case 76:
                    ch = Key_A_F2;
                    break;
                case 78:
                    ch = Key_A_F4;
                    break;
                case 79:
                    ch = Key_A_F3;
                    break;
                case 80:
                    ch = Key_F1;
                    break;
                case 81:
                    ch = Key_F2;
                    break;
                case 82:
                    ch = Key_F3;
                    break;
                case 83:
                    ch = Key_F4;
                    break;
                case 84:
                    ch = Key_F5;
                    break;
                case 91:
                    ch = Key_A_M;
                    break;
                case 92:
                    ch = Key_A_Q;
                    break;
                case 93:
                    ch = Key_A_O;
                    break;
                case 94:
                    ch = Key_A_F;
                    break;
                case 95:
                    ch = Key_A_K;
                    break;
                case 97:
                    ch = Key_C_F1;
                    break;
                case 98:
                    ch = Key_C_F2;
                    break;
                case 99:
                    ch = Key_C_F3;
                    break;
                case 100:
                    ch = Key_C_F4;
                    break;
                case 101:
                    ch = Key_C_F5;
                    break;
                case 102:
                    ch = Key_C_F6;
                    break;
                case 103:
                    ch = Key_C_F7;
                    break;
                case 104:
                    ch = Key_C_F8;
                    break;
                case 105:
                    ch = Key_C_F9;
                    break;
                case 106:
                    ch = Key_C_F10;
                    break;
                case 110:
                    ch = Key_Del;
                    break;
                case 112:
                    ch = Key_Ins;
                    break;
                case 113:
                    ch = Key_End;
                    break;
                case 115:
                    ch = Key_PgDn;
                    break;
                case 119:
                    ch = Key_Home;
                    break;
                case 121:
                    ch = Key_PgUp;
                    break;
                case 124:
                    ch = Key_A_S;
                    break;
                case 125:
                    ch = Key_A_P;
                    break;
                case 132:
                    ch = Key_A_N;
                    break;

                default:
                    ch = 0x1B;
                    break;
                }
                break;
            case '[': /* "ansi" cursor key codes */
                block_console(0,2);
                ch = getkey();
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
                case 'M':
                    ch = Key_F1;
                    break;
                case 'N':
                    ch = Key_F2;
                    break;
                case 'O':
                    ch = Key_F3;
                    break;
                case 'P':
                    ch = Key_F4;
                    break;
                case 'Q':
                    ch = Key_F5;
                    break;
                case 'R':
                    ch = Key_F6;
                    break;
                case 'S':
                    ch = Key_F7;
                    break;
                case 'T':
                    ch = Key_F8;
                    break;
                case 'U':
                    ch = Key_F9;
                    break;
                case 'V':
                    ch = Key_F10;
                    break;
                case '[':  /* linux console F1 .. F5 */
                    block_console(0,2);
                    ch = getkey();
                    block_console(0,0);
                    switch(ch)
                    {
                    case 'A':
                        ch = Key_F1;
                        break;
                    case 'B':
                        ch = Key_F2;
                        break;
                    case 'C':
                        ch = Key_F3;
                        break;
                    case 'D':
                        ch = Key_F4;
                        break;
                    case 'E':
                        ch = Key_F5;
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case '1':
                    block_console(0,2);
                    ch = getkey();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_Home; /* xterm Home ... */
                        break;
                    case 53:
                        block_console(0,2);
                        ch = getkey();
                        block_console(0,0);
                        switch (ch)
                        {
                        case '~':
                            ch = Key_F5; /* xterm F5 */
                            break;
                        default:
                            goto skip;
                        }
                        break;
                    case 55:
                        block_console(0,2);
                        ch = getkey();
                        block_console(0,0);
                        switch (ch)
                        {
                        case '~':
                            ch = Key_F6; /* xterm / ANSI F6 */
                            break;
                        default:
                            goto skip;
                        }
                        break;
                    case 56:
                        block_console(0,2);
                        ch = getkey();
                        block_console(0,0);
                        switch (ch)
                        {
                        case '~':
                            ch = Key_F7; /* xterm / ANSI F7 */
                            break;
                        default:
                            goto skip;
                        }
                        break;
                    case 57:
                        block_console(0,2);
                        ch = getkey();
                        block_console(0,0);
                        switch (ch)
                        {
                        case '~':
                            ch = Key_F8; /* xterm / ANSI F8 */
                            break;
                        default:
                            goto skip;
                        }
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case '2':
                    block_console(0,2);
                    ch = getkey();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_Ins; /* xterm Insert ... */
                        break;
                    case 48:
                        block_console(0,2);
                        ch = getkey();
                        block_console(0,0);
                        switch (ch)
                        {
                        case '~':
                            ch = Key_F9; /* ansi/xterm F9 */
                            break;
                        default:
                            goto skip;
                        }
                        break;
                    case 49:
                        block_console(0,2);
                        ch = getkey();
                        block_console(0,0);
                        switch (ch)
                        {
                        case '~':
                            ch = Key_F10; /* ansi/xterm F10 */
                            break;
                        default:
                            goto skip;
                        }
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case '3':
                    block_console(0,2);
                    ch = getkey();
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
                    ch = getkey();
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
                    ch = getkey();
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
                    ch = getkey();
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
                case '7':
                    block_console(0,2);
                    ch = getkey();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_Home; /* rxvt Home ... */
                        break;
                    default:
                        goto skip;
                    }
                    break;
                case '8':
                    block_console(0,2);
                    ch = getkey();
                    block_console(0,0);
                    switch (ch)
                    {
                    case '~':
                        ch = Key_End; /* rxvt End ... */
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


static volatile int jump_on_resize = 0;
static jmp_buf jmpbuf;

static void collect_events(int block)
{
    int msg;

#ifdef UNIX
    while (block)
    {
        if (setjmp(jmpbuf))
        {
                block_console(0, 0);
                while (mykbhit()) waiting = -1;
        }
        if (waiting != -1) break;
        if (resize_pending != 0) break;

        block_console (1, 0);
        jump_on_resize = 1;
        waiting = getkey();
        jump_on_resize = 0;
        if (waiting == EOF)
        {
            waiting = -1;
            clearerr(stdin);
        }
    }
    block_console (0, 0);

    if (resize_pending != 0)
    {
        TTSendMsg(resize_pending, 0, 0, WND_WM_RESIZE);
        resize_pending = 0;
    }
#endif
    if (mykbhit()
#ifndef UNIX
        || block
#endif        
        )
    {
        msg = TTGetKey();
        TTSendMsg(msg, 0, 0, WND_WM_CHAR);
    }
        
}

#ifdef UNIX
void sigwinch_handler(int sig)
{
    int newcol, newrow, i, x, y;
    struct winsize w;
    unsigned char *newbuf, *oldbuf;

    ioctl(fileno(stderr), TIOCGWINSZ, &w);
    newcol = w.ws_col; newrow = w.ws_row;
    if (newcol < MINTERMX) newcol = MINTERMX;
    if (newrow < MINTERMY) newrow = MINTERMY;

        for (i = 0; i <= 1; i++)
        {
               /* +3 to provide buffer for incorrect calls to the 
                  Win... routines if the window is very small */
            newbuf = malloc((newcol + 3) * (newrow + 3));
            oldbuf = i ? scrnbuf : colbuf;
            
            if (newbuf == NULL)
            {
                TTkclose();
                abort();
            }

            for (y = 0; y < newrow; y++)
            {
                for (x = 0; x < newcol; x++)
                {
                    if (x >= term.NCol || y >= term.NRow)
                    {
                        newbuf[y * newcol + x] = i ? ' ' : 0;
                    }
                    else
                    {
                        newbuf[y * newcol + x] = oldbuf[y * term.NCol + x];
                    }
                }
            }
            if (i)
            {
                free(scrnbuf);
                scrnbuf = newbuf;
            }
            else
            {
                free(colbuf);
                colbuf = newbuf;
            }

        }
        term.NCol = newcol;
        term.NRow = newrow;
        TTRepaint();

    resize_pending = 1;

#ifndef BSD                     /* need to reinstall the handler */
    signal (sig, sigwinch_handler);
#endif

    if (jump_on_resize)
    {
            jump_on_resize = 0;
            longjmp(jmpbuf, 1);
    }
}
#endif

int TTkopen(void)
{
#ifdef UNIX
    struct termios tios;
    struct winsize w;

#if 1
    ioctl(fileno(stderr), TIOCGWINSZ, &w);

    if (w.ws_row > MINTERMY)
        term.NRow = w.ws_row;
    if (w.ws_col > MINTERMY)
	term.NCol = w.ws_col;
#endif
#endif    

    if (term.NRow < MINTERMY) term.NRow = MINTERMY;
    if (term.NCol < MINTERMX) term.NCol = MINTERMX;

#ifdef SASC
    coninit();
#endif

#ifdef UNIX
    tcgetattr(0, &tios);
    oldtios = tios;
    tios.c_lflag &= ~(ICANON | ISIG);
    tios.c_lflag &= ~ECHO;
    tcsetattr(0, 0, &tios);
    block_console(0,0);
    setbuf(stdin, NULL);
#ifdef sun
    initscr();  //curses initialization
    cbreak();
    noecho();
#endif

#endif
               /* +3 to provide buffer for incorrect calls to the 
                  Win... routines if the window is very small */
    scrnbuf = malloc((term.NRow + 3) * (term.NCol + 3));
    colbuf = malloc((term.NRow + 3) * (term.NCol + 3));
    if (scrnbuf == NULL || colbuf == NULL)
    {
        TTkclose();
        fprintf (stderr, "Out of memory!\n");
        abort();
    }
        
    memset(scrnbuf, ' ', term.NRow * term.NCol);
    memset(colbuf, 11, term.NRow * term.NCol);

#if 1
#ifdef UNIX
    signal (SIGWINCH, sigwinch_handler);    
#endif
#endif

    return 0;
}

static void TTRepaint(void)
{
    int x,y, xp, yp, oldcol, col, c;

    xp = vcol; yp = vrow; oldcol = col = color;
    TTgotoxy_noflush(0,0);
    fputs("\033[2J", stdout);
    
    for (y = 0; y < term.NRow; y++)
    {
        for (x = 0; x < term.NCol; x++)
        {
            if (colbuf[y * term.NCol + x] != col)
            {
                TTScolor(col = colbuf[y * term.NCol + x]);
            }
            if (x != term.NCol -1 || y != term.NRow -1)
            {
                c = scrnbuf[y * term.NCol + x];
                putchar(c);
            }
            else
            {
                fputs("\033[K", stdout);
            }
        }
    }

    TTgotoxy(yp, xp);
    TTScolor(oldcol);
    fflush(stdout);
}

int TTkclose(void)
{
#ifdef SASC
    confin();
    fputs("\033[31m", stdout);
#else
    fputs("\033[0m\033[1;1H\033[J", stdout); 
#endif
#ifdef UNIX
    signal (SIGWINCH, SIG_DFL);
    tcsetattr(0, 0, &oldtios);
#endif
    fflush(stdout);
    if (scrnbuf != NULL)
    {
        free(scrnbuf); scrnbuf = NULL;
    }
    if (colbuf  != NULL)
    {
        free(colbuf); colbuf = NULL;
    }
/*  if (allowed_special_characters != NULL)
    {
        free(allowed_special_characters);
        allowed_special_characters = NULL;
    } 
    we don't free this list, so that it is available in case the
    calling program should call TTclose and then TTopen in sequence
    (e.g. when it does a DOS shell)
*/
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
    allowed_special_characters = (unsigned char *) strdup(special_characters);
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
        waiting = getkey();

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
