/*
 *  fconf.c
 *
 *  Written by Tobias Ernst et. al.
 *  Released to the public domain.
 *
 *  Reads a husky project fidoconfig config file.
 *
 *  There are two ways of reading the fidoconfig file: If the macro
 *  USE_FIDOCONFIG is defined, we will use the routines of the fidoconfig
 *  library. This is very easy, but as soon as a single new keyword is
 *  introduced to fidoconfig, you must recompile (or at least relink, but
 *  usually the interface also changes) Msged, because otherwise the
 *  library routines will complain about unknown keywords.
 *
 *  Therefore, this file also contains routines that directly parse the
 *  fidoconfig file and simply ignore all unknown keywords. This is good for
 *  doing binary releases, as this code has a high chance of continuing to work
 *  even if the fidoconfig library is modified and new keywords are introduced.
 *  It is even resistant against the addition of new flags to area definitions
 *  - but of course if area definitions would be changed fundamentally, this
 *  code has to be adapted.
 *
 *  Using the USE_FIDOCONFIG code is your choice if you compile Msged on your
 *  own and have no problem to update and recompile Msged every time you
 *  upgrade your fidoconfig and other Husky sources.
 *
 *  For doing binary releases, or if you don't want to regularly update Msged
 *  along with the other tools, you should not use the USE_FIDOCONFIG code.
 *
 */

#ifdef USE_FIDOCONFIG
#include <fidoconf/fidoconf.h>
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
#include "group.h"
#include "environ.h"

#ifdef USE_FIDOCONFIG

/* ===================================================================== */
/* Part 1: Fidoconfig routines that use the Fidoconfig library           */
/* ===================================================================== */

static void fc_copy_address(ADDRESS *a, hs_addr *fc_a)
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
        && fc_area->msgbType != MSGTYPE_JAM
#endif
        )
    {
        return;
    }

    fc_copy_address(&(a.addr), fc_area->useAka);

    a.tag = xstrdup(fc_area->areaName);
    a.description=makeareadesc(fc_area->areaName, fc_area->description);

    a.path = xstrdup(fc_area->fileName);

    if ((fc_area->group != NULL) && (SW->areafilegroups) &&
        (strcmp(fc_area->group, "\060") != 0))
    {
        a.group = group_gethandle(fc_area->group, 1);
    }
    else
    {
        a.group = 0;
    }

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
#ifdef USE_MSGAPI
    case MSGTYPE_SQUISH:
        a.msgtype = SQUISH;
        break;
    case MSGTYPE_JAM:
        a.msgtype = JAM;
        break;
#endif
    default:  /* should never get here */
        abort();
    }

    applyflags(&a, areafileflags);
    AddArea(&a);
}

void check_fidoconfig(char *option_string)
{
#ifndef USE_FIDOCONFIG
    printf("\r\aError! This version of "PROG" has been compiled\n"
           "without support for the FIDOCONFIG standard.\n");
    exit(-1);
#else

    s_fidoconfig *fc_config = readConfig(NULL);
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
	    if (fc_config->netMailAreaCount > 0)
	    {
	        release(ST->freqarea);
		ST->freqarea = xstrdup(fc_config->netMailAreas[0].areaName);
	    }

                                /* fido user list */
            if (fc_config->nodelistDir != NULL &&
                fc_config->fidoUserList != NULL)
            {
                release(ST->fidolist);
                ST->fidolist = xmalloc(strlen(fc_config->nodelistDir)+
                                       strlen(fc_config->fidoUserList) + 1);
                strcpy(ST->fidolist, fc_config->nodelistDir);
                strcat(ST->fidolist, fc_config->fidoUserList);
            }
        }
        if (check_type & 2)     /* load areas */
        {
                                /* netmail, dupe, bad */

            fc_add_area(&(fc_config->dupeArea), 0, 1);
            fc_add_area(&(fc_config->badArea), 0, 1);

                                /* netmail areas */
            for (i=0; i<fc_config->netMailAreaCount; i++)
            {
                fc_area = &(fc_config->netMailAreas[i]);
                if (fc_area->msgbType != MSGTYPE_PASSTHROUGH)
                {
                    fc_add_area(fc_area, 1, 0);
                }
            }

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
#else

/* ===================================================================== */
/* Part 2: Fidoconfig routines that directly parse the Fidoconfig file   */
/* ===================================================================== */

static void read_fidoconfig_file (char *filename, int check_type);
static ADDRESS fc_default_address;
static int fc_default_address_set;
static char *fc_config_nodelistDir;
static char *fc_config_fidoUserList;

void check_fidoconfig(char *option_string)
{
    char *filename;
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

    filename = getenv("FIDOCONFIG");

    if (filename == NULL)
    {
        printf ("\r\nError: You must set the FIDOCONFIG environment variable!\n");
        return;
    }

    fc_default_address_set = 0;
    memset(&fc_default_address, 0, sizeof(ADDRESS));
    release (fc_default_address.domain);

    fc_config_nodelistDir = NULL;
    fc_config_fidoUserList = NULL;

    read_fidoconfig_file(filename, check_type);

    if (fc_config_nodelistDir != NULL &&
        fc_config_fidoUserList != NULL)
    {
        release(ST->userlist);
        ST->userlist = xmalloc(strlen(fc_config_nodelistDir) +
                               strlen(fc_config_fidoUserList) + 1);
        strcpy(ST->userlist, fc_config_nodelistDir);
        strcat(ST->userlist, fc_config_fidoUserList);
    }
    release(fc_config_nodelistDir);
    release(fc_config_fidoUserList);
}

static char *get_rest_of_line(void)
{
    static char *rest;
    char *ptr;
    int len=0;

    if ((rest = strtok(NULL, "")) == NULL)
    {
        return NULL;
    }

    for (ptr = rest; *ptr == ' ' || *ptr == '\t'; ptr ++);

    if ((len = strlen(ptr)) == 0)
    {
        return NULL;
    }

    len --;

    while (len >= 0 && (ptr[len] == ' ' || ptr[len] =='\t'))
    {
        len--;
    }
    ptr[len + 1] = '\0';

    return ptr;
}


static void parse_fc_sysop(void)
{
    int i;
    char *sysop = get_rest_of_line();

    if (sysop)
    {
        for (i = 0; i < MAXUSERS; i++)
        {
            if (user_list[i].name == NULL)
            {
                break;
            }
        }

        if (i < MAXUSERS)
        {
            user_list[i].name = xstrdup(sysop);
            if (i == 0)
            {
                release(ST->username);
                ST->username = xstrdup(user_list[i].name);
                SW->useroffset = user_list[i].offset;
            }
        }
    }
}


static void parse_fc_address(int check_type)
{
    char *token = strtok(NULL, " \t");
    ADDRESS tmp;
    tmp = parsenode(token);

    if (token == NULL)
    {
        printf ("\r\nFidoconfig address statement missing argument.\n");
        return;
    }

    if (!fc_default_address_set)
    {
        fc_default_address_set = 1;
        copy_addr(&fc_default_address, &tmp);
    }

    if (check_type & 1) /* load settings */
    {
        alias = xrealloc(alias, (++SW->aliascount) * sizeof(ADDRESS));
        memset(alias + SW->aliascount - 1, 0, sizeof(ADDRESS));
        copy_addr(alias + SW->aliascount - 1, &tmp);
    }
    release(tmp.domain);
}

static void parse_fc_tosslog(void)
{
    char *tosslog = get_rest_of_line();

    if (tosslog)
    {
        release(ST->echotoss);
        ST->echotoss = pathcvt(xstrdup(tosslog));
    }
}

static void parse_fc_fidouserlist()
{
    fc_config_fidoUserList = xstrdup(get_rest_of_line());
}

static void parse_fc_nodelistdir()
{
    fc_config_nodelistDir = xstrdup(get_rest_of_line());
}

static void parse_fc_include(int check_type)
{
    char *token = strtok(NULL, " \t");
    char *fn;
    char *duptoken;

    if (token != NULL)
    {
        duptoken = xstrdup(token);
        fn = pathcvt(duptoken);
        read_fidoconfig_file(fn, check_type);
        xfree(fn);
    }
    else
    {
        printf ("\r\nFidoconfig include statement missing argument.\n");
    }
}

static char *fc_get_description(char *firsttoken)
{

    static char desc[257];
    char *token = firsttoken;
    int len = 0;

    *desc = '\0';
    while(token != NULL)
    {
        if (len + 1 >= sizeof(desc))
        {
            return NULL;
        }
        if (*desc)
        {
            desc[len++] = ' '; desc[len] = '\0';
        }
        else
        {
            if (*token=='"')
            {
                token++;
            }
        }
        if (len + strlen(token) >= sizeof(desc))
        {
            return NULL;
        }
        strcpy(desc + len, token);
        len += strlen(token);

        if (!len || desc[len - 1]!='"')
        {
            token=strtok(NULL, " \t");
        }
        else
        {
            token = NULL;
            if (len)
            {
                desc[len - 1] = '\0';
            }
        }
    }

    return desc;
}



static void parse_fc_area(int type)
{
    static AREA a;
    char *area_description = NULL;
    char *token;
    int option;

    memset(&a, 0, sizeof(AREA));

    token = strtok(NULL, " \t");
    if (token == NULL)
    {
        printf ("\r\nFidoconfig *area statement missing argument.\n");
        return;
    }

    a.tag = xstrdup(token);

    token = strtok(NULL, " \t");
    if (token == NULL)
    {
        xfree(a.tag);
        printf ("\r\nFidoconfig *area statement missing argument.\n");
        return;
    }
    else if (!stricmp(token, "passthrough"))
    {
        xfree(a.tag);
        return;
    }

    a.path = pathcvt(xstrdup(token));

    copy_addr(&(a.addr), &(fc_default_address));

    switch(type)
    {
    case 1:
        a.netmail = 1;
	a.priv    = 1;
        break;
    case 2:
        a.local = 1;
        break;
    case 3:
        a.echomail = 1;
        break;
    }

    a.msgtype = FIDO;

    token = strtok(NULL, " \t");
    while (token != NULL)
    {
        if (token[0] == '-')
        {
            option = 0;
            if (!stricmp(token + 1, "b"))
            {
                option = 1;
            }
            else if (!stricmp(token + 1, "a"))
            {
                option = 2;
            }
            else if (!stricmp(token + 1, "g"))
            {
                option = 3;
            }
            else if (!stricmp(token + 1, "d"))
            {
                option = 4;
            }

            if (option)
            {
                token = strtok(NULL, " \t");
                if (token != NULL)
                {
                    switch(option)
                    {
                    case 1:
                        if (!stricmp(token, "msg"))
                        {
                            a.msgtype = FIDO;
                        }
#ifdef USE_MSGAPI
                        else if (!stricmp(token, "squish"))
                        {
                            a.msgtype = SQUISH;
                        }
                        else if (!stricmp(token, "jam"))
                        {
                            a.msgtype = JAM;
                        }
#endif
                        else
                        {
                            release(a.tag);
                            release(a.path);
                            release(a.addr.domain);
                            return;
                        }
                        break;

                    case 2:
                        release(a.addr.domain);
                        a.addr = parsenode(token);
                        break;

                    case 3:
                        a.group = group_gethandle(token, 1);
                        break;

                    case 4:
                        area_description = fc_get_description(token);
                        break;
                    }
                }
            }
        }
        token = strtok(NULL, " \t");
    }

    a.description=makeareadesc(a.tag, area_description);
    applyflags(&a, areafileflags);
    AddArea(&a);
}

static void parse_fc_line(char *line, int check_type)
{
    char *token;

    if (!(*line))
    {
        return;
    }

    token = strtok(line, " \t");

    if (token == NULL)
    {
        return;
    }

    if ((check_type & 2) &&
        ((!stricmp(token, "netmailarea")) ||
         (!stricmp(token, "netarea"))))
    {
        parse_fc_area(1); /* netmail folders */
    }
    else if ((check_type && 2) &&
             ((!stricmp(token, "dupearea")) ||
              (!stricmp(token, "badarea")) ||
              (!stricmp(token, "localarea"))))
    {
        parse_fc_area(2); /* local folders */
    }
    else if ((check_type && 2) &&
             (!stricmp(token, "echoarea")))
    {
        parse_fc_area(3); /* echomail folders */
    }
    else if ((!stricmp(token, "include")))
    {
        parse_fc_include(check_type);
    }
    else if ((!stricmp(token, "address")))
    {
        parse_fc_address(check_type);
    }
    else if ((check_type && 1) &&
             (!stricmp(token, "sysop")))
    {
        parse_fc_sysop();
    }
    else if ((check_type && 1) &&
             (!stricmp(token, "echotosslog")))
    {
        parse_fc_tosslog();
    }
    else if ((check_type && 1) &&
             (!stricmp(token, "fidouserlist")))
    {
        parse_fc_fidouserlist();
    }
    else if ((check_type && 1) &&
             (!stricmp(token, "nodelistdir")))
    {
        parse_fc_nodelistdir();
    }
    else
    {
        /* unknown token */
        ;
    }

    return;
}

static void read_fidoconfig_file (char *filename, int check_type)
{
    FILE *f = fopen(filename, "r");
    static char *line = NULL; /* uh, oh, care for reentrance! */
    char *start;
    char *expanded_line;
    size_t l;
    int c;

    if (line == NULL) line = xmalloc(2048);

    if (f == NULL)
    {
        printf ("\r\nError: Can't open %s while parsing fidoconfig file.\n",
                filename);
        return;
    }

    while(fgets(line, 2048, f) != NULL)
    {
        l = strlen(line);

        /* handle trailing \n */
        if (l)
        {
            if (line[l-1] != '\n')
            {
                /* eat up superfluous characters in extra long line */
                do
                {
                    c = fgetc(f);
                } while (c != '\n' && c != EOF);
            }
            else
            {
                line[l-1] = '\0';
            }
        }

        /* trim spaces at beginning */
        for (start = line;
             ((*start == ' ') || (*start == '\t') || (*start == (char)0xFE));
             start++, l--);
        memmove(line, start, l+1);

        /* trim trailing spaces */
        while (l && (line[l - 1] == ' ' || line[l - 1] == '\t'))
        {
            line[l - 1] = '\0';
            l--;
        }

        /* strip comments */
        if (*line == '#')
        {
            *line='\0';
        }
        else
        {
            start = strchr(line,'#');
            if ((start != NULL) && (*(start - 1)==' ' || *(start - 1) == '\t'))
            {
                *(start - 1) = '\0';
            }
        }

        expanded_line = env_expand(line); /* expand %ENVIRONMENT% variables */
        parse_fc_line(expanded_line, check_type);
        xfree(expanded_line);
    }

    fclose(f);
}
#endif
