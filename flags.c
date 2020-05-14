#include <string.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "flags.h"
#include "mctype.h"
#include "strextra.h"

/* This routine prints the message's flags according to FSC 0053.
   - If storage == -1, all flags that are known to FSC 0053 will be
     printed. 
   - If echomail == 1, all flags except local, snt, rcv will be
     printed
   - If storage == SQUISH, HUDSON or QUICK, all flags will be printed
     except those that are known to the particular storage format.
   The string must be able to hold at least 130 characters.
*/

void printflags(char *dst, msg *m, int storage, int echomail)
{
    *dst = '\0';

    /* not supported by fsc 53: areq, ureq, rcpt, forward, local, orphan */

    if (storage == -1 || echomail) 
    {
        if (m->attrib.priv) strcat(dst, "PVT ");
    }
    if (storage == -1 || storage == QUICK || echomail)
    {
        if (m->attrib.hold) strcat(dst, "HLD ");
    }
    if (storage == -1 || echomail)
    {
        if (m->attrib.crash) strcat(dst, "CRA ");
        if (m->attrib.killsent) strcat(dst, "K/S ");
    }
    if (storage == -1)
    {
        if (m->attrib.sent) strcat(dst, "SNT ");
        if (m->attrib.rcvd) strcat(dst, "RCV ");
    }

    if (m->attrib.as) strcat(dst, "A/S ");

    if (storage == QUICK || storage == SQUISH || storage == -1 || echomail)
    {
        if (m->attrib.direct) strcat(dst, "DIR ");
    }

    if (m->attrib.zon) strcat(dst, "ZON ");
    if (m->attrib.hub) strcat(dst, "HUB ");

    if (storage == -1 || echomail)
    {        
        if (m->attrib.attach) strcat(dst, "FIL ");
    }

    if (storage == -1 || storage == QUICK || echomail)
    {
        if (m->attrib.freq) strcat(dst, "FRQ ");
    }

    if (m->attrib.immediate) strcat(dst, "IMM ");
    if (m->attrib.kfs) strcat(dst, "KFS ");
    if (m->attrib.tfs) strcat(dst, "TFS ");
    if (m->attrib.lock) strcat(dst, "LOK ");

    if (storage == -1 || echomail)
    {
        if (m->attrib.rreq) strcat(dst, "RRQ ");
    }

    if (m->attrib.cfm) strcat(dst, "CFM ");

    if (*dst)
    {
        dst[strlen(dst) - 1] = '\0';
    }
}


/* This line parses a string that contains a whitepace-separated list of
   flag names according to FSC 0053. The message structures corresponding flag
   bits are set if a corresponding name is found. Names that are not known are
   ignored. Note that those flag bits that are not specified will not be
   cleared automatically! If you want this behaviour, you have to clear them
   before calling parseflags. */
 
void parseflags(char *src, msg *m)
{
    char flag[4], *cp;
    int i;

    cp = src;
    while (m_isspace(*cp) && (*cp))
    {
        cp++;
    }
     
    while(*cp)
    {
        for (i = 0; i < 3 && (*cp) && (!m_isspace(*cp)); i++)
        {
            flag[i] = *cp++;
        }
        flag[i] = '\0';
        while (*(cp) && m_isspace(*cp))
        {
            cp++;
        }


        if (!stricmp(flag, "PVT"))
        {
            m->attrib.priv = 1;
        }
        else if (!stricmp(flag, "HLD"))
        {
            m->attrib.hold = 1;
        }
        else if (!stricmp(flag, "CRA"))
        {
            m->attrib.crash = 1;
        }
        else if (!stricmp(flag, "K/S"))
        {
            m->attrib.killsent = 1;
        }
        else if (!stricmp(flag, "SNT"))
        {
            m->attrib.sent = 1;
        }
        else if (!stricmp(flag, "RCV"))
        {
            m->attrib.rcvd = 1;
        }
        else if (!stricmp(flag, "A/S"))
        {
            m->attrib.as = 1;
        }
        else if (!stricmp(flag, "DIR"))
        {
            m->attrib.direct = 1;
        }
        else if (!stricmp(flag, "ZON"))
        {
            m->attrib.zon = 1;
        }
        else if (!stricmp(flag, "HUB"))
        {
            m->attrib.hub = 1;
        }
        else if (!stricmp(flag, "FIL"))
        {
            m->attrib.attach = 1;
        }
        else if (!stricmp(flag, "FRQ"))
        {
            m->attrib.freq = 1;
        }
        else if (!stricmp(flag, "IMM"))
        {
            m->attrib.immediate = 1;
        }
        else if (!stricmp(flag, "KFS"))
        {
            m->attrib.kfs = 1;
        }
        else if (!stricmp(flag, "TFS"))
        {
            m->attrib.tfs = 1;
        }
        else if (!stricmp(flag, "LOK"))
        {
            m->attrib.lock = 1;
        }
        else if (!stricmp(flag, "RRQ"))
        {
            m->attrib.rreq = 1;
        }
        else if (!stricmp(flag, "CFM"))
        {
            m->attrib.cfm = 1;
        }
    }
}

