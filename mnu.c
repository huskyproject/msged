/*
 *  MNU.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Routines to display horizontal and vertical menus with full drag/browse
 *  mouse support.
 */

#include <string.h>
#include "winsys.h"
#include "keys.h"
#include "menu.h"

int PullDown = 0;
int ExitType = 0;

static int bc = WHITE | _CYAN;
static int nc = BLACK | _CYAN;
static int sc = WHITE | _BLACK;

static int GetFocus(cmd * cmdtab, int num, int id)
{
    int i;

    for (i = 0; i < num; i++)
    {
        if (cmdtab[i].id == id)
        {
            return i;
        }
    }

    return -1;
}

static void DispItem(cmd * c, unsigned char attr, int indent, int slen, int type)
{
    char text[255];
    int len;

    if (!c || !c->itmtxt)
    {
        return;
    }

    len = strlen(c->itmtxt);
    memset(text, ' ', slen + 2);
    strncpy(text + indent, c->itmtxt, len);
    *(text + slen) = '\0';
    if (type == CMD_VER)
    {
        WndPutsn(0, c->row, slen, attr, text);
    }
    else
    {
        WndPutsn(c->col, c->row, slen, attr, text);
    }
}

static int NextItem(int num, int cur)
{
    cur++;
    if (cur == num)
    {
        cur = 0;
    }
    return cur;
}

static int PrevItem(int num, int cur)
{
    cur--;
    if (cur < 0)
    {
        cur = num - 1;
    }
    return cur;
}

/*
 *  Displays a horizontal menu in a window, according to the info in
 *  the passed MC structure.
 *
 *  The current mouse position is passed to the function to pinpoint
 *  the item where the mouse hit before calling this function.
 */

int ProcessMenu(MC * m, EVT * event, int show)
{
    HotGroup Hot;
    WND *hCurr, *hWnd = NULL;
    cmd *cmdtab = m->cmdtab;
    int num = m->num;
    int Focus = m->cur;
    int mnu = m->mode;
    int len = m->len;
    int OldFoc;
    int done = 0;
    int select = 0;
    int Msg;         /* Msg is used unitialised here. I did not yet
                        investigate it. FIXME! */
    int NewFoc;
    int i, j;

    if (m->btype & INSBDR)
    {
        m->btype = SBDR;
    }

    hCurr = WndTop();
    if (!show && !(m->mode & CMD_PRT))
    {
        hWnd = WndOpen(m->x1, m->y1, m->x2, m->y2, m->btype, bc, nc);
        if (hWnd == NULL)
        {
            WndCurr(hCurr);
            return WND_ERROR;
        }
    }

    if (m->btype & NBDR)
    {
        j = 0;
    }
    else
    {
        j = 1;
    }

    for (i = 0; i < num; i++)
    {
        Hot.harr[i].id = cmdtab[i].id;
        Hot.harr[i].x1 = m->x1 + j + cmdtab[i].col;
        Hot.harr[i].x2 = m->x1 + j + cmdtab[i].col + len;
        Hot.harr[i].y1 = m->y1 + j + cmdtab[i].row;
        Hot.harr[i].y2 = m->y1 + j + cmdtab[i].row;
        DispItem(&cmdtab[i], (unsigned char)nc, m->indent, m->len, mnu);
    }

    if (show)
    {
        return 0;
    }

    Hot.num = num;
    Hot.wid = WND_WN_MENU;

    PushHotGroup(&Hot);

    if (event->msg)
    {
        if (event->msgtype == WND_WM_COMMAND)
        {
            Focus = GetFocus(cmdtab, m->num, event->id);
        }
        else
        {
            if (event->msgtype == WND_WM_MOUSE)
            {
                Focus = LocateHotItem(event->x, event->y, WND_WN_MENU);
                if (Focus)
                {
                    Focus = GetFocus(cmdtab, m->num, Focus);
                }
            }
        }
        if (Focus == -1)
        {
            Focus = 0;
        }
    }

    DispItem(&cmdtab[Focus], (unsigned char)sc, m->indent, m->len, mnu);

    while (!done)
    {
        if (select)
        {
            if (cmdtab[Focus].m_sub)
            {
                ProcessMenu(cmdtab[Focus].m_sub, event, 0);
            }
            else
            {
                if (cmdtab[Focus].flags & CMD_EXIT)
                {
                    done = TRUE;
                    continue;
                }
                else
                {
                    if (cmdtab[Focus].itmfunc != NULL)
                    {
                        (*cmdtab[Focus].itmfunc) ();
                    }
                    done = TRUE;  /* should we exit here? */
                    continue;
                }
            }
            select = 0;
        }
        else
        {
            Msg = MnuGetMsg(event, WND_WN_MENU);
        }

        OldFoc = Focus;

        switch (event->msgtype)
        {
        case WND_WM_COMMAND:
            NewFoc = GetFocus(cmdtab, m->num, event->id);
            if (NewFoc != -1)
            {
                switch (Msg)
                {
                case MOU_LBTUP:
                case LMOU_CLCK:
                    Focus = NewFoc;
                    select = TRUE;
                    break;

                case MOU_LBTDN:
                case LMOU_RPT:
                case MOUSE_EVT:
                    Focus = NewFoc;
                    if (cmdtab[Focus].m_sub)
                    {
                        select = TRUE;
                    }
                    break;

                default:
                    break;
                }
            }
            else
            {
                if (mnu == CMD_VER && event->msg != m->parent)
                {
                    done = TRUE;
                }
            }
            break;

        case WND_WM_MOUSE:
            switch (Msg)
            {
            case LMOU_CLCK:
            case MOU_LBTUP:
                done = TRUE;
                break;

            default:
                break;
            }
            break;

        case WND_WM_CHAR:
            switch (Msg)
            {
            case Key_Up:
                if (mnu == CMD_VER)
                {
                    Focus = PrevItem(m->num, Focus);
                }
                break;

            case Key_Dwn:
                if (cmdtab[Focus].m_sub)
                {
                    select = TRUE;
                }
                if (mnu == CMD_VER)
                {
                    Focus = NextItem(m->num, Focus);
                }
                break;

            case Key_Lft:
                if (mnu == CMD_HOR)
                {
                    Focus = PrevItem(m->num, Focus);
                }
                else
                {
                    done = TRUE;
                }
                break;

            case Key_Rgt:
                if (mnu == CMD_HOR)
                {
                    Focus = NextItem(m->num, Focus);
                }
                else
                {
                    done = TRUE;
                }
                break;

            case Key_Esc:
                done = TRUE;
                break;

            case Key_Ent:
                select = TRUE;
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
        if (Focus != OldFoc)
        {
            DispItem(&cmdtab[OldFoc], (unsigned char)nc, m->indent, m->len, mnu);
            DispItem(&cmdtab[Focus], (unsigned char)sc, m->indent, m->len, mnu);
        }
    }
    DispItem(&cmdtab[Focus], (unsigned char)nc, m->indent, m->len, mnu);
    PopHotGroup();
    if (!(m->mode & CMD_PRT))
    {
        WndClose(hWnd);
        WndCurr(hCurr);
    }
    if (cmdtab[Focus].flags & CMD_EXIT && select)
    {
        /* then return the ID of ret item */

        event->msgtype = WND_WM_CHAR;
        event->msg = 0;

        return cmdtab[Focus].id;
    }
    else
    {
        return 0;
    }
}

void MnuSetColours(int nbc, int nnc, int nsc)
{
    bc = nbc;
    nc = nnc;
    sc = nsc;
}
