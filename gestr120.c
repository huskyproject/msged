
/*
 * gestr120.c
 *
 * This module contains routines that read in structures from
 * GECHO configuration files in a portable way, i.E. they will even
 * work in cases where a fread(&structure, sizeof(structure), 1, stream)
 * would fail because of structure packing or big endian problems.
 *
 * Only those structures that are of importance to MsgEd have been
 * implemented.
 *
 * Written 03-Oct-98 by Tobias Ernst and donated to the Public Domain.
 *
 */

#include <assert.h>
#include <stdio.h>

typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   dword;

#include "gestr120.h"
#include "memextra.h"

/*
 *  get_dword
 *
 *  Reads in a 4 byte word that is stored in little endian (Intel) notation
 *  and converts it to the local representation n an architecture-
 *  independent manner
 */

#define get_dword(ptr)            \
   ((dword)((ptr)[0]) |           \
    (((dword)((ptr)[1])) << 8)  | \
    (((dword)((ptr)[2])) << 16) | \
    (((dword)((ptr)[3])) << 24))  \

/*
 *  get_word
 *
 *  Reads in a 2 byte word that is stored in little endian (Intel) notation
 *  and converts it to the local representation in an architecture-
 *  independent manner
 */

#define get_word(ptr)         \
    ((word)(ptr)[0] |         \
     (((word)(ptr)[1]) << 8 ))


/*
 * read_setup_ge
 *
 * reads a SETUP_GE structure.
 *
 */

int read_setup_ge(SETUP_GE *Setup, FILE *fp)
{
    unsigned char *buffer = xmalloc(SETUP_GE_SIZE);
    unsigned char *pbuf;
    int i;

    pbuf = buffer;

    if (fread(buffer, SETUP_GE_SIZE, 1, fp) != 1)
    {
        xfree(buffer);
        return -1;
    }

    Setup->sysrev     = get_word(pbuf); pbuf += 2;
    Setup->options    = get_word(pbuf); pbuf += 2;
    Setup->autorenum  = get_word(pbuf); pbuf += 2;
    Setup->maxpktsize = get_word(pbuf); pbuf += 2;

    Setup->logstyle        = *pbuf++;
    Setup->oldnetmailboard = *pbuf++;
    Setup->oldbadboard     = *pbuf++;
    Setup->olddupboard     = *pbuf++;
    Setup->recoveryboard   = *pbuf++;
    Setup->filebuffer      = *pbuf++;
    Setup->days            = *pbuf++;
    Setup->swapping        = *pbuf++;
    Setup->compr_default   = *pbuf++;

    memcpy(Setup->pmcolor, pbuf, 15); pbuf += 15;

    for (i=0; i < OLDAKAS; i++)
    {
        Setup->oldaka[i].zone  = get_word(pbuf); pbuf += 2;
        Setup->oldaka[i].net   = get_word(pbuf); pbuf += 2;
        Setup->oldaka[i].node  = get_word(pbuf); pbuf += 2;
        Setup->oldaka[i].point = get_word(pbuf); pbuf += 2;
    }

    for (i=0; i < OLDAKAS; i++)
    {
        Setup->oldpointnet[i] = get_word(pbuf); pbuf += 2;
    }

    Setup->gekey  = get_dword(pbuf); pbuf += 4;
    Setup->mbukey = get_dword(pbuf); pbuf += 4;

    memcpy(Setup->geregto, pbuf, 51); pbuf += 51;
    memcpy(Setup->mburegto, pbuf, 51); pbuf += 51;
    for (i = 0; i < USERS; i++)
    {
        memcpy(Setup->username[i], pbuf, 36); pbuf += 36;
    }
    memcpy(Setup->hmbpath, pbuf, 53); pbuf += 53;
    memcpy(Setup->mailpath, pbuf, 53); pbuf += 53;
    memcpy(Setup->inbound_path, pbuf, 53); pbuf += 53;
    memcpy(Setup->outbound_path, pbuf, 53); pbuf += 53;
    memcpy(Setup->echotoss_file, pbuf, 65); pbuf += 65;
    memcpy(Setup->nodepath, pbuf, 53); pbuf += 53;
    memcpy(Setup->areasfile, pbuf, 65); pbuf += 65;
    memcpy(Setup->logfile, pbuf, 65); pbuf += 65;
    memcpy(Setup->mgrlogfile, pbuf, 65); pbuf += 65;
    memcpy(Setup->swap_path, pbuf, 53); pbuf += 53;
    memcpy(Setup->tear_line, pbuf, 31); pbuf += 31;
    for (i = 0; i < 20; i++)
    {
        memcpy(Setup->originline[i], pbuf, 61); pbuf += 61;
    }
    for (i = 0; i < 10; i++)
    {
        memcpy(Setup->compr_prog[i], pbuf, 13); pbuf += 13;
    }
    for (i = 0; i < 10; i++)
    {
        memcpy(Setup->compr_switches[i], pbuf, 20); pbuf += 20;
    }
    for (i = 0; i < 10; i++)
    {
        memcpy(Setup->decompr_prog[i], pbuf, 13); pbuf += 13;
    }
    for (i = 0; i < 10; i++)
    {
        memcpy(Setup->decompr_switches[i], pbuf, 20); pbuf += 20;
    }
    for (i = 0; i < 26; i++)
    {
        memcpy(Setup->oldgroups[i], pbuf, 21); pbuf += 21;
    }

    Setup->lockmode = *pbuf++;

    memcpy(Setup->secure_path, pbuf, 53); pbuf += 53;
    memcpy(Setup->rcvdmailpath, pbuf, 53); pbuf += 53;
    memcpy(Setup->sentmailpath, pbuf, 53); pbuf += 53;
    memcpy(Setup->semaphorepath, pbuf, 53); pbuf += 53;

    Setup->version_major = *pbuf++;
    Setup->version_minor = *pbuf++;
    Setup->semaphore_mode = *pbuf++;

    memcpy(Setup->badecho_path, pbuf, 53); pbuf += 53;

    Setup->mailer_type = *pbuf++;

    Setup->loglevel = get_word(pbuf); pbuf += 2;

    for (i = 0; i < 20; i++)
    {
        Setup->akamatch[i].zone = get_word(pbuf); pbuf += 2;
        Setup->akamatch[i].net  = get_word(pbuf); pbuf += 2;
        Setup->akamatch[i].aka  = *pbuf++;
    }

    memcpy(Setup->mbulogfile, pbuf, 65); pbuf += 65;

    Setup->maxqqqs = get_word(pbuf); pbuf += 2;
    Setup->maxqqqopen = *pbuf++;
    Setup->maxhandles = *pbuf++;
    Setup->maxarcsize = get_word(pbuf); pbuf += 2;
    Setup->delfuture = get_word(pbuf); pbuf += 2;
    Setup->extraoptions = get_word(pbuf); pbuf += 2;
    Setup->firstboard = *pbuf++;
    Setup->reserved1 = get_word(pbuf); pbuf += 2;
    Setup->copy_persmail = get_word(pbuf); pbuf += 2;

    memcpy(Setup->oldpersmailboard, pbuf, USERS); pbuf += USERS;

    Setup->old_public_groups = get_dword(pbuf); pbuf += 4;
    Setup->dupentries = get_word(pbuf); pbuf += 2;
    Setup->oldrcvdboard = *pbuf++;
    Setup->oldsentboard = *pbuf++;

    memcpy(Setup->oldakaboard, pbuf, OLDAKAS); pbuf += OLDAKAS;
    memcpy(Setup->olduserboard, pbuf, USERS); pbuf += USERS;

    Setup->reserved2 = *pbuf++;

    for (i = 0; i < OLDUPLINKS; i++)
    {
        Setup->uplink[i].address.zone = get_word(pbuf); pbuf += 2;
        Setup->uplink[i].address.net = get_word(pbuf); pbuf += 2;
        Setup->uplink[i].address.node = get_word(pbuf); pbuf += 2;
        Setup->uplink[i].address.point = get_word(pbuf); pbuf += 2;

        memcpy(Setup->uplink[i].areafix, pbuf, 9); pbuf += 9;
        memcpy(Setup->uplink[i].password, pbuf, 17); pbuf += 17;
        memcpy(Setup->uplink[i].filename, pbuf, 13); pbuf += 13;
        memcpy(Setup->uplink[i].unused, pbuf, 6); pbuf += 6;

        Setup->uplink[i].options = *pbuf++;
        Setup->uplink[i].filetype = *pbuf++;
        Setup->uplink[i].groups = get_dword(pbuf); pbuf += 4;
        Setup->uplink[i].origin = *pbuf++;
    }

    memcpy(Setup->persmail_path, pbuf, 53); pbuf += 53;
    memcpy(Setup->outpkts_path, pbuf, 53); pbuf += 53;

    for (i = 0; i < 10; i++)
    {
        Setup->compr_mem[i] = get_word(pbuf); pbuf += 2;
    }
    for (i = 0; i < 10; i++)
    {
        Setup->decompr_mem[i] = get_word(pbuf); pbuf += 2;
    }

    Setup->pwdcrc = get_dword(pbuf); pbuf += 4;
    Setup->default_maxmsgs = get_word(pbuf); pbuf += 2;
    Setup->default_maxdays = get_word(pbuf); pbuf += 2;

    memcpy(Setup->gus_prog, pbuf, 13); pbuf += 13;
    memcpy(Setup->gus_switches, pbuf, 20); pbuf += 20;

    Setup->gus_mem = get_word(pbuf); pbuf += 2;
    Setup->default_maxrcvddays = get_word(pbuf); pbuf += 2;
    Setup->checkname = *pbuf++;
    Setup->maxareacachesize = *pbuf++;

    memcpy(Setup->inpkts_path, pbuf, 53); pbuf += 53;
    memcpy(Setup->pkt_prog, pbuf, 13); pbuf += 13;
    memcpy(Setup->pkt_switches, pbuf, 20); pbuf += 20;

    Setup->pkt_mem = get_word(pbuf); pbuf += 2;
    Setup->maxareas = get_word(pbuf); pbuf += 2;
    Setup->maxconnections = get_word(pbuf); pbuf += 2;
    Setup->maxnodes = get_word(pbuf); pbuf += 2;
    Setup->default_minmsgs = get_word(pbuf); pbuf += 2;

    Setup->bbs_type = *pbuf++;
    Setup->decompress_ext = *pbuf++;
    Setup->reserved3 = *pbuf++;
    Setup->change_tearline = *pbuf++;
    Setup->prog_notavail = get_word(pbuf); pbuf += 2;

    Setup->gscolor.bg_char = *pbuf++;
    Setup->gscolor.headerframe = *pbuf++;
    Setup->gscolor.headertext = *pbuf++;
    Setup->gscolor.background = *pbuf++;
    Setup->gscolor.bottomline = *pbuf++;
    Setup->gscolor.bottomtext = *pbuf++;
    Setup->gscolor.bottomkey = *pbuf++;
    Setup->gscolor.errorframe = *pbuf++;
    Setup->gscolor.errortext = *pbuf++;
    Setup->gscolor.helpframe = *pbuf++;
    Setup->gscolor.helptitle = *pbuf++;
    Setup->gscolor.helptext = *pbuf++;
    Setup->gscolor.helpfound = *pbuf++;
    Setup->gscolor.winframe = *pbuf++;
    Setup->gscolor.wintitle = *pbuf++;
    Setup->gscolor.winline = *pbuf++;
    Setup->gscolor.wintext = *pbuf++;
    Setup->gscolor.winkey = *pbuf++;
    Setup->gscolor.windata = *pbuf++;
    Setup->gscolor.winselect = *pbuf++;
    Setup->gscolor.inputdata = *pbuf++;
    Setup->gscolor.exportonly = *pbuf++;
    Setup->gscolor.importonly = *pbuf++;
    Setup->gscolor.lockedout = *pbuf++;

    memcpy(Setup->reserved4, pbuf, 9); pbuf += 9;

    for (i = 0; i < AKAS; i++)
    {
        Setup->aka[i].zone  = get_word(pbuf); pbuf += 2;
        Setup->aka[i].net   = get_word(pbuf); pbuf += 2;
        Setup->aka[i].node  = get_word(pbuf); pbuf += 2;
        Setup->aka[i].point = get_word(pbuf); pbuf += 2;
    }
    for (i = 0; i < AKAS; i++)
    {
        Setup->pointnet[i]  = get_word(pbuf); pbuf += 2;
    }
    for (i = 0; i < AKAS; i++)
    {
        Setup->akaarea[i]  = get_word(pbuf); pbuf += 2;
    }
    for (i = 0; i < USERS; i++)
    {
        Setup->userarea[i]  = get_word(pbuf); pbuf += 2;
    }
    for (i = 0; i < USERS; i++)
    {
        Setup->persmailarea[i]  = get_word(pbuf); pbuf += 2;
    }

    Setup->rcvdarea = get_word(pbuf); pbuf += 2;
    Setup->sentarea = get_word(pbuf); pbuf += 2;
    Setup->badarea = get_word(pbuf); pbuf += 2;
    Setup->reserved5 = get_word(pbuf); pbuf += 2;

    memcpy(Setup->jampath, pbuf, 53); pbuf += 53;
    memcpy(Setup->userbase, pbuf, 53); pbuf += 53;
    memcpy(Setup->dos4gw_exe, pbuf, 65); pbuf += 65;

    memcpy(Setup->public_groups, pbuf, GROUPBYTES); pbuf += GROUPBYTES;

    Setup->maxgroupconnections = get_word(pbuf); pbuf += 2;
    Setup->maxmsgsize = get_word(pbuf); pbuf += 2;
    Setup->diskspace_threshold = get_word(pbuf); pbuf += 2;
    Setup->pktsort = *pbuf++;

    memcpy(Setup->wildcatpath, pbuf, 53); pbuf += 53;

    assert(pbuf - buffer == SETUP_GE_SIZE);

    xfree(buffer);

    return 0;
}

/*
 * read_areafile_hdr
 *
 * reads an AREAFILE_HDR structure.
 *
 */

int read_areafile_hdr(AREAFILE_HDR *AreaHdr, FILE *fp)
{
    unsigned char buffer[AREAFILE_HDR_SIZE], *pbuf = buffer;

    if (fread(buffer, AREAFILE_HDR_SIZE, 1, fp) != 1)
    {
        return -1;
    }

    AreaHdr->hdrsize        = get_word(pbuf); pbuf += 2;
    AreaHdr->recsize        = get_word(pbuf); pbuf += 2;
    AreaHdr->maxconnections = get_word(pbuf); pbuf += 2;

    assert(pbuf - buffer == AREAFILE_HDR_SIZE);

    return 0;
}


/*
 * read_areafile_ge
 *
 * reads a AREAFILE_GE structure.
 *
 */

int read_areafile_ge(AREAFILE_GE *Area, FILE *fp)
{
    unsigned char buffer[AREAFILE_GE_SIZE], *pbuf = buffer;

    if (fread(buffer, AREAFILE_GE_SIZE, 1, fp) != 1)
    {
        return -1;
    }

    memcpy(Area->name, pbuf, 51); pbuf += 51;
    memcpy(Area->comment, pbuf, 61); pbuf += 61;
    memcpy(Area->path, pbuf, 51); pbuf += 51;
    memcpy(Area->originline, pbuf, 61); pbuf += 61;

    Area->areanumber = get_word(pbuf); pbuf += 2;
    Area->group = *pbuf++;
    Area->options = get_word(pbuf); pbuf += 2;
    Area->originlinenr = *pbuf++;
    Area->pkt_origin = *pbuf++;
    Area->seenbys = get_dword(pbuf); pbuf += 4;
    Area->maxmsgs = get_word(pbuf); pbuf += 2;
    Area->maxdays = get_word(pbuf); pbuf += 2;
    Area->maxrcvddays = get_word(pbuf); pbuf += 2;
    Area->areatype = *pbuf++;
    Area->areaformat = *pbuf++;
    Area->extraoptions = *pbuf++;

    assert(pbuf - buffer == AREAFILE_GE_SIZE);

    return 0;
}
