/*
 *  VSEVOPS.C
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  Perform Version7 nodelist lookup operations.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mxbt.h"
#include "vsev.h"
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "userlist.h"
#include "vsevops.h"
#include "strextra.h"
#include "dirute.h"  /* adaptcase */

static int compareSysop(void *a, void *b, int len)
{
    return strncmpi((char *)a, (char *)b, len);
}

static int compareNode(void *a, void *b, int len)
{
    struct
    {
        short zone;
        short net;
        short node;
        short point;
    }
    v7addr;
    FIDO_ADDRESS *addrp;

    addrp = (FIDO_ADDRESS *) b;
    memcpy(&v7addr, a, len);
    if (len <= 6)
    {
        v7addr.point = 0;
    }
    if (v7addr.zone < addrp->zone)
    {
        return -1;
    }
    else if (v7addr.zone > addrp->zone)
    {
        return 1;
    }
    else if (v7addr.net < addrp->net)
    {
        return -1;
    }
    else if (v7addr.net > addrp->net)
    {
        return 1;
    }
    else if (v7addr.node < addrp->node)
    {
        return -1;
    }
    else if (v7addr.node > addrp->node)
    {
        return 1;
    }
    else if (v7addr.point < addrp->point)
    {
        return -1;
    }
    else if (v7addr.point > addrp->point)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
 *  The following stuff was written by John Dennis (with mods by Paul
 *  Edwards) and was also released to the public domain.
 */

char *v7lookupnode(FIDO_ADDRESS * faddr, char *name)
{
    long record;
    char index_filename[FILENAME_MAX];
    char data_filename[FILENAME_MAX];
    MXBT mxbt;
    VSEV vsev;

    strcpy(index_filename, ST->nodepath);
    strcat(index_filename, "/");
    if (ST->nodebase != NULL)
    {
        strcat(index_filename, ST->nodebase);
    }
    strcat(index_filename, ".ndx");
    strcpy(data_filename, ST->nodepath);
    strcat(data_filename, "/");
    if (ST->nodebase != NULL)
    {
        strcat(data_filename, ST->nodebase);
    }
    strcat(data_filename, ".dat");

    /* Find the files in the correct spelling on UNIX file systems */
    if (SW->adaptivecase)
    {
        adaptcase(data_filename);   
        adaptcase(index_filename);
    }

    record = mxbtOneSearch(&mxbt, index_filename, (void *)faddr, compareNode);

    if (record == -1)
    {
        return NULL;
    }

    if (vsevGetInfo(&vsev, data_filename, record) == 0)
    {
        strcpy(name, (char *)vsev.sysop);
        return name;
    }
    else
    {
        return NULL;
    }
}

FIDO_ADDRESS v7lookup(char *name)
{
    FIDO_ADDRESS faddr;
    char reverse[80];
    long record;
    char index_filename[FILENAME_MAX];
    char data_filename[FILENAME_MAX];
    MXBT mxbt;
    VSEV vsev;

    faddr = CurArea.addr;
    faddr.domain = NULL;

    makeReverse(reverse, name);

    strcpy(index_filename, ST->nodepath);
    strcat(index_filename, "/");
    if (ST->sysop != NULL)
    {
        strcat(index_filename, ST->sysop);
    }
    strcpy(data_filename, ST->nodepath);
    strcat(data_filename, "/");
    if (ST->nodebase != NULL)
    {
        strcat(data_filename, ST->nodebase);
    }
    strcat(data_filename, ".dat");

    /* Find the files in the correct spelling on UNIX file systems */
    if (SW->adaptivecase)
    {
        adaptcase(data_filename);   
        adaptcase(index_filename);
    }

    record = mxbtOneSearch(&mxbt, index_filename, (void *)reverse, compareSysop);

    if (record == -1)
    {
        faddr.notfound = 1;
        return faddr;
    }
    if (vsevGetInfo(&vsev, data_filename, record) != 0)
    {
        faddr.notfound = 1;
        return faddr;
    }
    faddr.zone = vsev.zone;
    faddr.net = vsev.net;
    faddr.node = vsev.node;
    faddr.point = vsev.point;
    faddr.notfound = 0;

    return faddr;
}

char *v7lookupsystem(FIDO_ADDRESS * faddr, char *system)
{
    long record;
    int is_point = 0;
    char index_filename[FILENAME_MAX];
    char data_filename[FILENAME_MAX];
    MXBT mxbt;
    VSEV vsev;

    strcpy(index_filename, ST->nodepath);
    strcat(index_filename, "/");
    if (ST->nodebase != NULL)
    {
        strcat(index_filename, ST->nodebase);
    }
    strcat(index_filename, ".ndx");
    strcpy(data_filename, ST->nodepath);
    strcat(data_filename, "/");
    if (ST->nodebase != NULL)
    {
        strcat(data_filename, ST->nodebase);
    }
    strcat(data_filename, ".dat");

    /* Find the files in the correct spelling on UNIX file systems */
    if (SW->adaptivecase)
    {
        adaptcase(data_filename);   
        adaptcase(index_filename);
    }

    if (faddr->point)
    {
        is_point = faddr->point;
        faddr->point = 0;
    }

    record = mxbtOneSearch(&mxbt, index_filename, (void *)faddr, compareNode);

    if (is_point)
    {
        faddr->point = is_point;
    }

    if (record == -1)
    {
        return NULL;
    }

    if (vsevGetInfo(&vsev, data_filename, record) == 0)
    {
        strcpy(system, (char *)vsev.board);
        if (is_point)
        {
            strcat(system, " (point)");
        }
        return system;
    }
    else
    {
        return NULL;
    }
}
