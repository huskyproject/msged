/*
 * Group handling for Msged
 *
 * Written 2000 by Tobias Ernst and released to the Public Domain
 *
 */

#include <time.h>
#include <string.h>

#include "addr.h"
#include "areas.h"
#include "dirute.h"
#include "nedit.h"
#include "msged.h" /* SW */

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
    int i;
    static int lastmatch = 0;

    if (!ngroups)
        return 0;

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

    return 0;
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

void group_setsettings(int handle, int username, int template)
{
    if (handle >= 1 && handle <= ngroups)
    {
        groups[handle - 1].template = template;
        groups[handle - 1].username = username;
    }
}

/* gets the name that the user used to define this group */

char *group_getname(int handle)
{
    if (handle >= 1 && handle <= ngroups)
    {
        return groups[handle - 1].name;
    }
    else
    {
        return "internal boundary error";
    }
}

int *grouparealist = NULL;

void group_build_arealist(void)
{
    int i;
    int groupno = SW->group;
    int lastgroup = -1;
    
    if (grouparealist == NULL)
    {
        grouparealist = malloc((SW->areas + ngroups) * sizeof(int));
    }

    SW->groupareas = 0; SW->grouparea = 0;
    if (groupno)
    {
        for (i = 0; i < SW->areas && arealist[i].group != groupno; i++);
        if (i == SW->areas) /* not a single area in this group */
        {
            groupno = 0;
        }
    }

    for (i = 0; i < SW->areas; i++)
    {
        if (!groupno || arealist[i].group == groupno)
        {
            /* when we insert an area, we see if the group has changed, which
               means that a separator might be needed. */

            if (SW->groupseparators &&
                arealist[i].group != lastgroup &&
                (groupno != 0   ||  /* we display only a single group, so a
                                       separator is allowed as group "title"
                                       header - otherwise ... */
                 ST->sort_criteria[0] == 'g' || /* separators only work when */
                 ST->sort_criteria[0] == 'G')   /* alist is sorted by group */
                )
            {
                lastgroup = arealist[i].group;
                grouparealist[SW->groupareas] = -lastgroup;
                SW->groupareas++;
            }

            /* now we actually insert the area. */
            
            grouparealist[SW->groupareas] = i;
            if (i == SW->area)
            {
                SW->grouparea = SW->groupareas;
            }
            SW->groupareas++;
        }
    }
    while (grouparealist[SW->grouparea] < 0)
    {
        SW->grouparea++;
    }
    SW->area = grouparealist[SW->grouparea];
}

void group_destroy_arealist(void)
{
    if (grouparealist != NULL)
    {
        xfree(grouparealist);
        grouparealist = NULL;
    }
}
    
    
int group_set_group(int group)
{
    int lastgroup = SW->group;
    
    if (group <= ngroups)
    {
        SW->group = group;
        group_build_arealist();
    }
    return lastgroup;
}
