/*
 *  WINNTSCR.C
 *
 *  Written in August 1996 by Andrew Clarke and released to the public domain.
 *
 *  Screen definitions & routines for Windows NT.
 */

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "winsys.h"
#include "unused.h"

#define TERMDEF 1

#ifdef KEYDEBUG
#include <stdio.h>
static FILE *fTrace = NULL;
#endif

int vcol, vrow;                 /* cursor position         */
int color;                      /* current color on screen */
int cur_start = 0;
int cur_end = 0;

TERM term =
{
    80,
    25,
    0
};

#define EBUFSZ 25

static int ebufin = 0;          /* event in */
static int ebufout = 0;         /* event out */
static EVT EVent[EBUFSZ];       /* event circular queue */

static HANDLE HInput = INVALID_HANDLE_VALUE;
static HANDLE HOutput = INVALID_HANDLE_VALUE;
static unsigned long key_hit = 0xFFFFFFFFUL;

static int FullBuffer(void);

int TTopen(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    TTkopen();
    GetConsoleScreenBufferInfo(HOutput, &csbi);
    vcol = (int) csbi.dwCursorPosition.X;
    vrow = (int) csbi.dwCursorPosition.Y;
    term.NCol = (short) csbi.dwSize.X;
    term.NRow = (short) csbi.dwSize.Y;
    color = 0x07;
    return 1;
}

int TTclose(void)
{
    return 1;
}

/* under Windows, you can always input any special character without
   lossing the ability to distinguish it from an Alt-Keycombination,
   so the function TTEnableSCInput does not need to be implemented for
   everything except the ANSI/VT100 screen module */

void TTEnableSCInput(char *special_characters)
{
}


int TTgotoxy(int row, int col)
{
    COORD coord;
    coord.X = (SHORT) col;
    coord.Y = (SHORT) row;
    SetConsoleCursorPosition(HOutput, coord);
    vcol = col;
    vrow = row;
    return 1;
}

int TTPutChr(unsigned int Ch)
{
    DWORD len;
    COORD coord;
    WORD wattr;
    unsigned char wch;
    coord.X = (SHORT) vcol;
    coord.Y = (SHORT) vrow;
    wattr = color;
    wch = (unsigned char) Ch;
    WriteConsoleOutputCharacterA(HOutput, &wch, 1, coord, &len);
    WriteConsoleOutputAttribute(HOutput, &wattr, 1, coord, &len);
    TTgotoxy(vrow, vcol + 1);
    return 1;
}

static int mykbhit(int block)
{
    int iKey = 0;
    INPUT_RECORD irBuffer;
    DWORD pcRead;

    if (FullBuffer())
    {
        return 0;
    }

    if (key_hit != 0xFFFFFFFFUL)
    {
        return (int)key_hit;
    }

    memset(&irBuffer, 0, sizeof irBuffer);

    if (WaitForSingleObject(HInput, (block)? INFINITE : 0L) == 0)
    {
        ReadConsoleInput(HInput, &irBuffer, 1, &pcRead);

#ifdef KEYDEBUG
        if (irBuffer.EventType == KEY_EVENT)
        {
            fprintf(fTrace, "bKD=%d wRC=%hd wVKC=%hd wVSC=%hd c=%d dwCKS=%lx\n",
                    irBuffer.Event.KeyEvent.bKeyDown,
                    irBuffer.Event.KeyEvent.wRepeatCount,
                    irBuffer.Event.KeyEvent.wVirtualKeyCode,
                    irBuffer.Event.KeyEvent.wVirtualScanCode,
#ifdef __MINGW32__
                    irBuffer.Event.KeyEvent.AsciiChar;
#else
                    irBuffer.Event.KeyEvent.uChar.AsciiChar;
#endif
                    irBuffer.Event.KeyEvent.dwControlKeyState);
        }
#endif


        if (irBuffer.EventType == KEY_EVENT && irBuffer.Event.KeyEvent.bKeyDown != 0 && irBuffer.Event.KeyEvent.wRepeatCount <= 1)
        {
            WORD vk, vs, uc;
            BOOL fShift, fAlt, fCtrl;

            vk = irBuffer.Event.KeyEvent.wVirtualKeyCode;
            vs = irBuffer.Event.KeyEvent.wVirtualScanCode;
#ifdef __MINGW32__
            uc = irBuffer.Event.KeyEvent.AsciiChar;
#else
            uc = irBuffer.Event.KeyEvent.uChar.AsciiChar;
#endif
            fShift = (irBuffer.Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED));
            fAlt = (irBuffer.Event.KeyEvent.dwControlKeyState & (RIGHT_ALT_PRESSED + LEFT_ALT_PRESSED));
            fCtrl = (irBuffer.Event.KeyEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED + LEFT_CTRL_PRESSED));

            if (uc == 0)       /* function keys */
            {
                switch (vk)
                {
                case 0x21:     /* PgUp */
                    if (fCtrl)
                    {
                        vs = 0x84;  /* Ctrl+PgUp */
                    }
                    break;

                case 0x22:     /* PgDn */
                    if (fCtrl)
                    {
                        vs = 0x76;  /* Ctrl+PgDn */
                    }
                    break;

                case 0x23:     /* End */
                    if (fCtrl)
                    {
                        vs = 0x75;  /* Ctrl+End */
                    }
                    break;

                case 0x24:     /* Home */
                    if (fCtrl)
                    {
                        vs = 0x77;  /* Ctrl+Home */
                    }
                    break;

                case 0x25:     /* Left Arrow */
                    if (fCtrl)
                    {
                        vs = 0x73;  /* Ctrl+Left Arrow */
                    }
                    break;

                case 0x26:     /* Up Arrow */
                    if (fCtrl)
                    {
                        vs = 0x8d;  /* Ctrl+Up Arrow */
                    }
                    break;

                case 0x27:     /* Right Arrow */
                    if (fCtrl)
                    {
                        vs = 0x74;  /* Ctrl+Right Arrow */
                    }
                    break;

                case 0x28:     /* Down Arrow */
                    if (fCtrl)
                    {
                        vs = 0x91;  /* Ctrl+Down Arrow */
                    }
                    break;

                case 0x70:     /* F-Keys */
                case 0x71:
                case 0x72:
                case 0x73:
                case 0x74:
                case 0x75:
                case 0x76:
                case 0x77:
                case 0x78:
                case 0x79:
                    if (fAlt)
                    {
                        vs += 0x2D;  /* Alt+F-Key */
                    }
                    else if (fShift)
                    {
                        vs += 0x19;  /* Shift+F-Key */
                    }
                    break;
                }

                if (vk > 0x20 && vk < 0x92)  /* If it's OK use scan code */
                {
                    iKey = vs << 8;
                }
            }
            else
            {
                if (fAlt && (!fCtrl))       /* Alt+Key */
                {
                    iKey = vs << 8;
                }
                else if (fCtrl && (!fAlt)) /* Ctrl+Key */
                {
                    iKey = vk & 0xBF;
                }
                else
                {
                    iKey = uc;
                }
            }
        }
    }

    if (iKey != 0)
    {
        key_hit = iKey;
    }

    return (int)iKey;
}

unsigned int TTGetKey(void)
{
    int iKey;
    while (key_hit == 0xFFFFFFFFUL)
    {
        mykbhit(1);
    }
    iKey = key_hit;
    key_hit = 0xFFFFFFFFUL;
    return (unsigned int)iKey;
}

int TTScolor(unsigned int Attr)
{
    color = Attr;
    return 1;
}

int TTCurSet(int st)
{
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(HOutput, &cci);
    cci.bVisible = st;
    SetConsoleCursorInfo(HOutput, &cci);
    return 0;
}

int TTgetxy(int *row, int *col)
{
    *row = vrow;
    *col = vcol;
    return 1;
}

int TTdelay(int mil)
{
    Sleep((DWORD) mil);
    return 0;
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

int collect_events(int delay)
{
    if (mykbhit(0))
    {
        TTSendMsg(TTGetKey(), 0, 0, WND_WM_CHAR);
    }
    else if (delay)
    {
        TTdelay(50);
    }
    return 0;
}

int TTkopen(void)
{
    DWORD cmode;

    HInput = GetStdHandle(STD_INPUT_HANDLE);
    HOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleMode(HInput, (LPDWORD)&cmode);
    SetConsoleMode(HInput, cmode & (~ENABLE_PROCESSED_INPUT));

#ifdef KEYDEBUG
    fTrace = fopen("ntkeys.trc", "w");
    if (fTrace == NULL)
    {
        abort();
    }
#endif

    return 0;
}

int TTkclose(void)
{
    CloseHandle(HInput);
    HInput = INVALID_HANDLE_VALUE;
    CloseHandle(HOutput);
    HOutput = INVALID_HANDLE_VALUE;

#ifdef KEYDEBUG
    fclose(fTrace);
#endif

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

int TTStrWr(unsigned char *s, int row, int col)
{
    DWORD i, len;
    COORD coord;
    LPWORD pwattr;
    pwattr = malloc(strlen(s) * sizeof *pwattr);
    if (pwattr == NULL)
    {
        return 0;
    }
    coord.X = (SHORT) col;
    coord.Y = (SHORT) row;
    for (i = 0; i < strlen(s); i++)
    {
        *(pwattr + i) = color;
    }
    WriteConsoleOutputCharacterA(HOutput, s, (DWORD) strlen(s), coord, &len);
    WriteConsoleOutputAttribute(HOutput, pwattr, (DWORD) strlen(s), coord, &len);
    free(pwattr);
    TTgotoxy(row, col + len);
    return 1;
}

static void clearbox(int x1, int y1, int x2, int y2)
{
    COORD coord;
    LPWORD pwattr;
    char y, *pstr;
    DWORD i, len, width;
    width = x2 - x1 + 1;
    pwattr = malloc(width * sizeof *pwattr);
    if (pwattr == NULL)
    {
        return;
    }
    pstr = malloc(width);
    if (pstr == NULL)
    {
        free(pwattr);
        return;
    }
    for (i = 0; i < width; i++)
    {
        *(pwattr + i) = color;
        *(pstr + i) = ' ';
    }
    for (y = y1; y <= y2; y++)
    {
        coord.X = (SHORT) x1;
        coord.Y = (SHORT) y;
        WriteConsoleOutputCharacterA(HOutput, pstr, width, coord, &len);
        WriteConsoleOutputAttribute(HOutput, pwattr, width, coord, &len);
    }
    free(pwattr);
    free(pstr);
}

int TTClear(int x1, int y1, int x2, int y2)
{
    clearbox(x1, y1, x2, y2);
    TTgotoxy(y1, x1);
    return 1;
}

int TTEeol(void)
{
    clearbox(vcol, vrow, term.NCol -1, vrow);
    TTgotoxy(vcol, vrow);
    return 1;
}

int TTWriteStr(unsigned short *b, int len, int row, int col)
{
    DWORD i, wlen;
    COORD coord;
    LPWORD pwattr;
    unsigned char *pstr;

    pwattr = malloc(len * sizeof *pwattr);
    if (pwattr == NULL)
    {
        return 0;
    }
    pstr = malloc(len);
    if (pstr == NULL)
    {
        free(pwattr);
        return 0;
    }
    for (i = 0; i < len; i++)
    {
        *(pstr + i) = *b & 0xff;
        *(pwattr + i) = (*b & 0xff00U) >> 8;
        b++;
    }
    coord.X = (SHORT) col;
    coord.Y = (SHORT) row;
    WriteConsoleOutputCharacterA(HOutput, pstr, len, coord, &wlen);
    WriteConsoleOutputAttribute(HOutput, pwattr, len, coord, &wlen);
    free(pwattr);
    free(pstr);
    TTgotoxy(row, col + len);
    return 1;
}

int TTReadStr(unsigned short *b, int len, int row, int col)
{
    DWORD i, wlen;
    COORD coord;
    LPWORD pwattr;
    unsigned char *pstr;

    pwattr = malloc(len * sizeof *pwattr);
    if (pwattr == NULL)
    {
        return 0;
    }
    pstr = malloc(len);
    if (pstr == NULL)
    {
        free(pwattr);
        return 0;
    }
    coord.X = (SHORT) col;
    coord.Y = (SHORT) row;
    ReadConsoleOutputCharacterA(HOutput, pstr, len, coord, &wlen);
    ReadConsoleOutputAttribute(HOutput, pwattr, len, coord, &wlen);
    for (i = 0; i < len; i++)
    {
        b[i] = pstr[i];
        b[i] |= pwattr[i] << 8;
    }
    free(pwattr);
    free(pstr);
    return 1;
}

static void gettext(int x1, int y1, int x2, int y2, char *dest)
{
    DWORD i, len, width;
    COORD coord;
    LPWORD pwattr;
    char y, *pstr;
    width = x2 - x1 + 1;
    pwattr = malloc(width * sizeof *pwattr);
    if (pwattr == NULL)
    {
        return;
    }
    pstr = malloc(width);
    if (pstr == NULL)
    {
        free(pwattr);
        return;
    }
    for (y = y1; y <= y2; y++)
    {
        coord.X = (SHORT) x1;
        coord.Y = (SHORT) y;
        ReadConsoleOutputCharacterA(HOutput, pstr, width, coord, &len);
        ReadConsoleOutputAttribute(HOutput, pwattr, width, coord, &len);
        for (i = 0; i < width; i++)
        {
            *((unsigned char *)dest) = *(pstr + i);
            dest++;
            *((unsigned char *)dest) = (char)*(pwattr + i);
            dest++;
        }
    }
    free(pwattr);
    free(pstr);
}

static void puttext(int x1, int y1, int x2, int y2, char *srce)
{
    DWORD i, len, width;
    COORD coord;
    LPWORD pwattr;
    char y, *pstr;
    width = x2 - x1 + 1;
    pwattr = malloc(width * sizeof *pwattr);
    if (pwattr == NULL)
    {
        return;
    }
    pstr = malloc(width);
    if (pstr == NULL)
    {
        free(pwattr);
        return;
    }
    for (y = y1; y <= y2; y++)
    {
        for (i = 0; i < width; i++)
        {
            *(pstr + i) = *((unsigned char *)srce);
            srce++;
            *(pwattr + i) = *((unsigned char *)srce);
            srce++;
        }
        coord.X = (SHORT) x1;
        coord.Y = (SHORT) y;
        WriteConsoleOutputCharacterA(HOutput, pstr, width, coord, &len);
        WriteConsoleOutputAttribute(HOutput, pwattr, width, coord, &len);
    }
    free(pwattr);
    free(pstr);
}

int TTScroll(int x1, int y1, int x2, int y2, int lines, int Dir)
{
    int width, height;
    char *buf;

    width = x2 - x1 + 1;
    height = y2 - y1 + 1;
    buf = malloc(width * height * 2);
    if (buf == NULL)
    {
        return 0;
    }

    if (Dir)
    {
        gettext(x1, y1, x2, y2, buf);
        puttext(x1, y1, x2, y2 - lines, buf + (width * 2));
    }
    else
    {
        gettext(x1, y1, x2, y2, buf);
        puttext(x1, y1 + lines, x2, y2, buf);
    }

    free(buf);
    return 1;
}
