/*
 *  ADDR.C
 *
 *  Released to the public domain.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "addr.h"
#include "config.h"
#include "nedit.h"
#include "memextra.h"
#include "msged.h"

char *show_address(ADDRESS * a)
{
    static char s[80];
    char field[20];
    char *truedomain=NULL;

    memset(s, 0, sizeof s);
    memset(field, 0, sizeof field);

    if (a->notfound)
    {
        strcpy(s, "0:0/0");
        return s;
    }
    if (a->fidonet)
    {
        if (a->zone)
        {
            sprintf(field, "%d", a->zone);
            strcat(s, field);
            strcat(s, ":");
        }
        sprintf(field, "%d", a->net);
        strcat(s, field);
        strcat(s, "/");
        sprintf(field, "%d", a->node);
        strcat(s, field);
        if (a->point)
        {
            strcat(s, ".");
            sprintf(field, "%d", a->point);
            strcat(s, field);
        }
        if (a->domain != NULL)
        {
            strcat(s, "@");
            strcat(s, a->domain);
        }
    }
    if (a->internet || a->bangpath)
    {
        parse_internet_address(a->domain,&truedomain,NULL);
        /* Note: Normally, truedomain should only contain the e-mail address.
                 However, during certain stages of header editing, it might
                 also contain the user name. Therefore, we parse the address
                 to make sure that really only the address is showed. */

    }
    if (a->internet)
    {
        strncpy(s, truedomain, 60);
    }
    if (a->bangpath)
    {
        char *t, *t1;

        t1 = strrchr(truedomain, '!');
        if (t1 == NULL)
        {
            strcpy(s, truedomain);
        }
        else
        {
            *t1 = '\0';
            t = strrchr(truedomain, '!');
            if (!t)
            {
                t = truedomain;
            }
            *t1 = '!';
            strcat(strcpy(s, "..."), t);
        }

    }
    release(truedomain);
    return s;
}

char *show_4d(ADDRESS * a)
{
    static char s[80];
    char field[20];

    memset(s, 0, sizeof s);
    memset(field, 0, sizeof field);

    if (a->notfound)
    {
        strcpy(s, "0:0/0");
        return s;
    }
    if (a->fidonet)
    {
        if (a->zone)
        {
            sprintf(field, "%d", a->zone);
            strcat(s, field);
            strcat(s, ":");
        }
        sprintf(field, "%d", a->net);
        strcat(s, field);
        strcat(s, "/");
        sprintf(field, "%d", a->node);
        strcat(s, field);
        if (a->point)
        {
            strcat(s, ".");
            sprintf(field, "%d", a->point);
            strcat(s, field);
        }
    }
    if (a->internet)
    {
        strncpy(s, a->domain, 60);
    }
    if (a->bangpath)
    {
        char *t, *t1;

        t1 = strrchr(a->domain, '!');
        if (t1 == NULL)
        {
            strcpy(s, a->domain);
        }
        else
        {
            *t1 = '\0';
            t = strrchr(a->domain, '!');
            if (!t)
            {
                t = a->domain;
            }
            *t1 = '!';
            strcat(strcpy(s, "..."), t);
        }

    }
    return s;
}

ADDRESS parsenode(char *t)
{
    ADDRESS tmp;
    int n, point = 0;
    char *s, ch;

    if (SW->areas)
    {
        tmp = CurArea.addr;
    }
    else
    {
        tmp = thisnode;
    }

    tmp.point = tmp.notfound = 0;
    tmp.fidonet = 1;
    tmp.internet = 0;
    tmp.bangpath = 0;
    tmp.domain = NULL;

    if (t == NULL)
    {
        tmp.notfound = 1;
        return tmp;
    }

    while (isspace(*t))
    {
        t++;
    }

    if (!isdigit(*t) && (*t != '.'))
    {
        tmp.notfound = 1;
        tmp.fidonet = 0;
        tmp.internet = 0;
        tmp.bangpath = 0;
        return tmp;
    }

    if (*t == '.')
    {
        tmp.net = CurArea.addr.net;
        point = 1;
        t++;
    }

    while (t)
    {
        n = (int)strtol(t, &t, 10);

        if (t == NULL)
        {
            if (point)
            {
                tmp.point = n;
            }
            else
            {
                tmp.node = n;
            }
            if (tmp.zone == 0)
            {
                tmp.zone = CurArea.addr.zone;
            }
            return tmp;
        }

        switch (*t)
        {
        case ')':
        case ' ':
        case '\0':
            if (point)
            {
                tmp.point = n;
            }
            else
            {
                tmp.node = n;
            }
            if (tmp.zone == 0)
            {
                tmp.zone = CurArea.addr.zone;
            }
            return tmp;

        case ':':
            tmp.zone = n;
            break;

        case '/':
            tmp.net = n;
            break;

        case '.':
            tmp.node = n;
            point = 1;
            break;

        case '@':
            if (point)
            {
                tmp.point = n;
            }
            else
            {
                tmp.node = n;
            }

            release(tmp.domain);

            s = t + 1;
            while (*s && !isspace(*s) && *s != ')')
            {
                s++;
            }

            if (*s)
            {
                ch = *s;
                *s = '\0';
            }
            else
            {
                ch = 0;
            }

            tmp.domain = xstrdup(t + 1);

            if (ch)
            {
                *s = ch;
            }

            if (tmp.zone == 0)
            {
                tmp.zone = CurArea.addr.zone;
            }
            return tmp;
        }
        t++;
    }

    if (tmp.zone == 0)
    {
        tmp.zone = CurArea.addr.zone;
    }

    return tmp;
}


/* Parses an Internet or Bangpath Address like in one of the following forms
   into domain/bangpath and user name. Recognised formats:

   email@address (User Name)
   email@address
   User Name <email@address>
   "User Name" <email@address>
*/

void parse_internet_address (const char *string, char **cpdomain, char **cpname)
{
    enum { SIMPLESTYLE,  /*  From: user@host */
           NORMALSTYLE,  /*  From: user@host (name) */
           ALTSTYLE      /*  From: name <user@host> */
         };
    int style;
    char *firststring, *bracestring, *worktext, *s, *t, *name, *domain;

    bracestring = NULL;  /* make compiler happy */

    if (string == NULL)
    {
       name = NULL;
       domain = xstrdup("");
    }
    else
    {
        worktext=xstrdup(string);
        s = strrchr(worktext, '(');
        if (s == NULL)
        {
            s=strrchr(worktext, '<');
            if (s==NULL)
            {
                style=SIMPLESTYLE;
            }
            else
            {
                style=ALTSTYLE;
            }
        }
        else
        {
            style=NORMALSTYLE;
        }
        if (s != NULL)
        {
            t = strrchr(s + 1, (*s == '(') ? ')' : '>' );
            *s = '\0';
            if (t != NULL)
            {
                *t = '\0';
            }
            bracestring = xstrdup(s + 1);
        }
        s = worktext;
        while (isspace(*s))
        {
            s++;
        }
        firststring = xstrdup(s);
        switch (style)
        {
            case SIMPLESTYLE: name=NULL;
                              domain=firststring;
                              break;
            case NORMALSTYLE: name=bracestring;
                              domain=firststring;
                              break;
            case ALTSTYLE:    name=firststring;
                              domain=bracestring;
                              break;
            default: abort();
        }
        release(worktext);
    }
    if (name==NULL)
    {
       if (strcmp(ST->uucpgate,"*") == 0)
       {
          name=xstrdup("UUCP");
       }
       else
       {
          name=xstrdup(ST->uucpgate);
       }
    }
    striptwhite(domain);
    strip_geese_feet(name);

    if (cpdomain != NULL)
    {
        *cpdomain = domain;
    }
    else
    {
        release(domain);
    }
    if (cpname != NULL)
    {
        *cpname = name;
    }
    else
    {
        release(name);
    }
}


/* compose_internet_address - counterpart to parse_internet_address */

char *compose_internet_address (const char *domain, const char *name)
{
    char *retval; const char *user;
    char *realdomain, *domainname;

    if (domain == NULL)
    {
        return xstrdup("");
    }

    /* the domain string might contain a user name */
    parse_internet_address(domain, &realdomain, &domainname);

    if (strcmp(domainname, "UUCP") && strcmp(domainname, ST->uucpgate))
    {
      user = domainname;
    }
    else
    {
        if (name == NULL)
        {
            user = "";
        }
        else
        {
            user = name;
        }
    }
    retval = xmalloc(strlen(realdomain) + strlen(user) + 4);

    strcpy(retval, realdomain);
    if (*user)
    {
       strcat(retval, " (");
       strcat(retval, user);
       strcat(retval, ")");
    }
    return retval;
}

