/*
 *  DIALOGS.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Contains all the static dialogs for Msged and functions that
 *  control/activate these dialogs.  Most of the functions are
 *  generic in the input they take.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "winsys.h"
#include "menu.h"
#include "main.h"
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "keys.h"
#include "misc.h"
#include "dialogs.h"

/* Initial colour value #defines */

#define f          BLACK | _LGREY
#define s          BLACK | _LGREY
#define bf         BLACK | _CYAN
#define bs         WHITE | _CYAN
#define bb         BLACK | _LGREY

char editbuf[255];

button dlg_ok = {ID_OK, 13, 15, 0, 0, bf, bs, bb, "  Ok  "};
button dlg_cancel = {ID_CANCEL, 25, 15, 0, 0, bf, bs, bb, "Cancel"};

ckbutton ck1 =  {3, 19, 1, 2, 0, 0, f, s, "Show Address"};
ckbutton ck2 =  {4, 19, 2, 2, 0, 0, f, s, "Hard Quoting"};
ckbutton ck3 =  {5, 19, 3, 2, 0, 0, f, s, "Write ^aMSGIDs"};
ckbutton ck4 =  {6, 19, 4, 2, 0, 0, f, s, "Opus Dates"};
ckbutton ck5 =  {7, 19, 5, 2, 0, 0, f, s, "Show SEEN-BYs"};
ckbutton ck6 =  {8, 19, 6, 2, 0, 0, f, s, "Confirm Actions"};
ckbutton ck7 =  {9, 19, 7, 2, 0, 0, f, s, "Use Lastread"};
ckbutton ck8 =  {10, 19, 8, 2, 0, 0, f, s, "Use ^aPID"};
ckbutton ck9 =  {11, 19, 9, 2, 0, 0, f, s, "Show CRs"};
ckbutton ck10 = {12, 19, 10, 2, 0, 0, f, s, "Show ImportFN"};

ckbutton ck21 = {13, 19, 11, 2, 0, 0, f, s, "Use SOT/EOT"};
ckbutton ck22 = {14, 19, 12, 2, 0, 0, f, s, "Show Time"};
ckbutton ck23 = {15, 19, 13, 2, 0, 0, f, s, "Msg#s At Top"};

ckbutton ck11 = {16, 40, 1, 25, 0, 0, f, s, "Save CC Msgs"};
ckbutton ck12 = {17, 40, 2, 25, 0, 0, f, s, "Raw CC Msgs"};
ckbutton ck13 = {17, 40, 3, 25, 0, 0, f, s, "Show Date Arvd"};
ckbutton ck14 = {19, 40, 4, 25, 0, 0, f, s, "Unused"};
ckbutton ck15 = {20, 40, 5, 25, 0, 0, f, s, "Show Real Msg#"};
ckbutton ck16 = {21, 40, 6, 25, 0, 0, f, s, "Show ^A Lines"};
ckbutton ck17 = {22, 40, 7, 25, 0, 0, f, s, "Chop Quotes"};
ckbutton ck18 = {23, 40, 8, 25, 0, 0, f, s, "Quote Quotes"};
ckbutton ck19 = {24, 40, 9, 25, 0, 0, f, s, "Show EOLs"};
ckbutton ck20 = {25, 40, 10, 25, 0, 0, f, s, "Status Bar"};

ckbutton ck24 = {26, 40, 11, 25, 0, 0, f, s, "Show System"};
ckbutton ck25 = {27, 40, 12, 25, 0, 0, f, s, "Ext Format"};
ckbutton ck26 = {28, 40, 13, 25, 0, 0, f, s, "Unused"};

dlgbox settings =
{
    10, 2, 68, 28,
    BLACK | _LGREY,
    BLACK | _LGREY,
    LCYAN | _LGREY,
    INSBDR | SHADOW,
    " System Switches ",
    28,
    {
        {D_CHK, 0, &ck1},
        {D_CHK, 0, &ck2},
        {D_CHK, 0, &ck3},
        {D_CHK, 0, &ck4},
        {D_CHK, 0, &ck5},
        {D_CHK, 0, &ck6},
        {D_CHK, 0, &ck7},
        {D_CHK, 0, &ck8},
        {D_CHK, 0, &ck9},
        {D_CHK, 0, &ck10},
        {D_CHK, 0, &ck21},
        {D_CHK, 0, &ck22},
        {D_CHK, 0, &ck23},
        {D_CHK, 0, &ck11},
        {D_CHK, 0, &ck12},
        {D_CHK, 0, &ck13},
        {D_CHK, 0, &ck14},
        {D_CHK, 0, &ck15},
        {D_CHK, 0, &ck16},
        {D_CHK, 0, &ck17},
        {D_CHK, 0, &ck18},
        {D_CHK, 0, &ck19},
        {D_CHK, 0, &ck20},
        {D_CHK, 0, &ck24},
        {D_CHK, 0, &ck25},
        {D_CHK, 0, &ck26},
        {D_BUT, 0, &dlg_ok},
        {D_BUT, 0, &dlg_cancel}}
};

/*
 *  Used by the GetString function.
 */

button strok = {ID_OK, 5, 3, 0, 0, bf, bs, bb, "  Ok  "};
button strcancel = {ID_CANCEL, 15, 3, 0, 0, bf, bs, bb, "Cancel"};
editf editfld = {ID_EDIT, 1, 1, 0, 0, f, f, editbuf, 20, 0};

dlgbox get_string =
{
    10, 5, 60, 20,
    BLACK | _LGREY,
    BLACK | _LGREY,
    LCYAN | _LGREY,
    INSBDR | SHADOW,
    NULL,
    3,
    {
        {D_EDT, 0, &editfld},
        {D_BUT, 0, &strok},
        {D_BUT, 0, &strcancel}
    }
};

/*
 *  Used by the ChoiceBox function.
 */

static button b_one = {ID_ONE, 0, 0, 0, 0, 0, 0, 0, NULL};
static button b_two = {ID_TWO, 0, 0, 0, 0, 0, 0, 0, NULL};
static button b_three = {ID_THREE, 0, 0, 0, 0, 0, 0, 0, NULL};

static dlgbox cb =
{
    0, 0, 0, 0,
    0, 0, 0,
    INSBDR | SHADOW,
    NULL,
    1,
    {
        {D_BUT, 0, (void *)&b_one},
        {D_BUT, 0, (void *)&b_two},
        {D_BUT, 0, (void *)&b_three}
    }
};

/*
 *  Menu at the top of the screen.
 */

MC MouseMnu =
{
    0, 0, 0, 0,
    CMD_HOR | CMD_PRT,
    NBDR,
    0,
    1,
    7,
    0,
    4,
    {
        {ID_SCAN, 0, scan_all_areas, NULL, "Scan", 'c', 0, 0},
        {ID_LIST, 0, dolist, NULL, "List", 'l', 0, 7},
        {ID_SETUP, 0, set_switch, NULL, "Setup", 's', 0, 14},
        {ID_QUIT, CMD_EXIT, NULL, NULL, "Quit", 'q', 0, 28}
    }
};

/*
 *  The module functions.
 */

void WriteSettings(void)
{
    SW->showaddr = ck1.down;
    SW->hardquote = ck2.down;
    SW->msgids = ck3.down;
    SW->opusdate = ck4.down;
    SW->showseenbys = ck5.down;
    SW->confirmations = ck6.down;
    SW->use_lastr = ck7.down;
    SW->usepid = ck8.down;
    SW->showcr = ck9.down;
    SW->importfn = ck10.down;
    SW->savecc = ck11.down;
    SW->rawcc = ck12.down;
    SW->datearrived = ck13.down;
    SW->showrealmsgn = ck15.down;
    SW->shownotes = ck16.down;
    SW->chopquote = ck17.down;
    SW->qquote = ck18.down;
    SW->showeol = ck19.down;
    SW->statbar = ck20.down;
    SW->soteot = ck21.down;
    SW->showtime = ck22.down;
    SW->dmore = ck23.down;
    SW->showsystem = ck24.down;
    SW->extformat = ck25.down;
}

void ReadSettings(void)
{
    ck1.down = (unsigned char)SW->showaddr;
    ck2.down = (unsigned char)SW->hardquote;
    ck3.down = (unsigned char)SW->msgids;
    ck4.down = (unsigned char)SW->opusdate;
    ck5.down = (unsigned char)SW->showseenbys;
    ck6.down = (unsigned char)SW->confirmations;
    ck7.down = (unsigned char)SW->use_lastr;
    ck8.down = (unsigned char)SW->usepid;
    ck9.down = (unsigned char)SW->showcr;
    ck10.down = (unsigned char)SW->importfn;
    ck11.down = (unsigned char)SW->savecc;
    ck12.down = (unsigned char)SW->rawcc;
    ck13.down = (unsigned char)SW->datearrived;
    ck14.down = 0;
    ck15.down = (unsigned char)SW->showrealmsgn;
    ck16.down = (unsigned char)SW->shownotes;
    ck17.down = (unsigned char)SW->chopquote;
    ck18.down = (unsigned char)SW->qquote;
    ck19.down = (unsigned char)SW->showeol;
    ck20.down = (unsigned char)SW->statbar;
    ck21.down = (unsigned char)SW->soteot;
    ck22.down = (unsigned char)SW->showtime;
    ck23.down = (unsigned char)SW->dmore;
    ck24.down = (unsigned char)SW->showsystem;
    ck25.down = (unsigned char)SW->extformat;
    ck26.down = 0;
}

int GetString(char *title, char *msg, char *buf, int len)
{
    WND *hCurr, *hWnd;
    int ret;

    if (len + 10 > maxx)
    {
        len = maxx - 10;
    }

    hCurr = WndTop();
    hWnd = WndPopUp(len + 8, 8, INSBDR | SHADOW, cm[IP_BTXT], cm[IP_NTXT]);

    WndWriteStr(1, 0, cm[IP_NTXT], msg);

    strcpy(editbuf, buf);

    editfld.len = len;
    editfld.curpos = strlen(buf);
    editfld.fattr = cm[IP_ETXT];
    editfld.sattr = cm[IP_ETXT];
    get_string.title = title;
    strok.x = len / 2 - 9;
    strcancel.x = len / 2 + 3;
    ret = DoDialog(&get_string, 1);

    switch (ret)
    {
    case ID_OK:
    case Key_Ent:
        strcpy(buf, editbuf);
        ret = 1;
        break;
    default:
        ret = 0;
        break;
    }
    WndClose(hWnd);
    WndCurr(hCurr);
    return ret;
}

/*
 *  Puts up a choice window, returning the ID of the button chosen.
 *  There can be only one line of text.  It works out the positioning of
 *  the buttons and text automagically.
 *
 *      title - title for the msgbox
 *      txt   - single line of text
 *      b1    - text for button 1
 *      b2    - text for button 2
 *      b3    - text for button 3
 *
 *  Returns ID of button pressed; this will be ID_ONE, ID_TWO or ID_THREE,
 *  or Key_Esc for an escape.
 */

void SetupButton(button * b, char *txt, int x, int y, unsigned char sel, unsigned char norm, unsigned char back)
{
    b->x = x;
    b->y = y;
    b->select = 0;
    b->down = 0;
    b->btext = txt;
    b->sattr = sel;
    b->fattr = norm;
    b->battr = back;
}

int ChoiceBox(char *title, char *txt, char *b1, char *b2, char *b3)
{
    WND *hCurr, *hWnd;
    int TextLen = 0;
    int ChoiceLen = 0;
    int wid, dep, pos, height, num = 0;

    if (txt != NULL)
    {
        TextLen = strlen(txt);
    }

    if (b1 != NULL)
    {
        ChoiceLen += strlen(b1) + 6;
        num++;
    }

    if (b2 != NULL)
    {
        ChoiceLen += strlen(b2) + 6;
        num++;
    }

    if (b3 != NULL)
    {
        ChoiceLen += strlen(b3) + 6;
        num++;
    }

    wid = (ChoiceLen > TextLen) ? ChoiceLen + 8 : TextLen + 8;
    dep = (txt == NULL) ? 6 : 7;

    hCurr = WndTop();
    hWnd = WndPopUp(wid, dep, INSBDR | SHADOW, cm[DL_BTXT], cm[DL_WTXT]);

    if (!hWnd)
    {
        return -1;
    }

    if (title != NULL)
    {
        WndTitle(title, cm[DL_BTXT]);
    }

    pos = (wid / 2) - (ChoiceLen / 2) - 1;
    height = (txt == NULL) ? 1 : 2;

    if (b1 != NULL)
    {
        SetupButton(&b_one, b1, pos, height, (unsigned char)cm[DL_BSEL],
          (unsigned char)cm[DL_BNRM], (unsigned char)cm[DL_BSHD]);
        pos += strlen(b1) + 6;
    }
    if (b2 != NULL)
    {
        SetupButton(&b_two, b2, pos, height, (unsigned char)cm[DL_BSEL],
          (unsigned char)cm[DL_BNRM], (unsigned char)cm[DL_BSHD]);
        pos += strlen(b2) + 6;
    }
    if (b3 != NULL)
    {
        SetupButton(&b_three, b3, pos, height, (unsigned char)cm[DL_BSEL],
          (unsigned char)cm[DL_BNRM], (unsigned char)cm[DL_BSHD]);
    }
    cb.num = num;

    if (txt != NULL)
    {
        WndPutsCen(0, cm[DL_BTXT], txt);
    }

    num = DoDialog(&cb, 1);

    WndClose(hWnd);
    WndCurr(hCurr);

    return num;
}


/*
 *  Sets the dialogs to the colours in the cm global array.
 */

void SetDlgColor(dlgbox * Dlg)
{
    int i;

    Dlg->fattr = cm[DL_WTXT];
    Dlg->sattr = cm[DL_WTXT];
    Dlg->battr = cm[DL_BTXT];

    for (i = 0; i < Dlg->num; i++)
    {
        switch (Dlg->ctrls[i].type)
        {
        case D_BUT:
            ((button *) Dlg->ctrls[i].ctl)->fattr = cm[DL_BNRM];
            ((button *) Dlg->ctrls[i].ctl)->sattr = cm[DL_BSEL];
            ((button *) Dlg->ctrls[i].ctl)->battr = cm[DL_BSHD];
            break;

        case D_CHK:
            ((ckbutton *) Dlg->ctrls[i].ctl)->fattr = cm[DL_CNRM];
            ((ckbutton *) Dlg->ctrls[i].ctl)->sattr = cm[DL_CSEL];
            break;

        case D_EDT:
            ((editf *) Dlg->ctrls[i].ctl)->fattr = cm[DL_ENRM];
            ((editf *) Dlg->ctrls[i].ctl)->sattr = cm[DL_ESEL];
            break;
        }
    }
}

void SetDialogColors(void)
{
    SetDlgColor(&settings);
    SetDlgColor(&get_string);
    cb.num = 3;
    SetDlgColor(&cb);
}
