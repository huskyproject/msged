/*
 *  IBMSCRN.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Screen definitions and routines for the IBM PC only; EGA/VGA support
 *  not using a commercial screen product.
 *
 *  Uses an event queue for mouse and keyboard input.  Currently doesn't ask
 *  ask for a DESQview buffer and doesn't give up any ticks - this should be
 *  implemented in some compiler independant fashion.
 */

#ifndef PACIFIC
#include <sys/timeb.h>
#include <bios.h>
#endif

#ifdef __DJGPP__
#include <dpmi.h>
#endif

#if defined(__TURBOC__) || defined(_MSC_VER)
#include <dos.h>
#endif

#if defined(__WATCOMC__)
#include <i86.h>
#include <dos.h>
#endif

#include <stdlib.h>

#include "winsys.h"
#include "vio.h"
#include "dosasm.h"
#include "unused.h"
#include "specch.h"

/* codepage 437 / 850 block graphics */
char *tt_specials="\272\315\311\273\310\274\263\304\332\277\300\331\261\020\021\334\337\030\031\024\035\000";

#define EBUFSZ 100
#define TERMDEF 1

#define NCOL 80
#define NROW 25

TERM term =
{
    NCOL,
    NROW,
    0,
};

unsigned int color;
int LButton = 0;
int RButton = 0;
int cur_start = 6;              /* these are ega/vga defaults */
int cur_end = 7;
static EVT EVent[EBUFSZ];       /* event circular queue */
static int ebufin = 0;          /* event in */
static int ebufout = 0;         /* event out */
static MOU mstatus = {0, 40, 12, 0, 0, 1, 1, 0};  /* mouse status */
static int mouse_avail = 0;
static int mousex = 40;
static int mousey = 12;

static int mykbhit(void);
static int FullBuffer(void);

void TTBeginOutput(void) {}
void TTEndOutput(void) {}

int TTScolor(unsigned int Attr)
{
    VIOsetfore(Attr & 0x000f);
    VIOsetback((Attr & 0x00f0) / 0x10);
    color = Attr & 0x00ff;
    return 0;
}

int TTCurSet(int st)
{
#ifdef __DJGPP__
    __dpmi_regs regs;
#else
    union REGS regs;
#endif

    if (st)
    {
        /* restore cursor */
        regs.h.ah = 0x01;
        regs.h.ch = cur_start;
        regs.h.cl = cur_end;
#if defined(__DJGPP__)
        __dpmi_int(0x10, &regs);
#elif defined(MSDOS) && (defined(__FLAT__)  || defined(__386__))
        int386(0x10, &regs, &regs);
#else
        int86(0x10, &regs, &regs);
#endif
    }
    else
    {
        /* hide cursor */
        regs.h.ah = 0x01;
        regs.h.ch = 0x20;
#if defined(__DJGPP__)
        __dpmi_int(0x10, &regs);
#elif defined(MSDOS) && (defined(__FLAT__) || defined(__386__))
        int386(0x10, &regs, &regs);
#else
        int86(0x10, &regs, &regs);
#endif
    }
    return 0;
}

int TTgotoxy(int row, int col)
{
    VIOgotoxy(col, row);
    VIOupdate();
    return 0;
}

int TTgetxy(int *row, int *col)
{
    *row = VIOwherex();
    *col = VIOwherey();
    return 0;
}

unsigned int TTGetKey(void)
{
    return obtkey();
}

int TTPutChr(unsigned int Ch)
{
    VIOputc(Ch);
    VIOupdate();
    return 0;
}

int TTWriteStr(unsigned long *b, int len, int row, int col)
{
    unsigned short *buf = malloc(len * sizeof(unsigned short));
    int i;

    if (buf != NULL)
    {
        for (i = 0; i < len; i++)
        {
            buf[i] =
                (unsigned short)((b[i] & 0xFFUL) | ((b[i] >> 8) & 0xFF00UL));
        }

        VIOputr(col, row, len, 1, buf);
        free(buf);
    }
    return 0;
}

int TTStrWr(unsigned char *s, int row, int col)
{
    unsigned long line[200];
    int i = 0;

    while (*s)
    {
        line[i] = ((unsigned long)(*s) & 0xffUL) | (((unsigned long)color) << 16);
        s++;
        i++;
    }
    TTWriteStr(line, i, row, col);
    return 0;
}

int TTReadStr(unsigned long *b, int len, int row, int col)
{
    int i;
    unsigned short * bc = (unsigned short *)b;

    VIOgetra(col, row, col + len - 1, row, bc);
    for (i = len - 1;  i >= 0; i--)
    {
        b[i] = MAKECELL(bc[i] & 0xFF, (bc[i] >> 8) & 0xFF);
    }
    return 0;
}

int TTClear(int x1, int y1, int x2, int y2)
{
    VIOclear(x1, y1, x2, y2);
    return 0;
}

int TTScroll(int x1, int y1, int x2, int y2, int lines, int Dir)
{
    if (Dir)
    {
        VIOscrollup(x1, y1, x2 + 1, y2, lines);
    }
    else
    {
        VIOscrolldown(x1, y1, x2 + 1, y2, lines);
    }
    return 0;
}

int TTEeol(void)
{
    int row, col;
    TTgetxy(&row, &col);
    TTScroll(col, row, NCOL - 1, row, 0, 1);
    return 0;
}

int TTdelay(int mil)
{
#ifdef __TURBOC__
    delay(mil);
#else
    unused(mil);
#endif
    return 0;
}

void TTSendMsg(int msg, int x, int y, int msgtype)
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


/*
 *  This is primarily the input section of the file. All messages
 *  are handled here.
 */

#define CLICKMAX      50  /* second max */
#define REPEAT_PAUSE  800
#define T_DOS         1
#define T_DV          2
#define T_DPMI        3

int ms_reset(int *mousetype);
int ms_show_cursor(void);
int ms_hide_cursor(void);
int ms_get_mouse_pos(int *horizpos, int *vertpos);

static unsigned long rtimer = -1;
static unsigned long ltimer = -1;
static int mtask = T_DOS;

void pause(void)
{
    switch (mtask)
    {
    case T_DOS:
        dospause();
        break;

    case T_DV:
        dvpause();
        break;

    case T_DPMI:
        dpmipause();
        break;

    default:
        break;
    }
}

static unsigned long hsec_time(void)
{
    static unsigned long old_id = 0;
    unsigned long i;
#if defined(__TURBOC__)
    struct time t;

    gettime(&t);

    i = (t.ti_sec * 100L) + t.ti_hund;
#elif defined(PACIFIC)
    i = 0;
#else
    struct dostime_t dtime;

    _dos_gettime(&dtime);
    i = (dtime.second * 100L) + dtime.hsecond;
#endif

    old_id = i;
    return i;
}


/*
 *  We should give up some ticks here.  Typically when polling this
 *  routine it is quite possible to loop through this in less than a
 *  millisecond.  Obviuosly this resolution is not needed :-)
 */

int collect_events(void)
{
    static int oy = 0, ox = 0;
    int moved = 0;
    int evt = 0;
    int ret;

    if (mouse_avail)
    {
        ret = ms_get_mouse_pos(&mousex, &mousey);

        if (mousex != ox || mousey != oy)
        {
            ox = mstatus.x = mousex;
            oy = mstatus.y = mousey;
            moved = 1;
        }

        if (mstatus.lrelease && (ret & 1))
        {
            evt = 1;
            mstatus.lbutton = 1;
            mstatus.lrelease = 0;
            ltimer = hsec_time() + CLICKMAX;
            TTSendMsg(MOU_LBTDN, ox, oy, WND_WM_MOUSE);
        }

        if (mstatus.lbutton && !(ret & 1))
        {
            evt = 1;
            mstatus.lbutton = 0;
            mstatus.lrelease = 1;
            TTSendMsg(MOU_LBTUP, ox, oy, WND_WM_MOUSE);
            if (hsec_time() < ltimer)
            {
                TTSendMsg(LMOU_CLCK, ox, oy, WND_WM_MOUSE);
            }
        }

        if (mstatus.rrelease && (ret & 2))
        {
            evt = 1;
            mstatus.rbutton = 1;
            mstatus.rrelease = 0;
            rtimer = hsec_time() + CLICKMAX;
            TTSendMsg(MOU_RBTDN, ox, oy, WND_WM_MOUSE);
        }

        if (mstatus.rbutton && !(ret & 2))
        {
            evt = 1;
            mstatus.rbutton = 0;
            mstatus.rrelease = 1;
            TTSendMsg(MOU_RBTUP, ox, oy, WND_WM_MOUSE);
            if (hsec_time() < rtimer)
            {
                TTSendMsg(RMOU_CLCK, ox, oy, WND_WM_MOUSE);
            }
        }

        if (!evt)
        {
            if (mstatus.lbutton || mstatus.rbutton)
            {
                if (mstatus.lbutton && hsec_time() > ltimer)
                {
                    TTSendMsg(LMOU_RPT, ox, oy, WND_WM_MOUSE);
                }

                if (mstatus.rbutton && hsec_time() > rtimer)
                {
                    TTSendMsg(RMOU_RPT, ox, oy, WND_WM_MOUSE);
                }

                if (moved)
                {
                    TTSendMsg(MOUSE_EVT, ox, oy, WND_WM_MOUSE);
                }
            }
        }

        if (mykbhit())
        {
            TTSendMsg(TTGetKey(), 0, 0, WND_WM_CHAR);
        }
    }
    else if (mykbhit())
    {
        TTSendMsg(TTGetKey(), 0, 0, WND_WM_CHAR);
    }

    return 0;
}

int TTGetMsg(EVT * event)
{
    while (ebufin == ebufout)
    {
        pause();
        collect_events();
    }

    event->msg = EVent[ebufout].msg;
    event->msgtype = EVent[ebufout].msgtype;
    event->x = EVent[ebufout].x;
    event->y = EVent[ebufout].y;
    event->id = 0;
    ebufout = (ebufout + 1) % EBUFSZ;

    return event->msg;
}

int TTPeekQue(void)
{
    collect_events();
    return (ebufin != ebufout);
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

void MouseON(void)
{
    if (mouse_avail)
    {
        ms_show_cursor();
    }
}

void MouseOFF(void)
{
    if (mouse_avail)
    {
        ms_hide_cursor();
    }
}

int GetMouInfo(int *x, int *y)
{
    if (mouse_avail)
    {
        ms_get_mouse_pos(&mousex, &mousey);
        *x = mousex;
        *y = mousey;
    }
    else
    {
        *x = *y = 0;
    }
    return 0;
}

int TTKopen(void)
{
    return 0;
}

int TTKclose(void)
{
    return 0;
}

int TTclose(void)
{
    if (mouse_avail)
    {
        ms_hide_cursor();
    }
    VIOclose();
    return 0;
}


#pragma warn -par

/*
 * Configure the terminal. This must be called *before* TTopen!
 *
 * The DOS/BIOS terminal does not need any configuration.
 *
 */

int TTconfigure(const char *keyword, const char *value)
{
    return 0;
}

#pragma warn +par


int dv_running(void)
{
    return dvcheck();
}

int TTopen(void)
{
    int m = 0;
    if (dv_running())
    {
        mtask = T_DV;
    }
    else if (dpmicheck())
    {
        mtask = T_DPMI;
    }

    VIOopen();
    term.NRow = VIOrows();
    term.NCol = VIOcolumns();
    TTScolor(0x07);
    mouse_avail = 0;
    if (!(term.Abil & NOMOUSE))
    {
        if (ms_reset(&m))
        {
            if (m != 0)
            {
                mouse_avail = 1;
                MouseON();
                ms_get_mouse_pos(&mousex, &mousey);
            }
        }
    }
    return 0;
}

static int mykbhit(void)
{
    if (FullBuffer())
    {
        return (0);
    }
    return kbdhit();
}

static int FullBuffer(void)
{
    return ((ebufin + 1) % EBUFSZ) != ebufout ? 0 : 1;
}
