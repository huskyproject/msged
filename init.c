/*
 *  INIT.C
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Handles sub-system initialisation for Msged.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "fido.h"
#include "quick.h"
#ifdef USE_MSGAPI
#include "msg.h"
#endif
#include "winsys.h"
#define INCL_MAIN
#include "main.h"
#include "memextra.h"

struct _sv *string_vars;
struct _swv *switch_vars;

msghandle msgdo[] =
{
    {
        FidoMsgReadHeader,
        FidoMsgReadText,
        FidoMsgWriteHeader,
        FidoMsgWriteText,
        FidoMsgDelete,
        FidoAreaSetLast,
        FidoMsgAreaOpen,
        FidoMsgAreaClose,
        FidoMsgClose,
        FidoUidToMsgn,
        FidoMsgnToUid,
        FidoMsgLock,
        FidoMsgUnlock
    },
    {
        QuickMsgReadHeader,
        QuickMsgReadText,
        QuickMsgWriteHeader,
        QuickMsgWriteText,
        QuickMsgDelete,
        QuickAreaSetLast,
        QuickMsgAreaOpen,
        QuickMsgAreaClose,
        QuickMsgClose,
        QuickUidToMsgn,
        QuickMsgnToUid,
        QuickMsgLock,
        QuickMsgUnlock
    }
#ifdef USE_MSGAPI
    ,
    {
        SquishMsgReadHeader,
        SquishMsgReadText,
        SquishMsgWriteHeader,
        SquishMsgWriteText,
        SquishMsgDelete,
        SquishAreaSetLast,
        SquishMsgAreaOpen,
        SquishMsgAreaClose,
        SquishMsgClose,
        SquishUidToMsgn,
        SquishMsgnToUid,
        SquishMsgLock,
        SquishMsgUnlock
    },
    {                           /* JAM uses same routines as Squish,     */
                                /* because both are handled by the SMAPI */
        SquishMsgReadHeader,
        SquishMsgReadText,
        SquishMsgWriteHeader,
        SquishMsgWriteText,
        SquishMsgDelete,
        SquishAreaSetLast,
        SquishMsgAreaOpen,
        SquishMsgAreaClose,
        SquishMsgClose,
        SquishUidToMsgn,
        SquishMsgnToUid,
        SquishMsgLock,
        SquishMsgUnlock
    }

#endif
};

D_LIST *node_lists = NULL;      /* the nodelists recognized by the system */
AREA *arealist = NULL;          /* list of areas */
ALIAS *aliaslist = NULL;        /* list of aliases */
ADDRESS *domain_list = NULL;    /* list of domain-gates */
ADDRESS *alias = NULL;          /* list of akas */
msg *message = NULL;            /* current message */
char **templates = NULL;        /* templates in system */
char **origins = NULL;          /* origins for the origin shuffler */
int n_origins = 0;              /* number of origins */
USER user_list[MAXUSERS];       /* list of users */
ADDRESS uucp_gate;              /* the uucp gate */

unsigned int *macros[41];       /* function key macros + 1 for autostart */
int maxx;                       /* maximum screen columns */
int maxy;                       /* maximum screen rows */
int rot13;
int stripSoft;
int softcrxlat = 0;

#ifndef READMAPSDAT
#ifdef UNIX
#define READMAPSDAT "~/.msged.readmaps"
#define WRITMAPSDAT "~/.msged.writmaps"
#else
#define READMAPSDAT "readmaps.dat"
#define WRITMAPSDAT "writmaps.dat"
#endif
#endif

int InitVars(void)
{
    /* Allocate some memory & initialize it. */

    string_vars = xmalloc(sizeof(struct _sv));
    switch_vars = xmalloc(sizeof(struct _swv));

    memset(string_vars, 0, sizeof(struct _sv));
    memset(switch_vars, 0, sizeof(struct _swv));
    memset(user_list, 0, sizeof(user_list));
    memset(macros, 0, sizeof(macros));

    /* Initialize all the variables to default values. */

    SW->orgrm = 0xFFFF; /* allow any margin */
    SW->orgqm = 75;     /* don't make quote lines too large */
    SW->tabsize = 4;
    SW->use_lastr = YES;
    SW->qquote = YES;
    SW->msgids = YES;
    SW->opusdate = NO;
    SW->shownotes = NO;
    SW->showseenbys = NO;
    SW->showorigins = YES;
    SW->showtearlines = YES;
    SW->confirmations = YES;
    SW->datearrived = YES;
    SW->redraw = YES;
    SW->showaddr = YES;
    SW->rawcc = YES;
    SW->savecc = YES;
    SW->hardquote = YES;
    SW->chopquote = NO;
    SW->showcr = NO;
    SW->showeol = NO;
    SW->showrealmsgn = NO;
    SW->usemouse = YES;
    SW->tabexpand = YES;
    SW->editcronly = NO;
    SW->usepid = NO;
    SW->soteot = NO;
    SW->showtime = NO;
    SW->importfn = YES;
    SW->dmore = NO;
    SW->statbar = YES;
    SW->rquote = MT_QUO;
    SW->rotharea = MT_QUO | MT_ARC;
    SW->rfollow = MT_QUO | MT_FOL;
    SW->rextra = MT_QUO | MT_FOL | MT_ARC;
    SW->showsystem = YES;
    SW->extformat = YES;
    SW->arealistexactmatch = YES;
    SW->echoflags = YES;
    SW->netmailvia = YES;
    SW->domainorigin = YES;
    SW->rightnextunreadarea = NO;
    SW->usetearlines = YES;
    SW->useoriginlines = YES;
    SW->edittearlines = NO;
    SW->editoriginlines = NO;
    SW->squish_lock = NO;
    SW->lowercase = NO;
    SW->adaptivecase = NO;
    SW->receiveallnames = YES;
    SW->receivealladdr  = YES;
    SW->carthy = YES;
    SW->direct_list = NO;

#ifndef UNIX
    ST->comspec = getenv("COMSPEC");
#else
    ST->comspec = getenv("SHELL");
#endif    
    ST->outfile = xstrdup("msged.txt");
    ST->infile  = NULL;
    ST->quotestr = xstrdup(" > ");
    ST->echotoss = xstrdup("echotoss.log");
    ST->lastread = xstrdup("lastread");
    ST->cfgfile = xstrdup("msged.cfg");
    ST->uucpgate = xstrdup("UUCP");
    ST->editorName = NULL;
    ST->quickbbs = NULL;
    ST->nodebase = NULL;
    ST->sysop = NULL;
    ST->output_charset = NULL;
    ST->input_charset = NULL;
    ST->sort_criteria = NULL;
    ST->freqarea = NULL;
    ST->special_characters = NULL;
    ST->uucpreplyto = NULL;
    ST->freqflags = NULL;
    ST->printer = NULL;
    ST->readmap  = xstrdup(READMAPSDAT);
    ST->writemap = xstrdup(WRITMAPSDAT);

    uucp_gate.notfound = 1;

    return OK;
}

void DeinitMem(void)
{
    release(origins);
    n_origins = 0;

    if (string_vars != NULL)
    {
        release(string_vars->readmap);
        release(string_vars->writemap);
    }
    release(switch_vars);
    release(string_vars);
}

