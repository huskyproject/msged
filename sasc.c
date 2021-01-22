/*
 *  S A S C
 *
 *  Some conio replacements for the Amiga, using SAS/C
 *
 *  Written by M. Stapleton of Graphic Bits
 *
 *  Oct 20 1996
 *
 *  This code is hereby donated to the Public Domain.
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <ios1.h>
#include "keys.h"

int coninit(void);
void confin(void);
int akbhit(void);
int agetch(void);
int agetche(void);
int agetchr(void);

/* Private prototypes */
struct MsgPort * findconport(void);
struct StandardPacket * createpkt(void);
void deletepkt(struct StandardPacket ** packet);

void fillpkt(struct StandardPacket * packet, LONG action, LONG args[], LONG nargs);
void sendpkt(struct StandardPacket * packet, struct MsgPort * port, struct MsgPort * replyport);

void setmode(int mode);

static int initflag;            /* Set to 1 after console is set to raw mode */
static struct StandardPacket * conpacket;
static struct MsgPort * conreply;
static struct MsgPort * conport;
static BPTR conin;              /* Console input AmigaDOS file handle */
/* Initialize console stuff */
int coninit(void)
{
    if(initflag)
    {
        return 0;
    }

    /* Find console filehandle & message port */
    conport = findconport();

    if(conport == NULL)
    {
        return 0;
    }

    conpacket = createpkt();

    if(conpacket == NULL)
    {
        return 0;
    }

    conreply = (struct MsgPort *)CreatePort(NULL, 0);

    if(conreply == NULL)
    {
        confin();
        return 0;
    }

    setmode(-1);  /* Set RAW Console input */
    initflag = 1;
    return 1;
} /* coninit */

/* End console stuff */
void confin(void)
{
    if(conreply)
    {
        if(initflag)
        {
            conport = findconport();

            if(conport != NULL)
            {
                setmode(0);
            }
        }

        initflag = 0;
        DeletePort(conreply);
    }

    if(conpacket)
    {
        deletepkt(&conpacket);
    }
}

/*
 *  Find console filehandle & message port
 *  Returns message port & sets conin
 */
struct MsgPort * findconport(void)
{
    struct UFB * ufb;

    ufb = chkufb(0);

    if(ufb == NULL)
    {
        return NULL;
    }

    conin = ufb->ufbfh);

    if(conin == NULL)
    {
        return NULL;
    }

    return ((struct FileHandle *)(conin << 2))->fh_Type;
}

struct StandardPacket * createpkt(void)
{
    struct StandardPacket * packet;

    packet = (struct StandardPacket *)AllocMem((long)sizeof(struct StandardPacket),
                                               MEMF_PUBLIC | MEMF_CLEAR);

    if(packet == NULL)
    {
        return NULL;
    }

    packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
    packet->sp_Pkt.dp_Link         = &(packet->sp_Msg);
    return packet;
}

void deletepkt(struct StandardPacket ** packet)
{
    if(*packet)
    {
        FreeMem((char *)*packet, (long)sizeof(struct StandardPacket));
    }

    *packet = NULL;  /* So we can't delete twice */
}

void fillpkt(struct StandardPacket * packet, LONG action, LONG args[], LONG nargs)
{
    LONG * pargs;

    packet->sp_Pkt.dp_Type = action;

    /* copy the arguments into the packet */
    pargs = &(packet->sp_Pkt.dp_Arg1);  /* address of 1st arg */

    while(nargs--)
    {
        *pargs++ = *args++;
    }
}

void sendpkt(struct StandardPacket * packet, struct MsgPort * port, struct MsgPort * replyport)
{
    packet->sp_Pkt.dp_Port = replyport;
    PutMsg(port, (struct Message *)packet);
}

/*
 *  Set console mode: -1 -> raw;  0 -> cooked.
 */
void setmode(int mode)
{
    fillpkt(conpacket, ACTION_SCREEN_MODE, (LONG *)&mode, 1);
    sendpkt(conpacket, conport, conreply);
    WaitPort(conreply);
    GetMsg(conreply);
}

/*
 #define TIMEOUT 1   *  Number of microseconds to wait.  Don't use 0, due to
 *  a bug in the V1 "timer.device".
 */
/*
 *  Tests if a character is available to be read from the console.
 *  Returns 1 if there is, 0 if there isn't, and -1 on failure.
 *  Assumes the console is set to "raw" mode.
 */
int akbhit(void)
{
    struct UFB * ufb;

    ufb = chkufb(0);

    if(ufb == NULL || ufb->ufbfh == NULL)
    {
        return -1;
    }

    return WaitForChar(ufb->ufbfh, TIMEOUT) ? 1 : 0;
}

/*
 *  Get a character from the console without waiting for <RETURN>.
 *  Returns EOF on end-of-file or failure.
 *  Assumes the console is set to "raw" mode.
 */
int agetch()
{
    int len;
    unsigned char buf;
    struct UFB * ufb;

    ufb = chkufb(0);

    if(ufb == NULL || ufb->ufbfh == NULL)
    {
        return EOF;
    }

    len = Read(ufb->ufbfh, &buf, 1);

    if(len == 0)
    {
        return EOF;
    }

    return (int)buf;
}

/* Getch() with echo */
int agetche(void)
{
    int buf;

    buf = agetch();

    if(buf != EOF)
    {
        Write(conin, (char *)&buf, 1);   /* Well... :) */
    }

    return buf;
}

/* Handle function & cursor keys, etc. Needs work! */
int agetchr(void)
{
    int c, d;

    c = agetch();

    if(c == 0x9b && akbhit())
    {
        /* Special key report sequence */
        d = 0;

        do
        {
            c = agetch();

            if(c == EOF || c == '~')
            {
                break;
            }

            if(isdigit(c))
            {
                c -= '0';
            }

            d = 10 * d + c;
        }
        while(c < '@' && akbhit());
        c = 256 + d;
    }

    /* Convert keys - should be done with table(s) */
    if(c > 0x80)
    {
        switch(c)
        {
            /* Convert Ctrl-Alt letters to Alt */
            case 0x0081:
                c = Key_A_A;
                break;

            case 0x0082:
                c = Key_A_B;
                break;

            case 0x0083:
                c = Key_A_C;
                break;

            case 0x0084:
                c = Key_A_D;
                break;

            case 0x0085:
                c = Key_A_E;
                break;

            case 0x0086:
                c = Key_A_F;
                break;

            case 0x0087:
                c = Key_A_G;
                break;

            case 0x0088:
                c = Key_A_H;
                break;

            case 0x0089:
                c = Key_A_I;
                break;

            case 0x008A:
                c = Key_A_J;
                break;

            case 0x008B:
                c = Key_A_K;
                break;

            case 0x008C:
                c = Key_A_L;
                break;

            case 0x008D:
                c = Key_A_M;
                break;

            case 0x008E:
                c = Key_A_N;
                break;

            case 0x008F:
                c = Key_A_O;
                break;

            case 0x0090:
                c = Key_A_P;
                break;

            case 0x0091:
                c = Key_A_Q;
                break;

            case 0x0092:
                c = Key_A_R;
                break;

            case 0x0093:
                c = Key_A_S;
                break;

            case 0x0094:
                c = Key_A_T;
                break;

            case 0x0095:
                c = Key_A_U;
                break;

            case 0x0096:
                c = Key_A_V;
                break;

            case 0x0097:
                c = Key_A_W;
                break;

            case 0x0098:
                c = Key_A_X;
                break;

            case 0x0099:
                c = Key_A_Y;
                break;

            case 0x009A:
                c = Key_A_Z;
                break;

            /* Alt Numeric keys */
            case 0x00b9:
                c = Key_A_1;
                break;

            case 0x00b2:
                c = Key_A_2;
                break;

            case 0x00b3:
                c = Key_A_3;
                break;

            case 0x00a2:
                c = Key_A_4;
                break;

            case 0x00bc:
                c = Key_A_5;
                break;

            case 0x00bd:
                c = Key_A_6;
                break;

            case 0x00be:
                c = Key_A_7;
                break;

            case 0x00b7:
                c = Key_A_8;
                break;

            case 0x00ab:
                c = Key_A_9;
                break;

            case 0x00bb:
                c = Key_A_0;
                break;

            /* Plain Function keys */
            case 0x0100:
                c = Key_F1;
                break;

            case 0x0101:
                c = Key_F2;
                break;

            case 0x0102:
                c = Key_F3;
                break;

            case 0x0103:
                c = Key_F4;
                break;

            case 0x0104:
                c = Key_F5;
                break;

            case 0x0105:
                c = Key_F6;
                break;

            case 0x0106:
                c = Key_F7;
                break;

            case 0x0107:
                c = Key_F8;
                break;

            case 0x0108:
                c = Key_F9;
                break;

            case 0x0109:
                c = Key_F10;
                break;

            /* Shifted Function keys */
            case 0x010a:
                c = Key_S_F1;
                break;

            case 0x010b:
                c = Key_S_F2;
                break;

            case 0x010c:
                c = Key_S_F3;
                break;

            case 0x010d:
                c = Key_S_F4;
                break;

            case 0x010e:
                c = Key_S_F5;
                break;

            case 0x010f:
                c = Key_S_F6;
                break;

            case 0x0110:
                c = Key_S_F7;
                break;

            case 0x0111:
                c = Key_S_F8;
                break;

            case 0x0112:
                c = Key_S_F9;
                break;

            case 0x0113:
                c = Key_S_F10;
                break;

            /* Help -> Alt-h */
            case 0x013f:
                c = Key_A_H;
                break;

            /* Cursor keys */
            case 0x0141:
                c = Key_Up;
                break;

            case 0x0142:
                c = Key_Dwn;
                break;

            case 0x0143:
                c = Key_Rgt;
                break;

            case 0x0144:
                c = Key_Lft;
                break;

            /* Shift-Cursor keys */
            case 0x0153:
                c = Key_PgDn;
                break;

            case 0x0154:
                c = Key_PgUp;
                break;

            case 0x0280:
                c = Key_C_Rgt;
                break;

            case 0x0281:
                c = Key_C_Lft;
                break;

            default:
                break;
        } /* switch */
    }

    return c;
} /* agetchr */
