/*
 *  DLGBOX.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  DialogBox Code.  Also includes the system HotSpot code.
 *
 *
 *  A dialog box consists of the main dlgbox structure, which defines
 *  colours and window position and the like, plus the dialog controls
 *  themselves (buttons, fields etc).  Each control has a corresponding
 *  HotSpot and ID, which allows any mouse press/release/movement to be
 *  related to a hotspot and corresponding control ID.  Other hotspots
 *  may be in the system, but note that if a hotspot is associated with
 *  a window, it will only be looked at if that window is current (on top).
 *
 *  This windowing system is NOT event based in the same manner as
 *  Windows.  It's done this way to make non-event based programs easily
 *  portable to this system; and as a result flexibility suffers.  If you
 *  want to add new types, you will have to recompile the affected
 *  modules.
 *
 *  Caveats: If a new ctrl type is wanted, it MUST be added here if it
 *    is to be processed by/for the DoDialog function.  You must also
 *    add the display function somwhere & link it in.
 *
 *  Functions to lookout for (that need mods):
 *    DoDialog(), ShowCtrl(), DoCursor(), GetYCoord(), BuildDialogHot().
 */

#define DLGBOX

#include <stdio.h>
#include <string.h>
#include "winsys.h"
#include "menu.h"
#include "keys.h"
#include "unused.h"
/* Global vars used by all functions */
static int Focus    = -1;
static int OldFocus = -1;
int ValidCtrl(int type)
{
    switch(type)
    {
        case D_WBX:
        case D_TXT:
            return 0;

        default:
            return 1;
    }
}

void BuildDialogHot(dlgbox * db, HotGroup * hot)
{
    WND * hCurr;
    int i, k, modx, mody;

    if(!db)
    {
        return;
    }

    hCurr = WndTop();

    if(hCurr == NULL)
    {
        return;
    }

    /*
     *  The dialog items are on a window, so we have to get the
     *  window's position relative to the screen.
     */
    if(hCurr->flags & SBDR)
    {
        modx = 1;
        mody = 1;
    }
    else
    {
        if(hCurr->flags & INSBDR)
        {
            modx = 3;
            mody = 2;
        }
        else
        {
            modx = 0;
            mody = 0;
        }
    }

    modx += hCurr->x1;
    mody += hCurr->y1;
    /* fill out the HotGroup */
    k        = i = 0;
    hot->wid = hCurr->wid;

    while(i < db->num)
    {
        if(ValidCtrl(db->ctrls[i].type))
        {
            switch(db->ctrls[i].type)
            {
                case D_CHK:
                    db->ctrls[i].id = ((ckbutton *)db->ctrls[i].ctl)->id;
                    hot->harr[k].id = ((ckbutton *)db->ctrls[i].ctl)->id;
                    hot->harr[k].x1 = ((ckbutton *)db->ctrls[i].ctl)->px + modx;
                    hot->harr[k].y1 = ((ckbutton *)db->ctrls[i].ctl)->y + mody;
                    hot->harr[k].x2 = ((ckbutton *)db->ctrls[i].ctl)->x + modx + 3;
                    hot->harr[k].y2 = ((ckbutton *)db->ctrls[i].ctl)->y + mody;
                    break;

                case D_BUT:
                    db->ctrls[i].id = ((button *)db->ctrls[i].ctl)->id;
                    hot->harr[k].id = ((button *)db->ctrls[i].ctl)->id;
                    hot->harr[k].x1 = ((button *)db->ctrls[i].ctl)->x + modx;
                    hot->harr[k].x2 = ((button *)db->ctrls[i].ctl)->x + modx +
                                      strlen(((button *)db->ctrls[i].ctl)->btext) + 4;
                    hot->harr[k].y1 = ((button *)db->ctrls[i].ctl)->y + mody;
                    hot->harr[k].y2 = ((button *)db->ctrls[i].ctl)->y + mody;
                    break;

                case D_EDT:
                    db->ctrls[i].id = ((editf *)db->ctrls[i].ctl)->id;
                    hot->harr[k].id = ((editf *)db->ctrls[i].ctl)->id;
                    hot->harr[k].x1 = ((editf *)db->ctrls[i].ctl)->x + modx;
                    hot->harr[k].x2 = ((editf *)db->ctrls[i].ctl)->x + modx +
                                      ((editf *)db->ctrls[i].ctl)->len;
                    hot->harr[k].y1 = ((editf *)db->ctrls[i].ctl)->y + mody;
                    hot->harr[k].y2 = ((editf *)db->ctrls[i].ctl)->y + mody;

                default:
                    break;
            } /* switch */
            k++;
        }
        else
        {
            db->ctrls[i].id = 0;
        }

        i++;
    }
    hot->num = k;
} /* BuildDialogHot */

/*
 *  A dummy MnuGetMsg used when an edit feild has focus, it handles all
 *  the msgs it understands and passes others back to the DoDialog()
 *  procedure.
 */
int DoEditField(EVT * event, editf * i, unsigned long wid)
{
    int ret;
    int pos = i->curpos;

    unused(wid);
    ret       = WndGetLine(i->x, i->y, i->len, i->buf, i->sattr, &pos, 1, 0, 0, event);
    i->curpos = pos;
    return ret;
}

/*
 *  Shows a dialog box controls.  Also used to show whether a control has
 *  been selected or not.
 */
static void ShowCtrl(ctrl * i, unsigned char sel)
{
    switch(i->type)
    {
        case D_EDT:
            ((editf *)i->ctl)->select = sel;
            ShowEditField((editf *)i->ctl);
            break;

        case D_BUT:
            ((button *)i->ctl)->select = sel;
            ShowButton((button *)i->ctl);
            break;

        case D_CHK:
            ((ckbutton *)i->ctl)->select = sel;
            ShowCkbutton((ckbutton *)i->ctl);
            break;

        case D_TXT:
            D_ShowTxt((textl *)i->ctl);
            break;

        case D_WBX:
            D_ShowWBox((wbox *)i->ctl);
            break;

        default:
            break;
    } /* switch */
} /* ShowCtrl */

/*
 *  Handles the placement of the cursor, if needed.
 */
void DoCursor(dlgbox * db, int foc)
{
    if(!db)
    {
        TTCurSet(0);
        return;
    }

    switch(db->ctrls[foc].type)
    {
        case D_CHK:
            WndGotoXY(((ckbutton *)db->ctrls[foc].ctl)->x + 2,
                      ((ckbutton *)db->ctrls[foc].ctl)->y);
            TTCurSet(1);
            break;

        default:
            TTCurSet(0);
            break;
    }
}

int GetPrevCtrl(dlgbox * db, int cur)
{
    int foc = cur;

    foc--;

    if(foc < 0)
    {
        foc = db->num - 1;
    }

    while(foc != cur && !ValidCtrl(db->ctrls[foc].type))
    {
        foc--;

        if(foc < 0)
        {
            foc = db->num - 1;
        }
    }
    return foc;
}

int GetNextCtrl(dlgbox * db, int cur)
{
    int foc = cur;

    foc++;

    if(foc == db->num)
    {
        foc = 0;
    }

    while(foc != cur && !ValidCtrl(db->ctrls[foc].type))
    {
        foc++;

        if(foc == db->num)
        {
            foc = 0;
        }
    }
    return foc;
}

int GetYCoord(ctrl * i)
{
    switch(i->type)
    {
        case D_BUT:
            return ((button *)i->ctl)->y;

        case D_CHK:
            return ((ckbutton *)i->ctl)->y;

        case D_EDT:
            return ((editf *)i->ctl)->y;

        default:
            return 0;
    }
}

int GetRightCtrl(dlgbox * db, int cur, int num)
{
    int y = GetYCoord(&db->ctrls[cur]);
    int i;

    for(i = cur + 1; i < num; i++)
    {
        if(GetYCoord(&db->ctrls[i]) == y && ValidCtrl(db->ctrls[i].type))
        {
            return i;
        }
    }

    for(i = 0; i < cur; i++)
    {
        if(GetYCoord(&db->ctrls[i]) == y && ValidCtrl(db->ctrls[i].type))
        {
            return i;
        }
    }
    return cur;
}

int GetLeftCtrl(dlgbox * db, int cur, int num)
{
    int y = GetYCoord(&db->ctrls[cur]);
    int i;

    for(i = cur - 1; i >= 0; i--)
    {
        if(GetYCoord(&db->ctrls[i]) == y && ValidCtrl(db->ctrls[i].type))
        {
            return i;
        }
    }

    for(i = num; i > cur; i--)
    {
        if(GetYCoord(&db->ctrls[i]) == y && ValidCtrl(db->ctrls[i].type))
        {
            return i;
        }
    }
    return cur;
}

int GetFocus(dlgbox * db, int id)
{
    int i;

    for(i = 0; i < db->num; i++)
    {
        if(db->ctrls[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

/*
 *  Process a dialogbox structure (dlgbox), incorporating mouse and
 *  keyboard support.  If there are buttons, it returns when one is
 *  pressed and returns the ID of the button.
 *
 *      db  - dialogbox structure to process
 *      wnd - use already open window?
 */
int DoDialog(dlgbox * db, int wnd)
{
    WND * hWnd, * hCurr = NULL;
    HotGroup Hot;
    int WAttr = db->fattr;
    int BAttr = db->battr;
    int done  = 0;              /* terminate condition */
    int i;                      /* loop var */
    int select = 0;
    int ret    = 0;
    int NewFoc;
    EVT event;
    int Msg;                    /* Key/Mou press */

    if(!wnd)
    {
        hCurr = WndTop();
        hWnd  = WndOpen(db->x1, db->y1, db->x2, db->y2, db->btype, BAttr, WAttr);

        if(hWnd == NULL)
        {
            WndCurr(hCurr);
            return WND_ERROR;
        }
    }
    else
    {
        hWnd = WndTop();
    }

    if(db->title)
    {
        WndTitle(db->title, BAttr);
    }

    for(i = 0; i < db->num; i++)
    {
        ShowCtrl(&db->ctrls[i], 0);
    }
    BuildDialogHot(db, &Hot);
    PushHotGroup(&Hot);
    Focus = 0;
    ShowCtrl(&db->ctrls[Focus], 1);
    DoCursor(db, Focus);
    TTClearQue();  /* clear input queue */

    while(!done)
    {
        OldFocus = Focus;

        if(db->ctrls[Focus].type == D_EDT)
        {
            Msg = DoEditField(&event, (editf *)db->ctrls[Focus].ctl, hWnd->wid);
        }
        else
        {
            Msg = MnuGetMsg(&event, hWnd->wid);
        }

        switch(event.msgtype)
        {
            case WND_WM_COMMAND: /* item pressed */
                NewFoc = GetFocus(db, event.id);

                if(NewFoc != -1)
                {
                    switch(Msg)
                    {
                        case LMOU_CLCK:
                            Focus = NewFoc;

                            if(db->ctrls[Focus].type != D_EDT)
                            {
                                select = TRUE;
                            }

                            break;

                        case MOU_LBTDN:
                            Focus = NewFoc;

                            if(db->ctrls[Focus].type == D_BUT)
                            {
                                if(!((button *)db->ctrls[Focus].ctl)->down)
                                {
                                    ((button *)db->ctrls[Focus].ctl)->down = 1;
                                    ShowCtrl(&db->ctrls[Focus], 1);
                                }
                            }

                            break;

                        case MOU_LBTUP:

                            if(db->ctrls[Focus].type == D_BUT)
                            {
                                if(NewFoc == Focus)
                                {
                                    select = TRUE;
                                }
                            }

                            break;

                        case LMOU_RPT:
                        case MOUSE_EVT:

                            if(NewFoc != Focus)
                            {
                                if(db->ctrls[Focus].type == D_BUT)
                                {
                                    if(((button *)db->ctrls[Focus].ctl)->down)
                                    {
                                        ((button *)db->ctrls[Focus].ctl)->down = 0;
                                        ShowCtrl(&db->ctrls[Focus], 1);
                                    }
                                }
                            }
                            else
                            {
                                if(db->ctrls[Focus].type == D_BUT)
                                {
                                    if(!((button *)db->ctrls[Focus].ctl)->down)
                                    {
                                        ((button *)db->ctrls[Focus].ctl)->down = 1;
                                        ShowCtrl(&db->ctrls[Focus], 1);
                                    }
                                }
                            }

                            break;
                    } /* switch */
                }

                break;

            case WND_WM_MOUSE: /* action in non-recognized area */

                switch(Msg) /* actual message */
                {
                    case MOU_LBTUP:

                        if(db->ctrls[Focus].type == D_BUT)
                        {
                            if(((button *)db->ctrls[Focus].ctl)->down)
                            {
                                ((button *)db->ctrls[Focus].ctl)->down = 0;
                                ShowCtrl(&db->ctrls[Focus], 1);
                            }
                        }

                        break;

                    case MOUSE_EVT:
                    case LMOU_RPT:

                        if(db->ctrls[Focus].type == D_BUT)
                        {
                            if(((button *)db->ctrls[Focus].ctl)->down)
                            {
                                ((button *)db->ctrls[Focus].ctl)->down = 0;
                                ShowCtrl(&db->ctrls[Focus], 1);
                            }
                        }

                        break;
                } /* switch */
                break;

            case WND_WM_CHAR: /* keyboard in use */

                switch(Msg)
                {
                    case Key_Up:
                    case Key_S_Tab:
                        Focus = GetPrevCtrl(db, Focus);
                        break;

                    case Key_Dwn:
                    case Key_Tab:
                        Focus = GetNextCtrl(db, Focus);
                        break;

                    case Key_Rgt:
                        Focus = GetRightCtrl(db, Focus, db->num);
                        break;

                    case Key_Lft:
                        Focus = GetLeftCtrl(db, Focus, db->num);
                        break;

                    case Key_Spc:
                    case Key_Ent:
                        select = TRUE;
                        break;

                    case Key_Esc:
                        done = TRUE;
                        ret  = Key_Esc;
                        break;

                    default:
                        break;
                } /* switch */
                break;

            default:
                break;
        } /* switch */

        if(OldFocus != Focus)
        {
            ShowCtrl(&db->ctrls[OldFocus], 0);
            ShowCtrl(&db->ctrls[Focus], 1);
        }

        DoCursor(db, Focus);

        if(select)
        {
            switch(db->ctrls[Focus].type)
            {
                case D_BUT:

                    if(event.msgtype == WND_WM_CHAR)
                    {
                        ((button *)db->ctrls[Focus].ctl)->down = 1;
                        ShowCtrl(&db->ctrls[Focus], 1);
                        TTdelay(100);
                    }

                    ((button *)db->ctrls[Focus].ctl)->down = 0;
                    ret  = db->ctrls[Focus].id;
                    done = TRUE;
                    ShowCtrl(&db->ctrls[Focus], 1);
                    break;

                case D_CHK:
                    ((ckbutton *)db->ctrls[Focus].ctl)->down =
                        (unsigned char)!((ckbutton *)db->ctrls[Focus].ctl)->down;
                    ShowCtrl(&db->ctrls[Focus], 1);
                    break;

                case D_EDT:
                    done = 1;
                    ret  = Key_Ent;
                    break;

                default:
                    break;
            } /* switch */
        }

        select = 0;
    }
    DoCursor(NULL, 0);
    PopHotGroup();

    if(!wnd)
    {
        WndClose(hWnd);
        WndCurr(hCurr);
    }

    return ret;
} /* DoDialog */
