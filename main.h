/*
 *  MAIN.H
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Contains global variables for use by Msged.  Most common functions
 *  are also prototyped here.
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#define CM_NTXT 0
#define CM_QTXT 1
#define CM_KTXT 2
#define CM_TTXT 3
#define CM_ITXT 4
#define CM_DTXT 5
#define CM_FTXT 6
#define CM_HTXT 7
#define CM_BTXT 8
#define CM_ETXT 9
#define CM_WTXT 10
#define CM_NINF 11              /* net information */

#define MN_BTXT 12              /* menu border */
#define MN_TTXT 13              /* menu title text */
#define MN_NTXT 14              /* menu normal text */
#define MN_STXT 15              /* menu selected text */


#define HP_BTXT 16              /* help window border text */
#define HP_TTXT 17              /* help window title text */
#define HP_NTXT 18              /* help normal text */
#define HP_HTXT 19              /* help highlighted text */

#define IN_BTXT 20
#define IN_NTXT 21

#define IP_BTXT 22
#define IP_NTXT 23
#define IP_ETXT 24

#define DL_BTXT 25              /* border */
#define DL_WTXT 26              /* window */
#define DL_CNRM 27              /* checkbox normal */
#define DL_CSEL 28              /* checkbox selected */
#define DL_ENRM 29              /* entry normal */
#define DL_ESEL 30              /* entry selected */
#define DL_BSEL 31              /* button selected */
#define DL_BNRM 32              /* button normal */
#define DL_BSHD 33              /* button shadow */

#define LS_BTXT 34              /* border */
#define LS_TTXT 35              /* title */
#define LS_NTXT 36              /* normal text */
#define LS_ITXT 37              /* info text */
#define LS_STXT 38              /* seletected text */

#define ID_MGRGT 100
#define ID_MGLFT 101
#define ID_SCRUP 102
#define ID_SCRDN 103
#define ID_QUIT 104
#define ID_EDIT 105
#define ID_SCAN 106
#define ID_AREA 107
#define ID_SETUP 108
#define ID_LNUP 109
#define ID_LNDN 110
#define ID_LIST 111             /* list */
#define ID_PARE 112             /* prev area */
#define ID_NARE 113             /* next area */

#define MNU_LEN 27

#ifdef INCL_MAIN

int cm[] = {
    /* color table */
    /* Main window colors */
    LGREY | _BLACK,             /* Normal message text */
    WHITE | _BLACK,             /* Quoted message text */
    WHITE | _BLACK,             /* Kludge/information message text */
    WHITE | _BLACK,             /* Template message text */
    BLACK | _LGREY,             /* Status bar */
    WHITE | _BLACK,             /* Divider line between header and message text */
    LGREY | _BLACK,             /* The header titles (From: etc) */
    LGREY | _BLACK,             /* The header text */
    BLACK | _LGREY,             /* A selected block */
    WHITE | _BLACK,             /* Fields being edited in the header */
    WHITE | _BLACK,             /* Warnings */
    WHITE | _BLACK,             /* Network information (area/address) */
    /* Menu colors */
    WHITE | _BLACK,             /* Menu border color */
    WHITE | _BLACK,             /* Menu title color */
    LGREY | _BLACK,             /* Unselected text */
    BLACK | _LGREY,             /* Selected text */
    /* Help colors */
    BLACK | _LGREY,             /* Help border */
    BLACK | _LGREY,             /* Help title text */
    BLACK | _LGREY,             /* Normal text */
    BLACK | _LGREY,             /* Help highlight text */
    /* Info colors */
    WHITE | _BLACK,             /* Border color */
    WHITE | _BLACK,             /* Normal text color */
    /* Input colors */
    BLACK | _LGREY,             /* Border color */
    BLACK | _LGREY,             /* Normal text color */
    BLACK | _LGREY,             /* Edit field color */
    /* Dialog colors */
    BLACK | _LGREY,             /* Dialog box borders */
    BLACK | _LGREY,             /* Dialog box window text */
    BLACK | _LGREY,             /* Dialog checkbox normal color */
    BLACK | _LGREY,             /* Dialog checkbox selected color */
    BLACK | _LGREY,             /* Dialog entry field normal color */
    BLACK | _LGREY,             /* Dialog entry field selected color */
    WHITE | _BLACK,             /* Dialog button selected color */
    BLACK | _LGREY,             /* Dialog button normal color */
    BLACK | _LGREY,             /* Dialog button shadow color */
    /* list colors */
    LGREY | _BLACK,             /* List border color */
    WHITE | _BLACK,             /* List title color */
    LGREY | _BLACK,             /* List normal text color */
    WHITE | _BLACK,             /* List information text color */
    BLACK | _LGREY              /* List selected text color */
};
WND * hMnScr;                   /* handle to main screen window */

#else // ifdef INCL_MAIN

extern int cm[];
extern WND * hMnScr;            /* handle to main screen window */
extern MC MouseMnu;

#endif // ifdef INCL_MAIN

void AreaScan(void);

/* functions from init.c */
int InitVars(void);
void DeinitMem(void);

#endif // ifndef __MAIN_H__
