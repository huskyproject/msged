/*
 *  RDIRENT.H - POSIX.1 compliant header
 *
 *  Original Copyright 1988-1991 by Bob Stout as part of the MicroFirm
 *  Function Library (MFL)
 *
 *  This subset version is functionally identical to the version
 *  originally published by the author in Tech Specialist magazine and
 *  is hereby donated to the public domain.
 *
 *  Modified and cleaned up by Andrew Clarke.
 */

#ifndef __RDIRENT_H__
#define __RDIRENT_H__

#ifndef OS2

#if defined(__ZTC__)

#define DSTRUCT    FIND
#define ATTRIBUTE  attribute
#define NAME       name
#define TIME       time
#define DATE       date
#define FSIZE      size
#pragma pack(1)
#include <direct.h>

#elif defined(__TURBOC__)

#define DSTRUCT    ffblk
#define ATTRIBUTE  ff_attrib
#define NAME       ff_name
#define TIME       ff_ftime
#define DATE       ff_fdate
#define FSIZE      ff_fsize
#include <dir.h>

#elif defined(PACIFIC)

struct DSTRUCT
{
    unsigned char reserved[21];
    unsigned char ATTRIBUTE;
    unsigned short TIME;
    unsigned short DATE;
    unsigned long FSIZE;
    char NAME[13];
};

#else

#define DSTRUCT    find_t
#define ATTRIBUTE  attrib
#define NAME       name
#define TIME       time
#define DATE       date
#define FSIZE      size
#pragma pack(1)
#include <direct.h>

#endif

#else

#define INCL_DOSFILEMAN

#include <os2.h>

struct DSTRUCT
{
    BYTE reserved[21];
    BYTE ATTRIBUTE;
    FTIME TIME;
    FDATE DATE;
    ULONG FSIZE;
    CHAR NAME[13];
};

#endif

#define FA_ANY 0xff
#undef FA_DIREC
#define FA_DIREC 0x10

/*
 *  Portable findfirst/findnext functions from RFIND1ST.C.
 */

struct DSTRUCT *rfind_1st(char *, unsigned, struct DSTRUCT *);
struct DSTRUCT *rfind_nxt(struct DSTRUCT *);

typedef struct
{
    int dd_fd;
    unsigned dd_loc, dd_size;
    struct DSTRUCT dd_buf;
    char dd_dirname[FILENAME_MAX];
}
DOS_DIR;

DOS_DIR *opendir(char *);
int closedir(DOS_DIR *), rewinddir(DOS_DIR *);
struct DSTRUCT *readdir(DOS_DIR *), *seekdir(DOS_DIR *, int, int);

#define telldir(dd) dd->loc

/*
 *  Other useful functions from DIRMASK.C and PATMAT.C.
 */

int dirmask(struct DSTRUCT *, char *, char *, unsigned, unsigned);
int patmat(const char *, const char *);

extern int DFerr;

extern DOS_DIR _DIRS[];

#endif
