/*
 *  fconf.c
 *
 *  Written on 08-Jan-98 by Tobias Ernst.
 *  Released to the public domain.
 *
 *  Reads a husky project fidoconfig config file.
 */

#ifdef USE_FIDOCONFIG
#include "fidoconfig.h"
#else
#include <stdlib.h>
#include <stdio.h>
#endif

#include <time.h>
#include <string.h>
#include "version.h"
#include "addr.h"
#include "areas.h"
#include "nedit.h"
#include "msged.h"
#include "strextra.h"
#include "memextra.h"
#include "config.h"
#include "fconf.h"
#include "version.h"

#ifdef USE_FIDOCONFIG
static void fc_copy_address(ADDRESS *a, s_addr *fc_a)
{  
    memset(a, 0, sizeof(ADDRESS));

    a->zone  = fc_a->zone;
    a->net   = fc_a->net;
    a->node  = fc_a->node;
    a->point = fc_a->point;

    a->fidonet = 1;

    if (fc_a->domain != NULL && *(fc_a->domain))
    {
        a->domain = xstrdup(fc_a->domain);
    }
    else
    {
        a->domain = NULL;
    }
}

static void fc_add_area(s_area *fc_area, int netmail, int local)
{
    static AREA a;

    memset(&a, 0, sizeof a);

    if (fc_area->msgbType != MSGTYPE_SDM
#ifdef USE_MSGAPI
        && fc_area->msgbType != MSGTYPE_SQUISH
#endif
        )
    {
        return;
    }

    fc_copy_address(&(a.addr), fc_area->useAka);

    a.description = xstrdup(fc_area->areaName);
    a.tag = xstrdup(a.description);
    a.path = xstrdup(fc_area->fileName);

    if (netmail)
    {
        a.netmail = 1; a.priv = 1;
    }
    else if (local)
    {
        a.local = 1;
    }
    else
    {
        a.echomail = 1;
    }

    switch (fc_area->msgbType)
    {
    case MSGTYPE_SDM:
        a.msgtype = FIDO;
        break;
    case MSGTYPE_SQUISH:
        a.msgtype = SQUISH;
        break;
    default:  /* should never get here */
        abort();
    }

    applyflags(&a, areafileflags);
    AddArea(&a);
}
#endif

void check_fidoconfig(char *option_string)
{
#ifndef USE_FIDOCONFIG
    printf("\r\aError! This version of "PROG" has been compiled\n"
           "without support for the FIDOCONFIG standard.\n");
    exit(-1);
#else

    s_fidoconfig *fc_config = readConfig();
    s_area       *fc_area;
    int i;
    int check_type;

    if (option_string != NULL && !stricmp(option_string, "settings"))
    {
        check_type = 1;
    }
    else if (option_string != NULL && !stricmp(option_string, "both"))
    {
        check_type = 3;
    }
    else /* Default: Load areas only */
    {
       check_type = 2;
    }
    
    if (fc_config != NULL)                    
    {
        if (check_type & 1)     /* load settings */
        {
                                /* sysop name */
            for (i = 0; i < MAXUSERS; i++)
            {
                if (user_list[i].name == NULL)
                {
                    break;
                }
            }
            if (i < MAXUSERS)
            {
                user_list[i].name = xstrdup(fc_config->sysop);
                if (i == 0)
                {
                    release(ST->username);
                    ST->username = xstrdup(user_list[i].name);
                    SW->useroffset = user_list[i].offset;
                }
            }

                                /* addresses */
            if (fc_config->addrCount)
            {
                alias = xrealloc(alias,
                                 (SW->aliascount + fc_config->addrCount) *
                                 sizeof (ADDRESS));
                
                for (i = 0; i < fc_config->addrCount; i++)
                {
                    fc_copy_address(alias + SW->aliascount + i,
                                    fc_config->addr + i);
                }
                SW->aliascount += fc_config->addrCount;
            }

                                /* echotoss log */
            if (fc_config->echotosslog !=NULL)
            {
                release(ST->echotoss);
                ST->echotoss =
                    pathcvt(xstrdup(fc_config->echotosslog));
            }

                                /* area to place file requests in */
            release(ST->freqarea);
            ST->freqarea = xstrdup(fc_config->netMailArea.areaName);
        }
        if (check_type & 2)     /* load areas */
        {
                                /* netmail, dupe, bad */

            fc_add_area(&(fc_config->netMailArea), 1, 0);
            fc_add_area(&(fc_config->dupeArea), 0, 1);
            fc_add_area(&(fc_config->badArea), 0, 1);
            
                                /* local areas */
            for (i=0; i<fc_config->localAreaCount; i++)
            {
                fc_area = &(fc_config->localAreas[i]);
                if (fc_area->msgbType != MSGTYPE_PASSTHROUGH)
                {
                    fc_add_area(fc_area, 0, 1);
                }
            }
      
                                /* echomail areas */
            for (i=0; i<fc_config->echoAreaCount; i++)
            {
                fc_area = &(fc_config->echoAreas[i]);
                if (fc_area->msgbType != MSGTYPE_PASSTHROUGH)
                {
                    fc_add_area(fc_area, 0, 0);
                }
            }
        }

        disposeConfig(fc_config);
    }
    else
    {
        printf ("\r\aError! Cannot open fidoconfig!\n");
        exit(-1);
    }
#endif
}
