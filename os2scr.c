/*
 *  OS2SCR.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Screen definitions & routines for the IBM PC under OS/2 only.  There
 *  are three threads of execution in this module; the mouse, keyboard
 *  and the main thread.
 *
 *  The mouse thread polls the OS/2 mouse subsystem - I know it *should*
 *  be blocked, but unfortunately a thread that is blocked cannot be
 *  killed and we have to explicitly release the mouse control for the
 *  current screen group (when we exit), if another application wants to
 *  be able to use the mouse.
 *
 *  The keyboard thread is blocked and only sends events when a key is
 *  pressed.
 */

#include "winsys.h"
#include "memextra.h"
#include "specch.h"

#define TERMDEF 1

#define NCOL 80
#define NROW 25

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define EBUFSZ    25
#define INCL_VIO
#define INCL_AVIO
#define INCL_KBD
#define INCL_MOU
#define INCL_DOSPROCESS
#define INCL_DOSINFOSEG
#include <os2.h>

#include "mcompile.h"

#ifndef KBDTRF_FINAL_CHAR_IN
#define KBDTRF_FINAL_CHAR_IN FINAL_CHAR_IN
#endif

/* codepage 437 / 850 block graphics */
char *tt_specials="\272\315\311\273\310\274\263\304\332\277\300\331\261\020\021\334\337\030\031\024\035\000";

TERM term =
{
    NCOL - 1,
    NROW - 1,
    0
};

int vcol, vrow;                 /* cursor position         */
int color;                      /* current color on screen */
int cur_start = 0;
int cur_end = 0;

static VIOMODEINFO fossil_data;

static MOUEVENTINFO *pMouEvent = 0;   /* Mouse Event storage */
static USHORT MouHandle;        /* handle for the mouse */
static USHORT MouAvail = 0;     /* mouse services available ? */
static MOU mstatus = {0, 39, 12, 0, 0, 1, 1, 0};
static EVT EVent[EBUFSZ];       /* event circular queue */
static int ebufin = 0;          /* event in */
static int ebufout = 0;         /* event out */

static int mykbhit(void);
static int FullBuffer(void);
static unsigned long hsec_time(void);

void TTBeginOutput(void) {}
void TTEndOutput(void) {}    

int TTScolor(unsigned int Attr)
{
    color = Attr & 0xFF; /* we don't need the F_ALTERNATE attribute! */
    return 1;
}

int TTCurSet(int st)
{
    static USHORT oldstart = 0, oldstop = 0;
    VIOCURSORINFO *pcur = xmalloc16(sizeof(VIOCURSORINFO));

    if (oldstart == 0)
    {
        if (cur_start != 0 && cur_end != 0)
        {
            /* save a user-defined cursor shape */
            oldstart = (USHORT) cur_start;
            oldstop = (USHORT) cur_end;
        }
        else
        {
            /* save the default cursor shape */
            VioGetCurType((PVIOCURSORINFO) pcur, 0);
            oldstart = pcur->yStart;
            oldstop = pcur->cEnd;
        }
    }

    if (st)
    {
        pcur->yStart = oldstart;
        pcur->cEnd = oldstop;
        pcur->attr = 0;
        pcur->cx = 0;
        VioSetCurType((PVIOCURSORINFO) pcur, 0);
    }
    else
    {
        pcur->yStart = oldstart;
        pcur->cEnd = oldstop;
        pcur->attr = 0xFFFF;
        pcur->cx = 0;
        VioSetCurType((PVIOCURSORINFO) pcur, 0);
    }

    xfree16(pcur);

    return 0;
}

int TTgotoxy(int row, int col)
{
    vrow = row;
    vcol = col;
    VioSetCurPos((short)row, (short)col, 0);
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
    unsigned int *d = xmalloc16(sizeof(unsigned int));

    *d = ((unsigned)Ch & 0xFF) | (color << 8);
    VioWrtCellStr((PCH) d, 2, (short)vrow, (short)vcol, 0);

    xfree16(d);

    return 1;
}

int TTWriteStr(unsigned long *b, int len, int row, int col)
{
    unsigned short *ptr16;
    int i;

    if (len == 0)
    {
        return 1;
    }

    ptr16 = xmalloc16((short)(len * 2));

    for (i = 0; i < len; i++)
    {
        ptr16[i] =
            (unsigned short)((b[i] & 0xFFUL) | ((b[i] >> 8) & 0xFF00UL));
    }

    VioWrtCellStr((PCH) ptr16, (short)(len * 2), (short)row, (short)col & 0xFF, 0);
    xfree16(ptr16);

    return 1;
}

int TTStrWr(unsigned char *s, int row, int col, int len)
{
    char *ptr16;

    if (len < 0)
        len = strlen((char *)s);

    if (len == 0)
    {
        return 0;
    }
    ptr16 = xmalloc16(len);

    memmove(ptr16, s, len);
    VioWrtCharStrAtt((char *)ptr16, (short)len, (short)row, (short)col,
                     (PBYTE) &color, 0);
    xfree16(ptr16);

    return 1;
}

int TTReadStr(unsigned long *b, int len, int row, int col)
{
    unsigned short l = (short)(len * 2);
    unsigned short *ptr16;
    int i;

    if (l == 0)
    {
        return 1;
    }
    ptr16 = xmalloc16(l);
    VioReadCellStr((PCH) ptr16, &l, (short)row, (short)col, 0);
    for (i = 0; i < len; i++)
    {
        b[i] = MAKECELL((ptr16[i] & 0xFF), ((ptr16[i] >> 8) & 0xFF));
    }
    xfree16(ptr16);

    return 1;
}

int TTScroll(int x1, int y1, int x2, int y2, int lines, int Dir)
{
    unsigned char *scroll_fill = xmalloc16(2);

    if (Dir)
    {
        y2 = min(y2, term.NRow);
        y1 = min(y1, term.NRow);
        x1 = min(x1, term.NCol);
        x2 = min(x2, term.NCol);
        scroll_fill[0] = ' ';
        scroll_fill[1] = (unsigned char)color;
        if (lines == 0)
        {
            lines = -1;
        }
        VioScrollUp((short)y1, (short)x1, (short)y2, (short)x2,
          (short)lines, (char *)scroll_fill, 0);
    }
    else
    {
        y2 = min(y2, term.NRow);
        y1 = min(y1, term.NRow);
        x1 = min(x1, term.NCol);
        x2 = min(x2, term.NCol);
        scroll_fill[0] = ' ';
        scroll_fill[1] = (unsigned char)color;
        if (lines == 0)
        {
            lines = -1;
        }
        VioScrollDn((short)y1, (short)x1, (short)y2, (short)x2,
          (short)lines, (char *)scroll_fill, 0);
    }
    xfree16(scroll_fill);
    return 1;
}

int TTClear(int x1, int y1, int x2, int y2)
{
    TTScroll(x1, y1, x2, y2, 0, 1);
    return 1;
}

int TTEeol(void)
{
    TTScroll(vcol, vrow, term.NCol - 1, vrow, 0, 1);
    return 1;
}

int TTdelay(int mil)
{
    DosSleep((long)mil);
    return (0);
}

unsigned int TTGetKey(void)
{
    KBDKEYINFO *pki = xmalloc16(sizeof(KBDKEYINFO));
    int retval;

    pki->chChar = pki->chScan = 0;
    KbdCharIn(pki, IO_WAIT, 0);

    if (pki->chChar == 0xe0 && (pki->fbStatus & 2))
    {
        if (pki->chScan)
        {
            pki->chChar = 0;      /* Force Scan return */
        }
        else
        {                       /* Get next block     */
            pki->chChar = 0;
            KbdCharIn(pki, IO_WAIT, 0);
            if (!pki->chScan)
            {                   /* Still no scan?     */
                pki->chScan = pki->chChar;  /* Move new char over */
                pki->chChar = 0;  /* Force its return  */
            }
            else
            {
                pki->chChar = 0;  /* Force new scan     */
            }
        }
    }
    if (pki->chScan == 0xe0 && (pki->fbStatus & 2))
    {
        if (!pki->chChar)
        {
            pki->chScan = 0;
            KbdCharIn(pki, IO_WAIT, 0);
            if (!pki->chScan)
            {                   /* Still no scan?     */
                pki->chScan = pki->chChar;  /* Move new char over */
                pki->chChar = 0;  /* Force its return  */
            }
            else
            {
                pki->chChar = 0;  /* Force new scan     */
            }
        }
        else
        {
            pki->chScan = 0;      /* Handle 0xe00d case */
        }
    }
    if (pki->chChar)
    {
        pki->chScan = 0;
    }

    retval = (unsigned int)((pki->chScan << 8) + (pki->chChar));
    xfree16(pki);
    return retval;
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

#define CLICKMAX                 50  /* second max */
#define PULSE_PAUSE              2
#define REPEAT_PAUSE             800

static unsigned long rtimer = 0;
static unsigned long ltimer = 0;
static unsigned long lpulse = 0;
static unsigned long rpulse = 0;

int collect_events(int delay)
{
    USHORT wait = MOU_NOWAIT;
    static int oy = 0, ox = 0;
    int moved = 0;
    int evt = 0;
    int temp1, temp2;


    if (MouAvail)
    {
        MouReadEventQue(pMouEvent, &wait, MouHandle);

        if (pMouEvent->time == 0)
        {
            if (mstatus.lbutton || mstatus.rbutton)
            {
                if (mstatus.lbutton && hsec_time() > ltimer && hsec_time() > lpulse)
                {
                    lpulse = hsec_time() + PULSE_PAUSE;
                    TTSendMsg(LMOU_RPT, ox, oy, WND_WM_MOUSE);
                }
                else if (delay)
                {
                    DosSleep(1L);
                }

                if (mstatus.rbutton && hsec_time() > rtimer && hsec_time() > rpulse)
                {
                    rpulse = hsec_time() + PULSE_PAUSE;
                    TTSendMsg(RMOU_RPT, ox, oy, WND_WM_MOUSE);
                }
                else if (delay)
                {
                    DosSleep(1L);
                }
            }
            else if (delay)
            {
                DosSleep(1L);
            }
        }
        else
        {
            temp1 = ((pMouEvent->fs & MOUSE_BN1_DOWN) || (pMouEvent->fs & MOUSE_MOTION_WITH_BN1_DOWN));
            temp2 = ((pMouEvent->fs & MOUSE_BN2_DOWN) || (pMouEvent->fs & MOUSE_MOTION_WITH_BN2_DOWN));
            evt = 0;

            if (pMouEvent->col != ox || pMouEvent->row != oy)
            {
                ox = mstatus.x = pMouEvent->col;
                oy = mstatus.y = pMouEvent->row;
            }
            moved = 1;

            if (mstatus.lrelease && temp1)
            {
                evt = 1;
                mstatus.lbutton = 1;
                mstatus.lrelease = 0;
                lpulse = hsec_time() + PULSE_PAUSE;
                ltimer = hsec_time() + CLICKMAX;
                TTSendMsg(MOU_LBTDN, ox, oy, WND_WM_MOUSE);
            }
            if (mstatus.lbutton && !temp1)
            {
                evt = 1;
                mstatus.lbutton = 0;
                mstatus.lrelease = 1;
                TTSendMsg(MOU_LBTUP, ox, oy, WND_WM_MOUSE);

                if (hsec_time() < ltimer)
                {
                    TTSendMsg(LMOU_CLCK, ox, oy, WND_WM_MOUSE);
                }

                ltimer = 0;
            }
            if (mstatus.rrelease && temp2)
            {
                evt = 1;
                mstatus.rbutton = 1;
                mstatus.rrelease = 0;
                rpulse = hsec_time() + PULSE_PAUSE;
                rtimer = hsec_time() + CLICKMAX;
                TTSendMsg(MOU_RBTDN, ox, oy, WND_WM_MOUSE);
            }
            if (mstatus.rbutton && !temp2)
            {
                evt = 1;
                mstatus.rbutton = 0;
                mstatus.rrelease = 1;
                TTSendMsg(MOU_RBTUP, ox, oy, WND_WM_MOUSE);

                if (hsec_time() < rtimer)
                {
                    TTSendMsg(RMOU_CLCK, ox, oy, WND_WM_MOUSE);
                }

                rtimer = 0;
            }
            if ((moved && evt) || ((mstatus.rbutton || mstatus.lbutton) && moved))
            {
                TTSendMsg(MOUSE_EVT, ox, oy, WND_WM_MOUSE);
            }
        }
        if (mykbhit())
        {
            TTSendMsg(TTGetKey(), 0, 0, WND_WM_CHAR);
        }
        else if (delay)
        {
            if (MouAvail)
            {
                DosSleep(1L);
            }
            else
            {
                DosSleep(50L);
            }
        }
    }
    else
    {
        if (mykbhit())
        {
            TTSendMsg(TTGetKey(), 0, 0, WND_WM_CHAR);
        }
        else if (delay)
        {
            if (MouAvail)
            {
                DosSleep(1L);
            }
            else
            {
                DosSleep(50L);
            }
        }
    }
    return 0;
}

int TTkopen(void)
{
    return (0);
}

int TTkclose(void)
{
    return (0);
}

void MouseOFF(void)
{
    NOPTRRECT *pmouRect = xmalloc16(sizeof(NOPTRRECT));

    if (!MouAvail)
    {
        return;
    }

    pmouRect->row = 0;
    pmouRect->col = 0;
    pmouRect->cRow = (short)(term.NRow - 1);
    pmouRect->cCol = (short)(term.NCol - 1);
    MouRemovePtr(pmouRect, MouHandle);
    xfree16(pmouRect);
}

void MouseON(void)
{
    if (!MouAvail)
    {
        return;
    }
    MouDrawPtr(MouHandle);
}

void MouseInit(void)
{
    USHORT MouMask = MOUSE_MOTION | MOUSE_MOTION_WITH_BN1_DOWN | MOUSE_BN1_DOWN | MOUSE_MOTION_WITH_BN2_DOWN | MOUSE_BN2_DOWN | MOUSE_MOTION_WITH_BN3_DOWN | MOUSE_BN3_DOWN;

    if (term.Abil & NOMOUSE)
    {
        return;
    }

    if (MouOpen(0L, &MouHandle))
    {
        MouAvail = 0;
        return;
    }

    MouAvail = 1;
    if (pMouEvent == 0)
    {
        pMouEvent = xmalloc16(sizeof(MOUEVENTINFO));
    }

    MouSetEventMask(&MouMask, MouHandle);
}

int GetMouInfo(int *x, int *y)
{
    if (!MouAvail)
    {
        return 0;
    }
    collect_events(0);
    *x = mstatus.x;
    *y = mstatus.y;
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

int TTopen(void)
{
    KBDINFO *pki = xmalloc16(sizeof(KBDINFO));
    fossil_data.cb = 2 * sizeof(fossil_data);
    VioGetMode(&fossil_data, 0);  /* Get mode info      */
    term.NCol = fossil_data.col;  /* Maximum 'X' value  */
    term.NRow = fossil_data.row;  /* Maximum 'Y' value  */
    pki->cb = sizeof(KBDINFO);      /* Set binary keyboard mode */
    pki->fsMask = KEYBOARD_BINARY_MODE;
    KbdSetStatus(pki, 0);
    vcol = vrow = 0;
    color = 0x07;
    TTkopen();
    MouseInit();
    xfree16(pki);
    return 1;
}

int TTclose(void)
{
    TTkclose();
    if (MouAvail)
    {
        if (pMouEvent != 0)
        {
            xfree16(pMouEvent);
            pMouEvent = 0;
        }

        MouClose(MouHandle);
    }
    return 1;
}


#pragma warn -par

/*
 * Configure the terminal. This must be called *before* TTopen!
 *
 * The OS/2 VIO terminal does not need any configuration.
 *
 */

int TTconfigure(const char *keyword, const char *value)
{
    return 0;
}

#pragma warn +par


static int mykbhit(void)
{
    KBDKEYINFO *pki;
    int retval;

    if (FullBuffer())
    {
        return (0);
    }

    pki = xmalloc16(sizeof(KBDKEYINFO));
    pki->fbStatus = 0;
    KbdPeek(pki, 0);
    retval = pki->fbStatus & KBDTRF_FINAL_CHAR_IN ? 1 : 0;
    xfree16(pki);
    return retval;
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

static unsigned long hsec_time(void)
{
    DATETIME pdt;
    DosGetDateTime(&pdt);
    return (pdt.year * 3214080000UL) + (pdt.month * 267840000L) +
      (pdt.day * 8640000L) + (pdt.hours * 360000L) +
      (pdt.minutes * 6000L) + (pdt.seconds * 100L) +
      pdt.hundredths;
}
