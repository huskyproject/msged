/*
 *  VERSION.H
 *
 *  Released to the public domain.
 */

#ifndef __MSGED_VERSION_H__
#define __MSGED_VERSION_H__

#define msged_VER_MAJOR  6
#define msged_VER_MINOR  3
#define msged_VER_PATCH  0
#define msged_VER_BRANCH BRANCH_CURRENT

#ifdef MSDOS
#ifndef __FLAT__
#define PROG "Msged"
#define OSID "DOS/16"
#else
#define PROG "Msged/386"
#define OSID "DOS/386"
#endif
#endif

#ifdef OS2
#define OSID "OS2;OS/2"
#define PROG "Msged/2"
#endif

#ifdef WINNT
#define OSID "W32;WNT;W95"
#define PROG "Msged/NT"
#endif

#ifdef SASC
#define OSID "AMIGA"
#define PROG "Msged/Ami"
#endif

#ifdef UNIX
#ifndef UNAME
#define UNAME "UNX"
#endif
#define PROG "Msged/"UNAME
#define OSID "UNIX;"UNAME
#endif

#ifndef PROG
#define OSID "UNKNOWN;DOS/16"
#define PROG "Msged"
#endif

#ifdef USE_FIDOCONFIG
#include "huskylib/huskyext.h"
#endif

#ifdef USE_FIDOCONFIG
HUSKYEXT
#endif
char * GenVersionStr(const char * programname,
                              unsigned major,
                              unsigned minor,
                              unsigned patchlevel,
                              unsigned branch,
                              const char * cvsdate);

char * versionStr;
char cvs_date[];

#endif // ifndef __MSGED_VERSION_H__
