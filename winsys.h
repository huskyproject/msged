/*
 *  WINSYS.H
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Header file for a system-independant windowing system.
 */

#ifndef __WINSYS_H__
#define __WINSYS_H__

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define WND_ERROR         -1
#define WND_NOERR         1
#define NBDR              0x01     /* No Border             */
#define SBDR              0x02     /* Single border         */
#define DBDR              0x04     /* Double Border         */
#define SHADOW            0x08     /* We have a shadow      */
#define INSBDR            0x10     /* Inset border          */
#define NOSAVE            0x20     /* don't save background */
#define NOMOUSE           0x40     /* we don't want a mouse */

#define MOUSE_EVT         0xff     /* Undefined event       */
#define RMOU_CLCK         0xfe     /* Right button clicked  */
#define LMOU_CLCK         0xfd     /* Left button clicked   */
#define LMOU_RPT          0xfc     /* Button repeating   */
#define RMOU_RPT          0xf7     /* Button repeating   */
#define MOU_LBTDN         0xfb     /* Left button pressed   */
#define MOU_RBTDN         0xfa     /* Right button pressed  */
#define MOU_LBTUP         0xf9     /* Left button released  */
#define MOU_RBTUP         0xf8     /* Right button released */

#define VERT_SCRL         0xf7     /* not used yet.. urk... */
#define HORZ_SCRL         0xf6
#define WM_SCRLFT         0xf5
#define WM_SCRRGT         0xf4
#define WM_SCRLUP         0xf3
#define WM_SCRLDN         0xf2
#define WM_SPGDN          0xf1
#define WM_SPGUP          0xef

/* colours */

#define BLACK             0x00     /* Black                 */
#define BLUE              0x01     /* Blue                  */
#define GREEN             0x02     /* Green                 */
#define CYAN              0x03     /* Cyan                  */
#define RED               0x04     /* Red                   */
#define MAGENTA           0x05     /* Magenta               */
#define BROWN             0x06     /* Brown                 */
#define LGREY             0x07     /* Light grey (white)    */
#define DGREY             0x08     /* Dark grey             */
#define LBLUE             0x09     /* Light blue            */
#define LGREEN            0x0A     /* Light green           */
#define LCYAN             0x0B     /* Light cyan            */
#define LRED              0x0C     /* Light red             */
#define LMAGENTA          0x0D     /* Light magenta         */
#define YELLOW            0x0E     /* Yellow                */
#define WHITE             0x0F     /* Intense white         */
#define INTENSE           0x08     /* Intensity bit         */

#define _BLACK            0x00     /* Black                 */
#define _BLUE             0x10     /* Blue                  */
#define _GREEN            0x20     /* Green                 */
#define _CYAN             0x30     /* Cyan                  */
#define _RED              0x40     /* Red                   */
#define _MAGENTA          0x50     /* Magenta               */
#define _BROWN            0x60     /* Brown                 */
#define _LGREY            0x70     /* White                 */
#define _DGREY            0x80     /* Blink/dark grey       */
#define _LBLUE            0x90     /* Blink/light blue      */
#define _LGREEN           0xA0     /* Blink/light green     */
#define _LCYAN            0xB0     /* Blink/light cyan      */
#define _LRED             0xC0     /* Blink/light red       */
#define _LMAGENTA         0xD0     /* Blink/light magenta   */
#define _YELLOW           0xE0     /* Blink/light yellow    */
#define _WHITE            0xF0     /* Blink/light white     */
#define _BLINK            0x80     /* Blink/intensity bit   */
#define _TRANSPARENT      0x100    /* transparent background, for Unix */

#define F_NORMAL         0x000     /* Normal Font    */
#define F_ALTERNATE      0x100     /* Alternate Font */

#define MAKECELL(ch, at) ((unsigned long)(((unsigned long)((unsigned char)(ch))) | \
                                          (((unsigned long)(at)) << 16)))
                                         

/* hotspot related stuff */

#define MAX_HOTS          30       /* max # of hot spots per group */
#define MAX_HOT_GROUP     40       /* max # of HotGroups */
#define WND_WM_MOUSE      0x01     /* we got a Mouse Message */
#define WND_WM_CHAR       0x02     /* we got a keyboard message */
#define WND_WM_RESIZE     0x03     /* the window has been resized */
#define WND_WM_COMMAND    0x0020   /* we got a recognized id (was 0x04) */

/* these are wid's reserved for useage - wids start at 20 */

#define WND_WN_MENU       1L
#define WND_WN_WIND       2L

/* kludge time! :-) */

/* defines local terminal characteristics */

typedef struct _term
{
    short NCol;                 /* Number Rows  0 org */
    short NRow;                 /* Number Columns 0 org */
    unsigned char Abil;         /* Abilities of Terminal */
}
TERM;

#ifdef UNIX     /* prevent a name clash with term.h */
#define term msged_term
#endif


#ifndef TERMDEF
#ifdef UNIX
volatile
#endif
extern TERM term;
#endif

typedef struct _window
{
    unsigned long wid;
    unsigned char x1;           /* screen coordinates (no shadow) */
    unsigned char y1;
    unsigned char x2;
    unsigned char y2;
    unsigned char wattr;        /* window forground attribute */
    unsigned char battr;        /* border background attr */
    unsigned char flags;        /* SHADOW DBDR SBDR IBDR TITLE */
    char *title;                /* pointer to title memory */
    unsigned long **buffer;    /* screen save buffer (screen behind) */
    struct _window *next;       /* pointer to window above this one */
    struct _window *prev;       /* pointer to window below this one */
}
WND;

/* Mouse event structure */
/* not used much now */

typedef struct _mou
{
    unsigned int event;         /* event number */
    int x;                      /* location x,y */
    int y;
    int lbutton;                /* left button down */
    int rbutton;                /* left button down */
    int lrelease;               /* left button has been released */
    int rrelease;               /* right button has been released */
    int repeat;                 /* repeat? */
}
MOU;

/* An input event */

typedef struct _event
{
    int x, y;                   /* location */
    int msgtype;                /* message type */
    int msg;                    /* message */
    int id;
}
EVT;

/*
 *  This is a group of hot-spots - they're usually pushed onto the
 *  global stack.  id MUST be unique for it to be effective.  Currently,
 *  the ID must be manually added; this may change in the future.
 */

typedef struct _hotgrp
{
    unsigned long wid;          /* window ID for this group.  0 = ignored */
    int num;                    /* number of hotspots in this group */
    struct
    {                           /* individual hotspots */
        int id;                 /* identifier for item */
        int x1;                 /* coordinates */
        int y1;
        int x2;
        int y2;
        void *ctl;              /* pointer to structure containing item */
    }
    harr[MAX_HOTS];
}
HotGroup;

/*
 *  Window terminal functions for use by higher modules.  Each
 *  system-dependant module should provide these functions.
 */

int  TTScolor(unsigned int Attr);
int  TTopen(void);
int  TTclose(void);
int  TTkopen(void);
int  TTkclose(void);
int  TTCurSet(int);
int  TTgotoxy(int row, int col);
int  TTgetxy(int *row, int *col);
void TTEnableSCInput(char *special_characters);
int  TTGetChr(void);
int  TTPutChr(unsigned int Ch);
int  TTWriteStr(unsigned long *b, int len, int row, int col);
int  TTStrWr(unsigned char *s, int row, int col);
int  TTReadStr(unsigned long *b, int len, int row, int col);
int  TTClear(int x1, int y1, int x2, int y2);
int  TTScroll(int x1, int y1, int x2, int y2, int lines, int Dir);
int  TTEeol(void);
int  TTdelay(int mil);
int  TTGetMsg(EVT * event);
unsigned int TTGetKey(void);
void  MouseOFF(void);
void  MouseON(void);
void  MouseInit(void);
void  MouseClose(void);
int  GetMouInfo(int *x, int *y);
void TTClearQue(void);
int  TTPeekQue(void);
void TTBeginOutput(void); /* suppresses screen drawing until ... */
void TTEndOutput(void);   /* .. ttendoutput is called. This improves speed! */ 

/* routines from the window module (system independant) */

WND *WndOpen(int x1, int y1, int x2, int y2, int Bdr, int BAttr, int Attr);
WND *WndPopUp(int wid, int dep, int Bdr, int BAttr, int NAttr);
void WndClose(WND * w);
void WndBox(int x1, int y1, int x2, int y2, int Attr, int type);
void WndTitle(const char *Str, int Attr);
void WndWriteStr(int x, int y, int Attr, char *Str);
void WndPutsCen(int y, int Attr, char *Str);
int WndPrintf(int x, int y, int Attr, char *Str, ...);
void WndPutsn(int x, int y, int len, int Attr, char *Str);
void WndScroll(int x1, int y1, int x2, int y2, int dir);
void WndCurr(WND * hWnd);
WND *WndTop(void);
void WndPutc(int Ch, int Attr);
void WndGotoXY(int x, int y);
void WndClear(int x1, int y1, int x2, int y2, int attr);
int WndGetLine(int x, int y, int len, char *buf, int Attr, int *pos, int nokeys, int fil, int disp, EVT * ev);
void WndClearLine(int y, int Att);
void WndFillField(int x, int y, int len, unsigned char ch, int Attr);
void WndGetRel(int x, int y, int *wx, int *wy);

/* Routines from system module (system independent) */

void PushHotGroup(HotGroup * New);
void PopHotGroup(void);
int LocateHotItem(int x, int y, unsigned long wid);
unsigned int MnuGetMsg(EVT * event, unsigned long wid);

extern int wnd_suppress_shadows;
extern int wnd_force_monochrome;
extern int wnd_bs_127;

extern int window_resized;


#endif
