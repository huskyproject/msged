
/*
 * fecfg145.c
 *
 * This module contains routines that read in structures from
 * FASTEECHO configuration files in a portable way, i.E. they will even
 * work in cases where a fread(&structure, sizeof(structure), 1, stream)
 * would fail because of structure packing or big endian problems.
 *
 * Only those structures that are of importance to MsgEd have been
 * implemented.
 *
 * Written 04-Oct-98 by Tobias Ernst and donated to the Public Domain.
 *
 */

#include <assert.h>
#include <stdio.h>

#include "fecfg145.h"
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

int read_fe_config(CONFIG *c, FILE *fp)
{
    unsigned char *buffer = xmalloc(FE_CONFIG_SIZE);
    unsigned char *pbuf;
    int i;

    pbuf = buffer;

    if (fread(buffer, FE_CONFIG_SIZE, 1, fp) != 1)
    {
        xfree(buffer);
        return -1;
    }

    c->revision = get_word(pbuf); pbuf += 2;
    c->flags    = get_dword(pbuf); pbuf += 4;
    c->NodeCnt  = get_word(pbuf); pbuf += 2;
    c->AreaCnt  = get_word(pbuf); pbuf += 2;
    c->unused1  = get_word(pbuf); pbuf += 2;

    memcpy(c->NetMPath, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->MsgBase, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->InBound, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->OutBound, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->Unpacker, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->LogFile, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->unused2, pbuf, 336); pbuf += 336;
    memcpy(c->Unpacker2, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->UnprotInBound, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->StatFile, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->SwapPath, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->SemaphorePath, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->BBSConfigPath, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->DBQueuePath, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->unused3, pbuf, 32); pbuf += 32;
    memcpy(c->RetearTo, pbuf, 40); pbuf += 40;
    memcpy(c->LocalInBound, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->ExtAfter, pbuf, _MAXPATH - 4); pbuf += _MAXPATH - 4;
    memcpy(c->ExtBefore, pbuf, _MAXPATH - 4); pbuf += _MAXPATH - 4;
    memcpy(c->unused4, pbuf, 480); pbuf += 480;

    for (i = 0; i < 10; i++)
    {
        c->CC[i].what = *pbuf++;
        memcpy(c->CC[i].object, pbuf, 31); pbuf += 31;
        c->CC[i].conference = get_word(pbuf); pbuf += 2;
    }

    c->security = *pbuf++;
    c->loglevel = *pbuf++;
    c->def_days = get_word(pbuf); pbuf += 2;
    c->def_messages = get_word(pbuf); pbuf += 2;
    memcpy(c->unused5, pbuf, 462); pbuf += 462;
    c->autorenum = get_word(pbuf); pbuf += 2;
    c->def_recvdays = get_word(pbuf); pbuf += 2;
    c->openQQQs = *pbuf++;
    c->Swapping = *pbuf++;
    c->compressafter = get_word(pbuf); pbuf += 2;
    c->afixmaxmsglen = get_word(pbuf); pbuf += 2;
    c->compressfree = get_word(pbuf); pbuf += 2;

    memcpy(c->TempPath, pbuf, _MAXPATH); pbuf += _MAXPATH;

    c->graphics = *pbuf++;
    c->BBSSoftware = *pbuf++;

    memcpy(c->AreaFixHelp, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->unused6, pbuf, 504); pbuf += 504;

    c->AreaFixFlags = get_word(pbuf); pbuf += 2;
    c->QuietLevel = *pbuf++;
    c->Buffers = *pbuf++;
    c->FWACnt = *pbuf++;
    c->GDCnt = *pbuf++;

    c->rescan_def.flags = get_word(pbuf); pbuf += 2;
    c->rescan_def.days[0] = get_word(pbuf); pbuf += 2;
    c->rescan_def.days[1] = get_word(pbuf); pbuf += 2;
    c->rescan_def.msgs[0] = get_word(pbuf); pbuf += 2;
    c->rescan_def.msgs[1] = get_word(pbuf); pbuf += 2;

    c->duperecords = get_dword(pbuf); pbuf += 4;

    c->arcext.inb = *pbuf++;
    c->arcext.outb = *pbuf++;

    c->AFixRcptLen = get_word(pbuf); pbuf += 2;
    c->AkaCnt = *pbuf++;
    c->resv = *pbuf++;
    c->maxPKT = get_word(pbuf); pbuf += 2;
    c->sharing = *pbuf++;
    c->sorting = *pbuf++;

    for (i=0; i < 11; i++)
    {
        memcpy(c->sysops[i].name, pbuf, 36); pbuf += 36;
        c->sysops[i].resv = get_dword(pbuf); pbuf += 4;
    }

    memcpy(c->AreaFixLog, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(c->TempInBound, pbuf, _MAXPATH); pbuf += _MAXPATH;

    c->maxPKTmsgs = get_word(pbuf); pbuf += 2;
    c->RouteCnt = get_word(pbuf); pbuf += 2;
    c->maxPACKratio = get_word(pbuf); pbuf += 2;
    c->PackerCnt = *pbuf++;
    c->UnpackerCnt = *pbuf++;
    c->GroupCnt = *pbuf++;
    c->OriginCnt = *pbuf++;
    c->mailer = get_word(pbuf); pbuf += 2;

    memcpy(c->reserved, pbuf, 810); pbuf += 810;

    c->AreaRecSize = get_word(pbuf); pbuf += 2;
    c->GrpDefRecSize = get_word(pbuf); pbuf += 2;
    c->MaxAreas = get_word(pbuf); pbuf += 2;
    c->MaxNodes = get_word(pbuf); pbuf += 2;
    c->NodeRecSize = get_word(pbuf); pbuf += 2;
    c->offset = get_dword(pbuf); pbuf += 4;

    assert(pbuf - buffer == FE_CONFIG_SIZE);

    xfree(buffer);
    return 0;
}

int read_fe_extension_header(ExtensionHeader *h, FILE *fp)
{
    unsigned char buffer[FE_EXTHEADER_SIZE];
    unsigned char *pbuf;

    pbuf = buffer;

    if (fread(buffer, FE_EXTHEADER_SIZE, 1, fp) != 1)
    {
        return -1;
    }

    h->type   = get_word(pbuf);  pbuf += 2;
    h->offset = get_dword(pbuf); pbuf += 4;

    assert(pbuf - buffer == FE_EXTHEADER_SIZE);

    return 0;
}

int read_fe_sysaddress(SysAddress *a, FILE *fp)
{
    unsigned char buffer[FE_SYS_ADDRESS_SIZE];
    unsigned char *pbuf;

    pbuf = buffer;

    if (fread(buffer, FE_SYS_ADDRESS_SIZE, 1, fp) != 1)
    {
        return -1;
    }

    a->main.zone = get_word(pbuf); pbuf += 2;
    a->main.net = get_word(pbuf); pbuf += 2;
    a->main.node = get_word(pbuf); pbuf += 2;
    a->main.point = get_word(pbuf); pbuf += 2;

    memcpy(a->domain, pbuf, 28); pbuf += 28;

    a->pointnet = get_word(pbuf); pbuf += 2;
    a->flags = get_dword(pbuf); pbuf += 4;

    assert(pbuf - buffer == FE_SYS_ADDRESS_SIZE);

    return 0;
}

int read_fe_area(Area *a, FILE *fp)
{
    unsigned char buffer[FE_AREA_SIZE];
    unsigned char *pbuf;
    word temp;

    pbuf = buffer;

    if (fread(buffer, FE_AREA_SIZE, 1, fp) != 1)
    {
        return -1;
    }

    memcpy(a->name, pbuf, 52); pbuf += 52;
    a->board = get_word(pbuf); pbuf += 2;
    a->conference = get_word(pbuf); pbuf += 2;
    a->read_sec = get_word(pbuf); pbuf += 2;
    a->write_sec = get_word(pbuf); pbuf += 2;

    temp = get_word(pbuf); pbuf += 2;
    a->info.aka = temp & 0x00FF;
    a->info.group = (temp >> 8) & 0x00FF;

    temp = get_word(pbuf); pbuf += 2;
    a->flags.storage = temp & 0x000F;
    a->flags.atype   = (temp >> 4) & 0x000F;
    a->flags.origin  = (temp >> 8) & 0x001F;
    a->flags.resv    = (temp >> 13) & 0x0007;

    temp = get_word(pbuf); pbuf += 2;
    a->advflags.autoadded  = temp & 1;
    a->advflags.tinyseen   = (temp >> 1) & 1;
    a->advflags.cpd        = (temp >> 2) & 1;
    a->advflags.passive    = (temp >> 3) & 1;
    a->advflags.keepseen   = (temp >> 4) & 1;
    a->advflags.mandatory  = (temp >> 5) & 1;
    a->advflags.keepsysop  = (temp >> 6) & 1;
    a->advflags.killread   = (temp >> 7) & 1;
    a->advflags.disablepsv = (temp >> 8) & 1;
    a->advflags.keepmails  = (temp >> 9) & 1;
    a->advflags.hide       = (temp >> 10) & 1;
    a->advflags.nomanual   = (temp >> 11) & 1;
    a->advflags.umlaut     = (temp >> 12) & 1;
    a->advflags.resv       = (temp >> 13) & 7;

    a->resv1 = get_word(pbuf); pbuf += 2;
    a->seenbys = get_dword(pbuf); pbuf += 4;
    a->resv2 = get_dword(pbuf); pbuf += 4;
    a->days = get_word(pbuf); pbuf += 2;
    a->messages = get_word(pbuf); pbuf += 2;
    a->recvdays = get_word(pbuf); pbuf += 2;
    memcpy(a->path, pbuf, _MAXPATH); pbuf += _MAXPATH;
    memcpy(a->desc, pbuf, 52); pbuf += 52;

    assert(pbuf - buffer == FE_AREA_SIZE);

    return 0;
}

