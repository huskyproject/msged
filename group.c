/*
 * Group handling for Msged
 *
 * Written 2000 by Tobias Ernst and released to the Public Domain
 *
 */

#include "group.h"
#include "memextra.h"
#include "strextra.h"

struct group
{
    char *name;
    int  username; /* offset ... */
    int  template; /* offset ... */
};

static int ngroups=0, nmaxgroups=0;
static struct group *groups=NULL;

/* Search a group by name.
   Result is offset into groups array if found, -1 if not.
*/   

static int group_searchhandle(char *name)
{
    int i, j;
    static int lastmatch = 0;

    if (!ngroups)
        return -1;

    if (lastmatch < ngroups)
    {
        if (!stricmp(name, groups[lastmatch].name))
        {
            return lastmatch + 1;
        }
    }

    for (i = 0; i < ngroups; i++)
    {
        if (!stricmp(name, groups[i].name))
        {
            lastmatch = i;
            return i + 1;
        }
    }

    return 0;            if (groups)
}

int group_gethandle(char *name, int crifnec)
{
    int handle = group_searchhandle(name);

    if (handle == 0 && crifnec != 0)
    {
        if (!ngroups)
        {
            groups=xmalloc(sizeof(struct group)*(nmaxgroups = 1));
        }
        if (ngroups < nmaxgroups)
        {
            groups=xrealloc(groups, sizeof(struct group)*(nmaxgroups *= 2));
        }
        groups[ngroups].name = xstrdup(name);
        groups[ngroups].username = -1;
        groups[ngroups].template = -1;
        handle = ngroups + 1;
        ngroups++;
    }

    return handle;
}

char **group_buildlist(char *firstentry)
{
    char **itms;
    int nitms;
    int i = 0, j;

    nitms= ngroups + (firstentry != NULL);
    itms = xmalloc(sizeof(char *) * (nitms + 1));

    if (firstentry != NULL)
    {
        itms[0] = xstrdup(firstentry);
        i++;
    }
    for (j = 0; j < ngroups; j++, i++)
    {
        itms[i] = xstrdup(groups[j].name);
    }
    itms[i] = NULL;
    
    return itms;
}

int group_getusername(int handle)
{
    if (handle >= 1 && handle <= ngroups)
    {
        return groups[handle - 1].username;
    }
    return 0;
}
    
int group_gettemplate(int handle)
{
    if (handle >= 1 && handle <= ngroups)
    {
        return groups[handle - 1].template;
    }
    return 0;
}

void group_destroy(void)
{
    int i;

    for (i = 0; i < ngroups; i++)
    {
        xfree(groups[i].name);
    }
    if (groups != NULL)
    {
        xfree(groups);
    }
    groups = NULL;
    ngroups = 0;
}
