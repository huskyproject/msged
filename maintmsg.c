/*
 *  MAINTMSG.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Handles message maintenance.
 */

#define TEXTLEN 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "memextra.h"
#include "keys.h"
#include "strextra.h"
#include "readmail.h"
#include "areas.h"
#include "makemsgn.h"
#include "template.h"
#include "winsys.h"
#include "menu.h"
#include "nshow.h"
#include "maintmsg.h"

static char *formenu[] =
{
    "Move Message",
    "Copy Message",
    "Redirect Message",
    "Forward Message",
    NULL
};

/*
 *  Deletes the current message.
 */

void deletemsg(void)
{
    unsigned long n = CurArea.current;
    msg *from;
    msg *to;
    int i;

    if (message == NULL)
    {
        message = readmsg(CurArea.current);
        if (message == NULL)
        {
            return;
        }
    }
    if (!confirm("Erase Message?"))
    {
        /* patch up reply links */
        return;
    }

    for (i = 0; i < 10; i++)
    {
        if (message->replies[i] != 0)
        {
            from = MsgReadHeader(message->replies[i], RD_HEADER);
            if (from)
            {
                from->replyto = 0;
                MsgWriteHeader(from, WR_HEADER);
                dispose(from);
            }
        }
    }

    to = MsgReadHeader(message->replyto, RD_HEADER);
    if (to)
    {
        for (i = 0; i < 10; i++)
        {
            if (to->replies[i] == n)
            {
                to->replies[i] = 0;
            }
        }
        MsgWriteHeader(to, WR_HEADER);
        dispose(to);
    }

    MsgDelete(message->msgnum);  /* we re-scan the area to be safe */
    MsgAreaClose();
    message = KillMsg(message);
    CurArea.messages = MsgAreaOpen(&CurArea);
    CurArea.current = min(max(1, n), CurArea.messages);
}

/*
 *  Forwards the current message.
 */

int forward_msg(int to_area)
{
    msg *m;
    msg *oldm;
    int fr_area = SW->area;
    time_t now = time(NULL);

    if (to_area == -1)
    {
        to_area = selectarea("Forward To Area", SW->area);
    }

    if (msgederr)
    {
        return -1;
    }

    if (message == NULL)
    {
        message = readmsg(CurArea.current);
        if (message == NULL)
        {
            return -1;
        }
    }

    /*
     *  Save the current message and make the global pointer equal
     *  to null (forcing a re-read).
     */

    oldm = duplicatemsg(message);
    m = message;
    message = NULL;

    set_area(to_area);

    if (!CurArea.status)
    {
        dispose(oldm);
        dispose(m);
        set_area(fr_area);
        return -1;
    }

    if ((m->reply && fr_area != to_area) || CurArea.netmail)
    {
        release(m->reply);
    }

    release(m->msgid);
    release(m->isfrom);
    release(m->isto);
    release(m->from.domain);
    release(m->to.domain);

    m->from = CurArea.addr;

    if (CurArea.addr.domain)
    {
        m->from.domain = xstrdup(CurArea.addr.domain);
    }

    clear_attributes(&m->attrib);
    memset(m->replies, 0, sizeof(m->replies));

    CurArea.new = 1;
    m->isfrom = xstrdup(ST->username);
    m->new = 1;
    m->msgnum = MsgnToUid(CurArea.messages) + 1;
    m->attrib.local = 1;
    m->attrib.sent = 0;
    m->timestamp = now;
    m->scanned = 0;
    m->replyto = 0;
    m->attrib.priv = CurArea.priv;
    m->invkludges = 1;

    if (EditHeader(m) == Key_Esc && confirm("Cancel?"))
    {
        set_area(fr_area);
    }
    else
    {
        MakeTemplateMsg(m, oldm, fr_area, MT_FOR);
        save(m);
        set_area(fr_area);
    }
    dispose(oldm);
    dispose(m);
    return to_area;
}

/*
 *  Redirects the current message.
 */

int redirect_msg(int to_area)
{
    msg *m;
    msg *oldm;
    int fr_area = SW->area;

    if (to_area == -1)
    {
        to_area = selectarea("Redirect To Area", SW->area);
    }

    if (msgederr)
    {
        return -1;
    }

    if (message != NULL)
    {
        dispose(message);
    }

    read_verbatim = 1;
    message = readmsg(CurArea.current);
    if (message == NULL)
    {
        return -1;
    }

    oldm = duplicatemsg(message);
    m = message;
    message = NULL;
    set_area(to_area);

    if (!CurArea.status)
    {
        dispose(oldm);
        dispose(m);
        set_area(fr_area);
        return -1;
    }

    memset(m->replies, 0, sizeof(m->replies));

    if ((m->reply && fr_area != to_area) || CurArea.netmail)
    {
        release(m->reply);
    }

    release(m->msgid);
    release(m->isto);
    clear_attributes(&m->attrib);

    CurArea.new = 1;
    m->new = 1;
    m->attrib.local = 1;
    m->attrib.sent = 0;
    m->msgnum = MsgnToUid(CurArea.messages) + 1;
    m->scanned = 0;
    m->replyto = 0;
    m->attrib.priv = CurArea.priv;
    m->rawcopy = 1;

    if (EditHeader(m) == Key_Esc && confirm("Cancel?"))
    {
        set_area(fr_area);
    }
    else
    {
        MakeTemplateMsg(m, oldm, fr_area, MT_RED);
        save(m);
        set_area(fr_area);
    }
    dispose(oldm);
    dispose(m);
    return to_area;
}

/*
 *  Copies the current message.
 */

int copy_msg(int to_area)
{
    msg *m;
    int fr_area = SW->area;

    if (to_area == -1)
    {
        to_area = selectarea("Copy To Area", SW->area);
    }

    if (msgederr)
    {
        return -1;
    }

    if (message != NULL)
    {
        dispose(message);
    }
    
    read_verbatim = 1;
    message = readmsg(CurArea.current);

    if (message == NULL)
    {
        return -1;
    }

    m = message;
    message = NULL;

    set_area(to_area);
    if (!CurArea.status)
    {
        dispose(m);
        set_area(fr_area);
        return -1;
    }
    clear_attributes(&m->attrib);
    CurArea.new = 1;
    release(m->from.domain);
    m->from = CurArea.addr;
    if (CurArea.addr.domain != NULL)
    {
        m->from.domain = xstrdup(CurArea.addr.domain);
    }
    m->msgnum = MsgnToUid(CurArea.messages) + 1;
    m->new = 1;
    m->attrib.sent = 0;
    m->attrib.local = 1;
    m->scanned = 0;
    m->attrib.priv = CurArea.priv;
    m->rawcopy = 1;

    memset(m->replies, 0, sizeof(m->replies));
    m->replyto = 0;

    writemsg(m);
    set_area(fr_area);
    dispose(m);
    return to_area;
}

/*
 *  Moves the current message.
 */

int move_msg(int to_area)
{
    msg *m;
    int fr_area = SW->area;
    int status;

    if (to_area == -1)
    {
        to_area = selectarea("Move To Area", SW->area);
    }

    if (msgederr)
    {
        return -1;
    }

    if (message != NULL)
    {
        dispose(message);
        message = NULL;
    }
    
    read_verbatim = 1;
    message = readmsg(CurArea.current);

    if (message == NULL)
    {
        return -1;
    }

    m = message;      /* save the msg so we can write it */
    message = NULL;

    set_area(to_area);
    if (!CurArea.status)
    {
        set_area(fr_area);
        dispose(m);
        return -1;
    }
    clear_attributes(&m->attrib);
    CurArea.new = 1;
    release(m->from.domain);
    m->from = CurArea.addr;
    if (CurArea.addr.domain != NULL)
    {
        m->from.domain = xstrdup(CurArea.addr.domain);
    }
    m->msgnum = MsgnToUid(CurArea.messages) + 1;
    m->new = 1;
    m->attrib.sent = 0;
    m->attrib.local = 1;
    m->scanned = 0;
    m->attrib.priv = CurArea.priv;
    m->rawcopy = 1;

    memset(m->replies, 0, sizeof(m->replies));
    m->replyto = 0;

    status = writemsg(m);
    dispose(m);
    set_area(fr_area);

    if (status == TRUE)
    {
        int confirm_temp;
        confirm_temp = SW->confirmations;
        SW->confirmations = 0;
        deletemsg();
        SW->confirmations = confirm_temp;
    }
    return to_area;
}

/*
 *  Allows forwarding, redirecting, moving and copying of messages.
 */

void movemsg(void)
{
    int rc;

    rc = DoMenu((maxx / 2) - 8, (maxy / 2) - 1, (maxx / 2) + 8,
      (maxy / 2) + 2, formenu, 0, SELBOX_MOVEMSG, "");

    switch (rc)
    {
    case 0:                    /* Move */
        move_msg(-1);          /* to_area = -1  will present a menu that */
        break;                 /* lets the user select an area           */

    case 1:                    /* Copy */
        copy_msg(-1);
        break;

    case 2:                    /* Redirect */
        redirect_msg(-1);
        break;

    case 3:                    /* Forward */
        forward_msg(-1);
        break;

    case -1:                   /* Escape */
        return;
    }

    ShowNewArea();
}
