/*
 *  VSEV.C
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  Version7 nodelist processor.
 *
 *  The version 7 nodelist is normally comprised as follows:
 *
 *  1. NODEX.DAT - the complete nodelist data file.
 *  2. NODEX.NDX - an index into nodex.dat.  The index entries are all
 *  address-based.  After using the index, you will obtain an offset,
 *  which can be used to access nodex.dat.
 *  3. SYSOP.NDX - another index into nodex.dat, this time by sysop name.
 *  The name used for the index is "surname, firstname".  Although the
 *  names are stored in mixed case, they are ordered case-insensitive.
 */
/*
 *  This module has only one external function - vsevGetInfo.  It
 *  requires a pointer to a VSEV structure, which should be empty.  The
 *  second parameter is the name of the V7 data file.  The third
 *  parameter is the offset.  You are  expected to have used one of the
 *  indexes to have retrieved the proper offset before retrieving the
 *  data.
 *
 *  The function returns 0 on success, non-zero on failure.  If the call
 *  was successful, then via the passed VSEV data structure, you will now
 *  have access to the following information:
 *
 *
 *  zone, net, node, point, hub, cost, fee, flags, modem, phone, password,
 *  board, sysop, misc, baud.
 *
 *  Note that on return from this function, the board name, sysop name
 *  and misc info will be both unpacked and converted into mixed case.
 *  Internally, the data in the v7 nodelist data file is packed by
 *  converting everything to uppercase and then compressing it by
 *  compressing 3 bytes into 2.  Presumably someone thought that the
 *  space advantages of making an abortion like this was worth it, and
 *  not just a gigantic wank.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "vsev.h"

static void vsevActual(VSEV * vsev);
static void vsevActual2(VSEV * vsev);
static void processPack(VSEV * vsev);
static void unpack(unsigned char * one, unsigned char * two, int len);
static void mixstr(unsigned char * str);

int vsevGetInfo(VSEV * vsev, char * dataFile, long offset)
{
    vsev->error = 0;
    vsev->fp    = fopen(dataFile, "rb");

    if(vsev->fp == NULL)
    {
        vsev->error = 1;
    }
    else
    {
        if(fseek(vsev->fp, offset, SEEK_SET) != 0)
        {
            vsev->error = 1;
        }
        else
        {
            vsevActual(vsev);
        }
    }

    fclose(vsev->fp);

    if(vsev->error)
    {
        return 1;
    }
    else
    {
        return 0;
    }
} /* vsevGetInfo */

static void vsevActual(VSEV * vsev)
{
    fread(&vsev->zone, sizeof vsev->zone, 1, vsev->fp);
    fread(&vsev->net, sizeof vsev->net, 1, vsev->fp);
    fread(&vsev->node, sizeof vsev->node, 1, vsev->fp);
    fread(&vsev->hub, sizeof vsev->hub, 1, vsev->fp);
    fread(&vsev->cost, sizeof vsev->cost, 1, vsev->fp);
    fread(&vsev->fee, sizeof vsev->fee, 1, vsev->fp);
    fread(&vsev->flags, sizeof vsev->flags, 1, vsev->fp);
    fread(&vsev->modem, sizeof vsev->modem, 1, vsev->fp);
    fread(&vsev->phoneLen, sizeof vsev->phoneLen, 1, vsev->fp);
    fread(&vsev->passwordLen, sizeof vsev->passwordLen, 1, vsev->fp);
    fread(&vsev->boardLen, sizeof vsev->boardLen, 1, vsev->fp);
    fread(&vsev->sysopLen, sizeof vsev->sysopLen, 1, vsev->fp);
    fread(&vsev->miscLen, sizeof vsev->miscLen, 1, vsev->fp);
    fread(&vsev->packLen, sizeof vsev->packLen, 1, vsev->fp);
    fread(&vsev->baud, sizeof vsev->baud, 1, vsev->fp);

    if(feof(vsev->fp) || ferror(vsev->fp))
    {
        vsev->error = 1;
    }
    else
    {
        vsevActual2(vsev);
    }
} /* vsevActual */

static void vsevActual2(VSEV * vsev)
{
    if((vsev->flags & 0x1000U) != 0)
    {
        vsev->point = vsev->hub;
        vsev->hub   = vsev->node;
    }
    else
    {
        vsev->point = 0;
    }

    fread(vsev->phone, vsev->phoneLen, 1, vsev->fp);
    fread(vsev->password, vsev->passwordLen, 1, vsev->fp);
    fread(vsev->packData, sizeof vsev->packData, 1, vsev->fp);

    if(feof(vsev->fp) || ferror(vsev->fp))
    {
        vsev->error = 1;
    }
    else
    {
        processPack(vsev);
    }
}

static void processPack(VSEV * vsev)
{
    int x, offset = 0;

    vsev->packData[vsev->packLen] = '\0';  /* just in case */

    unpack(vsev->unpackData, vsev->packData, vsev->packLen);
    x = vsev->boardLen;

    if(x > 100)
    {
        x = 100;
    }

    memcpy(vsev->board, vsev->unpackData + offset, x);
    vsev->board[x] = '\0';
    offset        += vsev->boardLen;
    mixstr(vsev->board);
    x = vsev->sysopLen;

    if(x > 100)
    {
        x = 100;
    }

    memcpy(vsev->sysop, vsev->unpackData + offset, x);
    vsev->sysop[x] = '\0';
    offset        += vsev->sysopLen;
    mixstr(vsev->sysop);
    x = vsev->miscLen;

    if(x > 100)
    {
        x = 100;
    }

    memcpy(vsev->misc, vsev->unpackData + offset, x);
    vsev->misc[x] = '\0';
    mixstr(vsev->misc);
} /* processPack */

static void unpack(unsigned char * one, unsigned char * two, int len)
{
    int x, y;
    char * code = " EANROSTILCHBDMUGPKYWFVJXZQ-'0123456789";
    unsigned int i;

    for(x = 0, y = 0; x < len; x += 2, y += 3)
    {
        i          = two[x] | (two[x + 1] << 8);
        one[y + 2] = code[(size_t)(i % 40)];
        i         /= 40;
        one[y + 1] = code[(size_t)(i % 40)];
        i         /= 40;
        one[y]     = code[(size_t)(i % 40)];
    }
    one[y] = '\0';
}

static void mixstr(unsigned char * str)
{
    int upper = 1;

    while(*str != '\0')
    {
        if(upper)
        {
            *str = (unsigned char)toupper(*str);
        }
        else
        {
            *str = (unsigned char)tolower(*str);
        }

        if(isalpha(*str))
        {
            upper = 0;
        }
        else
        {
            upper = 1;
        }

        str++;
    }
} /* mixstr */
