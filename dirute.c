/*
 *  DIRUTE.C
 *
 *  Written on 30-Jul-90 by jim nutt and released to the public domain.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "dirute.h"

#if defined(OS216)
#include <ctype.h>
#include <time.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define INCL_DOSPROCESS
#include <os2.h>

union
{
    FDATE a;
    unsigned int b;
}
dc;

union
{
    FTIME a;
    unsigned int b;
}
tc;

static struct _FILEFINDBUF InfoBuf;

HDIR hDir;
USHORT cSearch;
USHORT usAttrib;

#define FILENAMELEN 255

int dir_findfirst(char *filename, int attribute, struct _dta *dta)
{
    hDir = 0x0001;
    usAttrib = attribute;
    cSearch = 1;

    if (DosFindFirst2(filename, &hDir, usAttrib, &InfoBuf, (USHORT) (sizeof(InfoBuf) * cSearch), &cSearch, FIL_STANDARD, (ULONG) NULL) != 0)
    {
        DosFindClose(hDir);
        errno = ENOENT;
        return -1;
    }
    else
    {
        dta->attrib = (char)InfoBuf.attrFile;
        dta->size = InfoBuf.cbFile;
        dc.a = InfoBuf.fdateLastWrite;
        tc.a = InfoBuf.ftimeLastWrite;
        dta->wr_time = tc.b;
        dta->wr_date = dc.b;
        strcpy(dta->name, InfoBuf.achName);
        errno = 0;
        return 0;
    }
}

int dir_findnext(struct _dta *dta)
{
    if (DosFindNext(hDir, &InfoBuf, (USHORT) (FILENAMELEN + 23), &cSearch) || cSearch != 1)
    {
        DosFindClose(hDir);
        errno = ENOENT;
        return -1;
    }
    else
    {
        dta->attrib = (char)InfoBuf.attrFile;
        dta->size = InfoBuf.cbFile;
        dc.a = InfoBuf.fdateLastWrite;
        tc.a = InfoBuf.ftimeLastWrite;
        dta->wr_time = tc.b;
        dta->wr_date = dc.b;
        strcpy(dta->name, InfoBuf.achName);
        errno = 0;
        return 0;
    }
}

#elif defined(OS2)
#define INCL_DOSPROCESS
#include <os2.h>

union
{
    FDATE a;
    unsigned int b;
}
dc;

union
{
    FTIME a;
    unsigned int b;
}
tc;

static FILEFINDBUF3 InfoBuf;

HDIR hDir;
ULONG cSearch;
ULONG usAttrib;

#define FILENAMELEN 255

int dir_findfirst(char *filename, int attribute, struct _dta *dta)
{
    hDir = 0x0001;
    usAttrib = attribute;
    cSearch = 1;

    if (DosFindFirst((unsigned char *)filename, &hDir, usAttrib, &InfoBuf,
                     sizeof InfoBuf, &cSearch, FIL_STANDARD) != 0)
    {
        DosFindClose(hDir);
        errno = DIRUTE_NONE;
        return -1;
    }
    else
    {
        dta->attrib = (char)InfoBuf.attrFile;
        dta->size = InfoBuf.cbFile;
        dc.a = InfoBuf.fdateLastWrite;
        tc.a = InfoBuf.ftimeLastWrite;
        dta->wr_time = tc.b;
        dta->wr_date = dc.b;
        strcpy(dta->name, InfoBuf.achName);
        errno = 0;
        return 0;
    }
}

int dir_findnext(struct _dta *dta)
{
    if (DosFindNext(hDir, &InfoBuf, (USHORT) (FILENAMELEN + 23), &cSearch) || cSearch != 1)
    {
        DosFindClose(hDir);
        errno = DIRUTE_NONE;
        return -1;
    }
    else
    {
        dta->attrib = (char)InfoBuf.attrFile;
        dta->size = InfoBuf.cbFile;
        dc.a = InfoBuf.fdateLastWrite;
        tc.a = InfoBuf.ftimeLastWrite;
        dta->wr_time = tc.b;
        dta->wr_date = dc.b;
        strcpy(dta->name, InfoBuf.achName);
        errno = 0;
        return 0;
    }
}

#elif defined(__RSXNT__)

#define NOUSER
#include <windows.h>

static HANDLE           hDirA = INVALID_HANDLE_VALUE;
static WIN32_FIND_DATA  InfoBuf;
static DWORD            dwFileAttributes;

static int dir_find_common(struct _dta *dta);

int dir_findfirst(char *filename, int attribute, struct _dta *dta)
{
    if (hDirA != INVALID_HANDLE_VALUE)
    {
        FindClose(hDirA);
    }

    dwFileAttributes = attribute;
    
    hDirA = FindFirstFile(filename, &InfoBuf);
    return dir_find_common(dta);
}

int dir_findnext(struct _dta *dta)
{
    if (!FindNextFile(hDirA, &InfoBuf))
    {
        if (hDirA != INVALID_HANDLE_VALUE)
        {
            FindClose(hDirA);
            hDirA = INVALID_HANDLE_VALUE;
        }
    }
    return dir_find_common(dta);
}

static int dir_find_common(struct _dta *dta)
{
    while (hDirA != INVALID_HANDLE_VALUE)
    {
        if (!(InfoBuf.dwFileAttributes & 
              (~(dwFileAttributes | DIR_ARCHVD))))
        {
            if (strlen(InfoBuf.cFileName) < sizeof(dta->name))
            {
                break;
            }
        }
        if (!FindNextFile(hDirA, &InfoBuf))
        {
            if (hDirA != INVALID_HANDLE_VALUE)
            {
                FindClose(hDirA);
                hDirA = INVALID_HANDLE_VALUE;
            }
        }
    }
    if (hDirA != INVALID_HANDLE_VALUE)
    {
        dta->attrib = (unsigned char)(InfoBuf.dwFileAttributes);
        dta->size = (long)(InfoBuf.nFileSizeLow);
        strcpy(dta->name, InfoBuf.cFileName);
        errno = 0;
        return 0;
    }
    else
    {
        errno = DIRUTE_NONE;
        return -1;
    }
}

#elif defined(__TURBOC__)

#include <dos.h>
#include <dir.h>

static struct ffblk ffblk;

int dir_findfirst(char *filename, int attribute, struct _dta *dta)
{
    int done;

    done = findfirst(filename, &ffblk, attribute);

    if (!done)
    {
        strcpy(dta->name, ffblk.ff_name);
        dta->wr_time = ffblk.ff_fdate;
        dta->wr_date = ffblk.ff_ftime;
        dta->size = ffblk.ff_fsize;
        dta->attrib = ffblk.ff_attrib;
        return 0;
    }
    return -1;
}

int dir_findnext(struct _dta *dta)
{
    if (!findnext(&ffblk))
    {
        strcpy(dta->name, ffblk.ff_name);
        dta->wr_time = ffblk.ff_fdate;
        dta->wr_date = ffblk.ff_ftime;
        dta->size = ffblk.ff_fsize;
        dta->attrib = ffblk.ff_attrib;
        return 0;
    }
    return -1;
}

#elif defined(PACIFIC)

#include "rfind1st.h"

static struct DSTRUCT dstr;

int dir_findfirst(char *filename, int attribute, struct _dta *dta)
{
    if (rfind_1st(filename, attribute, &dstr) != NULL)
    {
        strcpy(dta->name, dstr.NAME);
        dta->wr_time = dstr.DATE;
        dta->wr_date = dstr.TIME;
        dta->size = dstr.FSIZE;
        dta->attrib = dstr.ATTRIBUTE;
        return 0;
    }
    return -1;
}

int dir_findnext(struct _dta *dta)
{
    if (rfind_nxt(&dstr) != NULL)
    {
        strcpy(dta->name, dstr.NAME);
        dta->wr_time = dstr.DATE;
        dta->wr_date = dstr.TIME;
        dta->size = dstr.FSIZE;
        dta->attrib = dstr.ATTRIBUTE;
        return 0;
    }
    return -1;
}

#elif defined(SASC)
#include <dos.h>

extern int __msflag;
static struct FileInfoBlock info;
static int error;
static int oldmsf;

int dir_findfirst(char *filename, int attribute, struct _dta *dta)
{
    oldmsf = __msflag;
    __msflag = 1;
    error = dfind(&info, filename, 0);
    __msflag = oldmsf;
    while (error == 0)
    {
        strcpy(dta->name, info.fib_FileName);
        return 0;
    }
    return -1;
}

int dir_findnext(struct _dta *dta)
{
    oldmsf = __msflag;
    __msflag = 1;
    error = dnext(&info);
    __msflag = oldmsf;
    if (error == 0)
    {
        strcpy(dta->name, info.fib_FileName);
        return 0;
    }
    return -1;
}

#elif defined(UNIX)

/*
 *  Support for UNIX goes here - use the POSIX.1 routines:
 *    opendir, scandir, readdir, etc.
 */

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include "patmat.h"

static DIR *dir;
static struct dirent *de;
static int  ff_attribute;
static char firstbit[FILENAME_MAX + 1];
static char lastbit[FILENAME_MAX + 1];
static char fullname[FILENAME_MAX + 1];

static int match(const char *name, const char *pattern, int attribute,
                 int mode, int rdonly)
{
    char *matpattern = strdup(pattern);
    char *matname = strdup(name);
    char *cp;
    int rc;

    if (matpattern == NULL || matname == NULL)
    {
        return 0;
    }

    if (attribute & DIR_ICASE)
    {
        for (cp=matpattern; *cp; cp++)
        {
            *cp = toupper(*cp);
        }
        for (cp=matname; *cp; cp++)
        {
            *cp = toupper(*cp);
        }
    }

    rc = patmat(matname, matpattern);

    free(matname);
    free(matpattern);

    if (rc)
    {
        /* the name matches, now check the other criteria */

        if (!(attribute & DIR_READON)) /* read only files not allowed */
        {
            if (rdonly)
            {
                return 0;
            }
        }

        if (!(attribute & DIR_DIRECT))   /* directories not allowed */
        {
            if (S_ISDIR(mode))
            {
                return 0;
            }
        }

        return 1;
    }

    return 0;

}
    

int dir_findfirst(char *filename, int attribute, struct _dta *dta)
{
    char *p;
    int fin = 0;
    struct stat sb;

    p = strrchr(filename, '/');
    if (p == NULL)
    {
        strcpy(firstbit, ".");
        strcpy(lastbit, filename);
    }
    else
    {
        memcpy(firstbit, filename, p - filename);
        firstbit[p - filename] = '\0';
        strcpy(lastbit, p + 1);
    }
    dir = opendir(firstbit);
    if (dir == NULL)
    {
        return -1;
    }
    while (!fin)
    {
        de = readdir(dir);
        if (de == NULL)
        {
            closedir(dir);
            return -1;
        }

        strcpy(fullname, firstbit);
        strcat(fullname, "/");
        strcat(fullname, de->d_name);

        if (stat(fullname, &sb))
        {
            closedir(dir);
            return -1;
        }
        
        if (match(de->d_name, lastbit,
                  attribute, sb.st_mode, access(fullname, W_OK)))
        {
            fin = 1;
        }
    }
    strcpy(dta->name, de->d_name);
    dta->size = sb.st_size;
    ff_attribute = attribute;
    return 0;
}

int dir_findnext(struct _dta *dta)
{
    int fin = 0;
    struct stat sb;

    while (!fin)
    {
        de = readdir(dir);
        if (de == NULL)
        {
            closedir(dir);
            return -1;
        }
        strcpy(fullname, firstbit);
        strcat(fullname, "/");
        strcat(fullname, de->d_name);

        if (stat(fullname, &sb))
        {
            closedir(dir);
            return -1;
        }
        
        if (match(de->d_name, lastbit, ff_attribute,
                  sb.st_mode, access(fullname, W_OK)))
        {
            fin = 1;
        }
    }
    strcpy(dta->name, de->d_name);
    dta->size = sb.st_size;
    return 0;
}

#else  /* MSC version of these routines */

#include <dos.h>

struct find_t ffblk;

int dir_findfirst(char *filename, int attribute, struct _dta *dta)
{
    int done;

    done = _dos_findfirst(filename, attribute, &ffblk);

    if (!done)
    {
        strcpy(dta->name, ffblk.name);
        dta->wr_time = ffblk.wr_date;
        dta->wr_date = ffblk.wr_time;
        dta->size = ffblk.size;
        dta->attrib = ffblk.attrib;
        return 0;
    }
    return -1;
}

int dir_findnext(struct _dta *dta)
{
    if (!_dos_findnext(&ffblk))
    {
        strcpy(dta->name, ffblk.name);
        dta->wr_time = ffblk.wr_date;
        dta->wr_date = ffblk.wr_time;
        dta->size = ffblk.size;
        dta->attrib = ffblk.attrib;
        return 0;
    }
    return -1;
}

#endif
