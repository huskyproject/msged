/*
 *  DIRUTE.H
 *
 *  Released to the public domain.
 */

#ifndef __DIRUTE_H__
#define __DIRUTE_H__

/* file info -- any file size (HPFS) */

#ifdef OS2

struct _dta
{
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[255];
};

#define DIR_DIRECT 0x0010
#define DIR_NORMAL 0x0000
#define DIR_ARCHVD 0x0020
#define DIR_READON 0x0001
#define DIR_HIDDEN 0x0002
#define DIR_SYSTEM 0x0004
#define DIR_ICASE  0           /* HPFS is always case insensitive  */
#define DIR_NO_WILDCARDS 0x0

#elif defined(__RSXNT__)

struct _dta
{
    char reserved[21];
    unsigned char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[260];
};

#define DIR_DIRECT 0x00000010
#define DIR_NORMAL 0x00000080
#define DIR_ARCHVD 0x00000020
#define DIR_READON 0x00000001
#define DIR_HIDDEN 0x0          /* ? */
#define DIR_SYSTEM 0x0          /* ? */
#define DIR_ICASE  0x0          /* NT is always case insensitive */
#define DIR_NO_WILDCARDS 0x0

#elif defined(UNIX)

#include <stdio.h>             /* FILENAME_MAX */

struct _dta
{
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[FILENAME_MAX + 1];
};

#define DIR_DIRECT 1
#define DIR_NORMAL 2
#define DIR_ARCHVD
#define DIR_READON 4
#define DIR_HIDDEN
#define DIR_SYSTEM
#define DIR_ICASE  8
#define DIR_NO_WILDCARDS 16

#else

struct _dta
{
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

#if defined(__TURBOC__) || defined(__DJGPP__)

#include <dir.h>

#define DIR_DIRECT FA_DIREC
#define DIR_NORMAL 0
#define DIR_ARCHVD FA_ARCH
#define DIR_READON FA_RDONLY
#define DIR_HIDDEN FA_HIDDEN
#define DIR_SYSTEM FA_SYSTEM
#define DIR_ICASE  0
#define DIR_NO_WILDCARDS 0

#else

#define DIR_DIRECT _A_DIRECT
#define DIR_NORMAL _A_NORMAL
#define DIR_ARCHVD
#define DIR_READON _A_RDONLY
#define DIR_HIDDEN
#define DIR_SYSTEM
#define DIR_ICASE  0
#define DIR_NO_WILDCARDS 0

#endif

#endif

int  dir_findnext(struct _dta *dta);
int  dir_findfirst(char *filename, int attribute, struct _dta *dta);
void dir_findclose(struct _dta *dta);

#ifdef UNIX
void adaptcase(char *filename);
#else
#define adaptcase(x) ((void)(x))
#endif

#define DIRUTE_NONE 1

#endif
