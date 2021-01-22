/*
 *  SYSTEM.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Code that handles system events and passes them through MnuGetMsg()
 *  back to the caller. Hotspots their location are also handled.
 */

#include <stdio.h>
#include "winsys.h"

HotGroup * HotSpot[MAX_HOT_GROUP];
int (* KeyPreProc)(int ch) = NULL;
int NumHots = 0;
EVT e;
int window_resized = 0;        /* signals a resize ... */
void PushHotGroup(HotGroup * New)
{
    if(NumHots == MAX_HOT_GROUP)
    {
        return;
    }

    HotSpot[NumHots] = New;
    NumHots++;
}

void PopHotGroup(void)
{
    if(NumHots == 0)
    {
        return;
    }

    NumHots--;
}

int LocateHotItem(int x, int y, unsigned long wid)
{
    int gr, it;

    for(gr = 0; gr < NumHots; gr++)
    {
        if(!wid || HotSpot[gr]->wid == wid)
        {
            for(it = 0; it < HotSpot[gr]->num; it++)
            {
                if(x >= HotSpot[gr]->harr[it].x1 && x <= HotSpot[gr]->harr[it].x2 &&
                   y >= HotSpot[gr]->harr[it].y1 && y <= HotSpot[gr]->harr[it].y2)
                {
                    return HotSpot[gr]->harr[it].id;
                }
            }
        }
    }
    return 0;
}

/*
 *  Returns a key or mouse press and defines the return type in p1.
 *  Should be used over the TTGetMsg counterpart.
 */
unsigned int MnuGetMsg(EVT * event, unsigned long wid)
{
    unsigned int ch = 0;
    int proc        = 0;
    int id;

    if(KeyPreProc != NULL)
    {
        ch = KeyPreProc(0);

        if(ch)
        {
            proc      = 1;
            e.msgtype = WND_WM_CHAR;
            e.msg     = ch;
        }
    }

    if(ch == 0)
    {
        ch = TTGetMsg(&e);
    }

    *event = e;

    switch(e.msgtype)
    {
        case WND_WM_MOUSE:
            id = LocateHotItem(e.x, e.y, wid);

            if(id != 0)
            {
                event->msgtype = WND_WM_COMMAND;
                event->id      = id; /* id of screen object */
            }

            break;

        case WND_WM_CHAR:

            if(!proc)
            {
                if(KeyPreProc != NULL)
                {
                    event->msg = KeyPreProc(ch);
                }
            }

            break;

        case WND_WM_RESIZE:
            window_resized = 1;
            break;

        default:
            break;
    } /* switch */
    return (unsigned int)(event->msg);
} /* MnuGetMsg */

void RegisterKeyProc(int (* fnc)(int ch))
{
    KeyPreProc = fnc;
}
