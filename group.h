/*
 * Group handling for Msged
 *
 * Written 2000 by Tobias Ernst and released to the Public Domain
 *
 */

#ifndef MSGED_GROUP_H
#define MSGED_GROUP_H

int group_gethandle(char *name, int crifnec);
char **group_buildlist(char *firstentry);
int group_getusername(int handle);
int group_gettemplate(int handle);
char *group_getname(int handle);
void group_setsettings(int handle, int username, int template);
void group_destroy(void);

#define group_getareano(x) (grouparealist[(x)])
extern int *grouparealist;
void group_build_arealist(void);
void group_destroy_arealist(void);
int group_set_group(int group);

#endif
