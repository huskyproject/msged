/*
 *  MENU.H
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Definitions for menus, dialog boxes and hotspots.
 */

#ifndef __MENU_H__
#define __MENU_H__

#define MAXITMS    28           /* max number of dialog/menu items */

#define ID_ONE     999          /* IDs for the ChoiceBox function */
#define ID_TWO     998
#define ID_THREE   997
#define ID_OK      996          /* standard items */
#define ID_CANCEL  995

#define D_BUT      1            /* a button */
#define D_CHK      2            /* a radio button */
#define D_EDT      3            /* an edit field */
#define D_WBX      4            /* a box */
#define D_TXT      5            /* static text */

#define CMD_EXIT   0x01
#define CMD_RDRW   0x02
#define CMD_SUB    0x04
#define MOU_RET    0x08
#define CMD_HOR    0x16         /* horizontal menu */
#define CMD_VER    0x32         /* vertical menu */
#define CMD_PRT    0x64         /* use parent window */

/* for an edit line */

typedef struct _edit
{
    int id;
    int x;
    int y;
    unsigned char changed;
    unsigned char select;
    int fattr;
    int sattr;
    char *buf;                  /* buffer for text  */
    int len;                    /* length of display field */
    int curpos;                 /* cursor position */
}
editf;

/* a pushdown button */

typedef struct _button
{
    int id;                     /* id of the button */
    int x;                      /* start of the button */
    int y;                      /* start of the button */
    unsigned char down;         /* down? */
    unsigned char select;       /* selected? */
    int fattr;                  /* normal attribute */
    int sattr;                  /* selected attribute */
    int battr;                  /* shadow attribute */
    char *btext;                /* text for the button */
}
button;

/* a checkbox button, eg [x]/[ ] */

typedef struct _ckbutton
{
    int id;                     /* id of check button */
    int x;                      /* x location */
    int y;                      /* y location */
    int px;                     /* prompt x location */
    unsigned char down;         /* checked? */
    unsigned char select;       /* selected? */
    int fattr;                  /* normal attribute */
    int sattr;                  /* selected attribute */
    char *prtext;               /* prompt text */
}
ckbutton;

/* a line of text - no ID as not a control  */

typedef struct _text
{
    int x;
    int y;
    int fattr;
    char *text;
}
textl;

/* a box on the screen, relative to window - no ID */

typedef struct _box
{
    int x1;
    int y1;
    int x2;
    int y2;
    int type;
    int fattr;
    int tattr;
    char *title;
}
wbox;

/* a radio button - a set of choices for a variable */

/* not done */

typedef struct _sbar
{
    int id;                     /* id of scroll bar */
    int type;                   /* 0 = vert, 1 = horiz */
    int xy;                     /* x pos in window */
    int xy1;                    /* top pos */
    int xy2;                    /* bottom pos */
    int spos;                   /* slider position */
    int sattr;                  /* slider attribute */
    int battr;                  /* button attribute */
    int tattr;                  /* track attribute */
}
sbar;

/* a dialog box - a set of controls */

typedef struct _ctl
{
    int type;                   /* type of control */
    int id;                     /* id of control (same as ctrl, if one) */
    void *ctl;                  /* pointer to the control */
}
ctrl;

typedef struct _dialog
{
    int x1;                     /* window coordinates */
    int y1;
    int x2;
    int y2;
    int fattr;                  /* window forground attribute */
    int battr;                  /* window border attribute */
    int sattr;                  /* selected item attribute */
    int btype;                  /* border type (shadow etc) */
    char *title;                /* title of the window */
    int num;                    /* number of items */
    ctrl ctrls[MAXITMS];        /* the item array */
}
dlgbox;

/* These are straight menus _only_ No dialogs... */

struct _mc;
typedef struct _mc MC;

typedef struct _cmd
{
    int id;                     /* number of this item */
    int flags;                  /* any flags for the item - can be divider */
    void (*itmfunc) (void);     /* function it points to */
    MC *m_sub;                  /* submenu, if one */
    char *itmtxt;               /* text of the item */
    int hotkey;                 /* the hotkey for the item - lowercase */
    int row;                    /* row of the item */
    int col;                    /* row of the item */
}
cmd;

struct _mc
{
    int x1;                     /* window coords */
    int y1;
    int x2;                     /* width of window */
    int y2;                     /* height of window */
    int mode;                   /* horiz or vertical menu ? */
    int btype;                  /* border type */
    int parent;                 /* parent item handle */
    int indent;                 /* indent to display item on either side */
    int len;
    int cur;                    /* current item */
    int num;                    /* number of items in array */
    cmd cmdtab[25];             /* item array (max 25) */
};

/* dlgbox.c */

int ChoiceBox(char *title, char *txt, char *b1, char *b2, char *b3);
int DoDialog(dlgbox * db, int wnd);

/* control.c */

char *MakeButton(button * b, int sel);
void ShowButton(button * b);
void ShowCkbutton(ckbutton * i);
void ShowEditField(editf * i);
void D_ShowTxt(textl * i);
void D_ShowWBox(wbox * i);

/* mnu.c */

int ProcessMenu(MC * m, EVT * event, int show);
void MnuSetColours(int nbc, int nnc, int nsc);

/* menu.c */

#define SELBOX_REPLYOTH  1
#define SELBOX_MOVEMSG   2
#define SELBOX_ADDRESS   3
#define SELBOX_NODELIST  4
#define SELBOX_USERNAME  5
#define SELBOX_WRTMODE   6
#define SELBOX_WRTOVER   7
#define SELBOX_LINKTO    8
#define SELBOX_CHARSET   9
#define SELBOX_GROUP    10


int DoMenu(int x1, int y1, int x2, int y2, char **Itms, int def, int selbox_id, char *topMsg);
int SelBox(char **Itms, int y1, int y2, int len, int def, WND * hPrev, WND * hWnd, int Sel, int Norm, int selbox_id, char *topMsg);

extern char **alist2;

#endif
