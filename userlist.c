/*
 *  USERLIST.C
 *
 *  Written by Paul Edwards and modified by Bill Bond.
 *  Modified to make it do a binary search by Bill Bond.
 *  Modfied by Tobias Ernst (based on code by Kim Lykkegaard) to support
 *             multiple results for a single lookup.
 *  Released to the public domain.
 *
 *  Look up user name in a Fido userlist.
 *
 *  Note: The Fido userlist consists of fixed length records sorted
 *  alphabetically.  A record is in the format:
 *
 *  Bloggs, Joe      3:666/666
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "winsys.h"
#include "keys.h"
#include "menu.h"
#include "msged.h"
#include "memextra.h"
#include "strextra.h"
#include "nshow.h"
#include "userlist.h"
#include "screen.h"

#define SELBOX_WRTOVER  7
#if defined(MSDOS) && (!defined(__FLAT__))
#define LOOKUPMAX      200      /* prevent memory exhaustage */
#else
#define LOOKUPMAX      200000L
#endif
#define LOOKUPSTEP     50

static long filelen(FILE * fp)
{
    long ret;

    ret = fseek(fp, 0, SEEK_END);
    if (ret != 0L)
    {
        return (0L);
    }
    ret = ftell(fp);
    if (ret < 0)
    {
        ret = 0;
    }
    return ret;
}

ADDRESS lookup(char *name, char *fn)
{
    FILE *fp;
    ADDRESS tmpAddr;
    char buf[200];
    char revName[200];
    char **nodeinfo = NULL;
    int lenRev, result, fureclen = 1, done = 0;
    int i, j, k;
    long found = 0, lookupmax = 0L;
    long low, mid, high;
    char *p;

    tmpAddr = CurArea.addr;
    if (tmpAddr.domain != NULL)
    {
        tmpAddr.domain = xstrdup(tmpAddr.domain);
    }
    tmpAddr.notfound = 1;
    fp = fopen(fn, "r");
    if (fp == NULL)
    {
        return (tmpAddr);
    }
    makeReverse(revName, name);
    strlwr(revName);
    lenRev = strlen(revName);
    if (fgets(buf, sizeof buf, fp) != NULL)
    {
        fureclen = strlen(buf);
#ifndef UNIX
        fureclen++;  /* take the \r character into account */
        i++;
#endif        
    }
    high = filelen(fp) / fureclen;
    low = 0;
    while (low <= high && !done)
    {
        mid = low + (high - low) / 2;
        fseek(fp, (long)mid * fureclen, SEEK_SET);
        if (fgets(buf, sizeof buf, fp) != NULL)
        {
            strlwr(buf);
            result = strncmp(buf, revName, lenRev);
            if (result > 0)
            {
                high = mid - 1;
            }
            else if (result < 0)
            {
                low = mid + 1;
            }
            else
            {
                if (!OpenMsgWnd(50, 6, " Scanning Fido User List ",
                                NULL, 0, 0))
                {
                    return tmpAddr;
                }
                SendMsgWnd("Press Esc to stop", 1);                

                /* seek backwards to find the first match */
                while (result == 0)
                {
                    mid=mid-10;
                    if (mid<0)
                    {
                        fseek(fp, 0L, SEEK_SET);
                        mid = 0;
                        break;
                    }
                    fseek(fp, (long)mid * fureclen, SEEK_SET);
                    fgets(buf, sizeof buf, fp);
                    strlwr(buf);
                    result = strncmp(buf, revName, lenRev);
                    if (KeyHit() && GetKey() == Key_Esc)
                    {
                        CloseMsgWnd();
                        return tmpAddr;
                    }
                }

                /* we seeked backwards too far, now find the first match. */
                while (result != 0)
                {
                    fgets(buf, sizeof buf, fp);
                    strlwr(buf);
                    result = strncmp(buf, revName, lenRev);
                    if (KeyHit() && GetKey() == Key_Esc)
                    {
                        CloseMsgWnd();
                        return tmpAddr;
                    }
                }

                /*  Now we have found the first match. Implement a lookup
                    window */
                while (result == 0)
                {
                    /* kill trailing LFs and CRs */
                    if (buf[strlen(buf)-1]=='\n')
                    {
                        buf[strlen(buf)-1]='\0';
                    }
                    if (buf[strlen(buf)-1]=='\r')
                    {
                        buf[strlen(buf)-1]='\0';
                    }

                    /* uppercase the first letters of the name */
                    buf[0] = toupper(buf[0]);
                    for(i = 1; *buf && i < strlen(buf) - 1; i++)
                    {
                        if ((buf[i] == ' ') && (buf[i+1] != ' '))
                        {
                            buf[i + 1] = toupper(buf[i + 1]);
                        }
                    }

                    if (found + 1 >= lookupmax)
                    {
                        if (lookupmax + LOOKUPSTEP > LOOKUPMAX)
                        {
                            break;
                        }
                        if (lookupmax == 0)
                        {
                            lookupmax = LOOKUPSTEP;
                            nodeinfo = xmalloc(sizeof(char *) * lookupmax);
                        }
                        else
                        {
                            lookupmax += LOOKUPSTEP;
                            nodeinfo = xrealloc(nodeinfo,
                                                sizeof(char *) * lookupmax);
                        }
                    }
                    
                    nodeinfo[found++] = xstrdup(buf);
                    if (fgets(buf, sizeof buf, fp) == NULL)
                    {
                        break;
                    }
                    strlwr(buf);
                    result = strncmp(buf, revName, lenRev);
                    if (KeyHit() && GetKey() == Key_Esc)
                    {
                        CloseMsgWnd();
                        return tmpAddr;
                    }
                }
                nodeinfo[found++] = NULL;

                CloseMsgWnd();                
                

                result = DoMenu(8, 7, 8+61,
                                (5 + found > maxy - 4) ? maxy - 4 : 5 + found,
                                nodeinfo, 0,
                                SELBOX_WRTOVER, " Nodelist Lookup ");
                if (result == (unsigned long) - 1)
                {
                    fclose(fp);
                    return tmpAddr;
                }
                strcpy(buf, nodeinfo[result]);

                /*  Use the name as it was found in the userlist */
                i = strlen(buf) - 1;
                while (i > 0 && isspace(buf[i]))
                {
                    i--;
                }
                while (i > 0 && !isspace(buf[i]))
                {
                    i--;
                }
                while (i > 0 && isspace(buf[i]))
                {
                    i--;
                }
                j = i;
                while (i > 0 && buf[i] != ',')
                {
                    i--;
                }
                if (buf[i + 1] == ' ')
                {
                    i++;
                }
                if (j - i + 1 > 0)
                {
                    memmove(name, buf + i + 1, j - i);
                }
                k = i;
                if (buf[i] == ' ')
                {
                    i--;
                }
                if (i > 0)
                {
                    name[j - k] = ' ';
                    memmove(name + j - k + 1, buf, k);
                    name[j] = '\0';
                }

                for(i = 0; i < found; i++)
                {
                    if (nodeinfo[i] == NULL)
                    {
                        break;
                    }
                    xfree(nodeinfo[i]);
                }
                
                p = buf + strlen(buf) - 1;
                while ((p >= buf) && (isspace(*p)))
                {
                    *p = '\0';
                    p--;
                }
                while ((p >= buf) && (!isspace(*p)))
                {
                    p--;
                }
                if ((p >= buf) && (isspace(*p)))
                {
                    p++;
                }
                if (tmpAddr.domain != NULL)
                {
                    xfree(tmpAddr.domain);
                }
                fclose(fp);
                return parsenode(p);
            }
        }
        else
        {
            done = 0;  /* force end of loop */
        }
    }
    fclose(fp);
    return tmpAddr;
}

void makeReverse(char *revName, char *name)
{
    char *lastSpace;
    int len;

    lastSpace = strrchr(name, ' ');
    if (lastSpace == NULL)
    {
        strcpy(revName, name);
        return;
    }
    len = strlen(lastSpace + 1);
    memcpy(revName, lastSpace + 1, len);
    memcpy(revName + len, ", ", 2);
    memcpy(revName + len + 2, name, (size_t) (lastSpace - name));
    *(revName + len + 2 + (size_t) (lastSpace - name)) = '\0';
}
