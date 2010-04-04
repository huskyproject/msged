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
#include <assert.h>
#include <ctype.h>

#define __DIRUTE_C

#if defined(__WATCOMC__) && defined(WINNT)
#include <wtypes.h>
#endif

#include "dirute.h"
#include "strextra.h"
#include "memextra.h"


#if defined(OS216)
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

#error The drive letter routines are not yet implemented for this platform.
#error You might want to copypaste them from the 32-bit OS/2 section.

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

/* drive letter routines for OS/2 32 Bit */

const int drive_letters = 1;

void dir_setdrive(int d)
{
    DosSetDefaultDisk((ULONG)d + 1UL);
}
int dir_getdrive(void)
{
    unsigned long drivenr;
    unsigned long logmap;

    DosQueryCurrentDisk((PULONG)(&drivenr), (PULONG)(&logmap));

    return drivenr - 1;
}
char *dir_getdrivelist(void)
{
    unsigned long drivenr;
    unsigned long logmap;
    char *str = xmalloc(27);
    int i , j;
    unsigned long comp = 1;

    DosQueryCurrentDisk((PULONG)(&drivenr), (PULONG)(&logmap));

    for (i = 0, j = 0; i < 26; i++, comp = comp * 2UL)
    {
        if (logmap & comp)
        {
            str[j++] = (char)('A' + i);
        }
    }
    str[j] = '\0';
    return str;
}



#elif defined(__RSXNT__) || defined(__CYGWIN__) || defined (__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER >= 1200)) || (defined (__WATCOMC__) && defined(__NT__))
#define WIN32_LEAN_AND_MEAN
#define NOGDI
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

/* drive letter routines for RSX compiler on NT 32 Bit */

const int drive_letters = 1;

void dir_setdrive(int d)
{
    char buf[3];
    buf[0] = d + 'A';
    buf[1] = ':';
    buf[2] = '\0';
    SetCurrentDirectory(buf);
}
int dir_getdrive(void)
{
    char buf[FILENAME_MAX + 1];

#if defined (__WATCOMC__) || defined(__CYGWIN__) || defined (__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER >= 1200))
    GetCurrentDirectory(FILENAME_MAX, buf);
#else
    GetCurrentDirectory(buf, FILENAME_MAX);
#endif
    return (toupper(buf[0]) - 'A');
}
char *dir_getdrivelist(void)
{
    unsigned long logmap;
    char *str = xmalloc(27);
    int i , j;
    unsigned long comp = 1;

    logmap = GetLogicalDrives();

    for (i = 0, j = 0; i < 26; i++, comp = comp * 2UL)
    {
        if (logmap & comp)
        {
            str[j++] = 'A' + i;
        }
    }
    str[j] = '\0';
    return str;
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

/* drive letter routines for Borland C */

const int drive_letters = 1;

void dir_setdrive(int d)
{
    setdisk(d);
}
int dir_getdrive(void)
{
    return getdisk();
}
#if defined(WINNT) || defined(__NT__)
#define NOUSER
#include <windows.h>
char *dir_getdrivelist(void)
{
    unsigned long logmap;
    char *str = xmalloc(27);
    int i , j;
    unsigned long comp = 1;

    logmap = GetLogicalDrives();

    for (i = 0, j = 0; i < 26; i++, comp = comp * 2UL)
    {
        if (logmap & comp)
        {
            str[j++] = 'A' + i;
        }
    }
    str[j] = '\0';
    return str;
}
#else
char *dir_getdrivelist(void)
{
    int ct = 0, curd, i;
    char *buf = xmalloc(27);

    curd = getdisk();

    for (i = 0; i < 26; i++)
    {
        setdisk(i);
        if (i == getdisk())
        {
            buf[ct++] = i + 'A';
        }
    }
    buf[ct] = '\0';
    setdisk(curd);

    return buf;
}
#endif

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

#error You must implement the drive letter routines for this platform first!

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

#error You must implement the drive letter routines for this platform first!

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

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>          /* used to differentiate BSD from SYSV  */
#endif


static DIR *dir;
static struct dirent *de;
static int  ff_attribute;
static char firstbit[FILENAME_MAX + 1];
static char lastbit[FILENAME_MAX + 1];
static char fullname[FILENAME_MAX + 1];

static int match(const char *name, const char *pattern, int attribute,
                 int mode, int rdonly)
{
    char *matpattern;
    char *matname;
    char *cp;
    int rc;
    if (!(attribute & DIR_NO_WILDCARDS))
    {
        matpattern = strdup(pattern);
        matname = strdup(name);

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
    }
    else
    {
        rc = !stricmp(pattern, name);
    }

    if (rc)
    {
        /* the name matches, now check the other criteria */

        if (!(attribute & DIR_READON)) /* read only files not allowed */
        {
            if ((!S_ISDIR(mode)) && (rdonly))
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
    if (!*firstbit)
    {
        dir = opendir("/");
    }
    else
    {
        dir = opendir(firstbit);
    }
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
            dir = NULL;
            return -1;
        }

        strcpy(fullname, firstbit);
        strcat(fullname, "/");
        strcat(fullname, de->d_name);

        if (stat(fullname, &sb))
        {
            closedir(dir);
            dir = NULL;
            return -1;
        }

        if (match(de->d_name, lastbit,
                  attribute, sb.st_mode, access(fullname, W_OK)))
        {
            fin = 1;
        }
    }
    strcpy(dta->name, de->d_name);
    if (S_ISDIR(sb.st_mode))
    {
        dta->attrib = DIR_DIRECT;
    }
    else if (access(fullname, W_OK))
    {
        dta->attrib = DIR_READON;
    }
    else
    {
        dta->attrib = DIR_NORMAL;
    }
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
            dir = NULL;
            return -1;
        }
        strcpy(fullname, firstbit);
        strcat(fullname, "/");
        strcat(fullname, de->d_name);

        if (stat(fullname, &sb))
        {
            closedir(dir);
            dir = NULL;
            return -1;
        }

        if (match(de->d_name, lastbit, ff_attribute,
                  sb.st_mode, access(fullname, W_OK)))
        {
            fin = 1;
        }
    }
    strcpy(dta->name, de->d_name);
    if (S_ISDIR(sb.st_mode))
    {
        dta->attrib = DIR_DIRECT;
    }
    else if (access(fullname, W_OK))
    {
        dta->attrib = DIR_READON;
    }
    else
    {
        dta->attrib = DIR_NORMAL;
    }
    dta->size = sb.st_size;
    return 0;
}

void dir_findclose(struct _dta *dta)
{
    if (dir != NULL)
    {
        closedir(dir);
        dir = NULL;
    }
}

/* UNIX does not have drive letters */

const int drive_letters = 0;

void dir_setdrive(int d)
{
}
int dir_getdrive(void)
{
    return 2;
}
char *dir_getdrivelist(void)
{
    return xstrdup("C");
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

/* Drive letter routines for Microsoft C */
/* I have no idea if these ones work. I copied them from Borland. */

const int drive_letters = 1;

void dir_setdrive(int d)
{
    int e = d;
    _dos_setdrive(d, &e);
}
int dir_getdrive(void)
{
    int d;
    _dos_getdrive(&d);
    return d;
}
#if defined(WINNT) || defined(__NT__)
#define NOUSER
#include <windows.h>
char *dir_getdrivelist(void)
{
    ungsigned long logmap;
    char *str = xmalloc(27);
    int i , j;
    unsigned long comp = 1;

    logmap = GetLogicalDrives();

    for (i = 0, j = 0; i < 26; i++, comp = comp * 2UL)
    {
        if (logmap & comp)
        {
            str[j++] = 'A' + i;
        }
    }
    str[j] = '\0';
    return str;
}
#else
char *dir_getdrivelist(void)
{
    int ct = 0, curd, i;
    char *buf = xmalloc(27);

    curd = dir_getdrive();

    for (i = 0; i < 26; i++)
    {
        dir_setdrive(i);
        if (i == dir_getdrive())
        {
            buf[ct++] = i + 'A';
        }
    }
    buf[ct] = '\0';
    dir_setdrive(curd);

    return buf;
}
#endif

#endif



#ifdef UNIX

/* The adaptcase routine behaves as follows: It assumes that pathname
   is a path name which may contain multiple dashes, and it assumes
   that you run on a case sensitive file system but want / must to
   match the path name insensitively. adaptcase takes every path
   element out of pathname and uses findfirst to check if it
   exists. If it exists, the path element is replaced by the exact
   spelling as used by the file system. If it does not exist, it is
   converted to lowercase.  This allows you to make you program deal
   with things like mounts of DOS file systems under unix

   Return value is 1 if the file exists and 0 if not.

   Attention: Do not ever try to understand this code. I had to do
   heavy caching and other optimizations in this routine in order to
   reduce that startup time of msged to a reasonable value. (The
   problem is that opendir / readdir is very slow ...). If you ever
   have to fix something in this routine, you'd better rewrite it from
   scratch.
*/

/* the cache will take  about 30 * 4192 + 30 * 512 * 4 bytes in this
   configuration, i.e. 180K */

#define adaptcase_cachesize 30
#define rawcache_stepsize 4192
#define cacheindex_stepsize 512

struct adaptcase_cache_entry
{
    char *query;
    char *result;
    char *raw_cache;
    size_t *cache_index;
    size_t n;
};
static int adaptcase_cache_position = -1;
static struct adaptcase_cache_entry adaptcase_cache[adaptcase_cachesize];

static char *current_cache;
static int cache_sort_cmp(const void *a, const void *b)
{
    return stricmp(current_cache+(*((const size_t *)a)),
                   current_cache+(*((const size_t *)b)));
}

static int cache_find_cmp(const void *a, const void *b)
{
    return stricmp((const char *)a, current_cache+(*((const size_t *)b)));
}

/* #define TRACECACHE */

#ifdef BSD
#define DIRENTLEN(x) ((x)->d_namlen)
#else
#define DIRENTLEN(x) (strlen((x)->d_name))
#endif

void adaptcase(char *pathname)
{
    int i,j,k,l,n,nmax, found=1, addresult=0;
    size_t *m; size_t raw_high, rawmax;
    char buf[FILENAME_MAX + 1];
    DIR *dirp = NULL;
    struct dirent *dp;
    char c;

#ifdef TRACECACHE
    FILE *ftrc;
#endif

    if (!*pathname)
        return;
#ifdef TRACECACHE
    ftrc = fopen ("trace.log", "a");
    fprintf(ftrc, "--Query: %s\n", pathname);
#endif

    if (adaptcase_cache_position == -1)
    {
        /* initialise the cache */
        memset(adaptcase_cache, 0, adaptcase_cachesize *
               sizeof(struct adaptcase_cache_entry));
        adaptcase_cache_position = 0;
    }

    k = strlen(pathname);
    if (k > 2)
    {
        for (k = k - 2; k>0 && pathname[k] != '/'; k--);
    }
    else
    {
        k = 0;
    }

    j = 0; i = 0;


start_over:

    if (k != 0)
    {
        l = adaptcase_cache_position;
        do
        {
            if (adaptcase_cache[l].query != NULL)
            {
                if ((!memcmp(adaptcase_cache[l].query,pathname,k)) &&
                    (adaptcase_cache[l].query[k] == '\0'))
                {
                    /* cache hit for the directory */
#ifdef TRACECACHE
                    fprintf (ftrc, "Cache hit for Dir: %s\n",
                             adaptcase_cache[l].result);
#endif
                    memcpy(buf, adaptcase_cache[l].result, k);
                    buf[k] = '/';
                    current_cache=adaptcase_cache[l].raw_cache;
                    m = bsearch(pathname + k + 1,
                                adaptcase_cache[l].cache_index,
                                adaptcase_cache[l].n,
                                sizeof(size_t),
                                cache_find_cmp);
                    if (m == 0)
                    {
#ifdef TRACECACHE
                        fprintf (ftrc, "Cache miss for file.\n");
#endif

                        /* file does not exist - convert to lower c. */
                        for (n = k + 1; pathname[n-1]; n++)
                        {
                            buf[n] = tolower(pathname[n]);
                        }
                        memcpy(pathname, buf, n-1);
#ifdef TRACECACHE
                        fprintf(ftrc, "Return: %s\n", pathname);
                        fclose(ftrc);
#endif
                        return;
                    }
                    else
                    {
#ifdef TRACECACHE
                        fprintf (ftrc, "Cache hit for file: %s\n",
                                 adaptcase_cache[l].raw_cache+(*m));
#endif

                        /* file does exist = cache hit for the file */
                        for (n = k + 1; pathname[n-1]; n++)
                        {
                            buf[n] =
                                adaptcase_cache[l].raw_cache[(*m) + n - k - 1];
                        }
                        assert(buf[n-1] == '\0');
                        memcpy(pathname, buf, n-1);
#ifdef TRACECACHE
                        fprintf(ftrc, "Return: %s\n", pathname);
                        fclose(ftrc);
#endif
                        return;
                    }
                }
            }
            l = (l == 0) ? adaptcase_cachesize - 1 : l - 1;
        } while (l != adaptcase_cache_position);

#ifdef TRACECACHE
        fprintf (ftrc, "Cache miss for directory.\n");
#endif


        /* no hit for the directory */
        addresult = 1;
    }


    while (pathname[i])
    {
        if (pathname[i] == '/')
        {
            buf[i] = pathname[i];
            if (addresult && i == k)
            {
                goto add_to_cache;
            }
cache_failure:
            i++;
            buf[i]='\0';
            dirp = opendir(buf);
#ifdef TRACECACHE
            if (dirp == NULL)
            {
                fprintf (ftrc, "Error opening directory %s\n", buf);
            }
#endif
        }
        else
        {
            assert(i==0);
            dirp = opendir("./");
#ifdef TRACECACHE
            if (dirp == NULL)
            {
                fprintf (ftrc, "Error opening directory ./\n");
            }
#endif
        }

        j = i;
        for (; pathname[i] && pathname[i]!='/'; i++)
            buf[i] = pathname[i];
        buf[i] = '\0';
        found = 0;

        if (dirp != NULL)
        {
            while ((dp = readdir(dirp)) != NULL)
            {
                if (!strcasecmp(dp->d_name, buf + j))
                {
                    /* file exists, take over it's name */

                    assert(i - j == DIRENTLEN(dp));
                    memcpy(buf + j, dp->d_name, DIRENTLEN(dp) + 1);
                    closedir(dirp);
                    dirp = NULL;
                    found = 1;
                    break;
                }
            }
        }
        if (!found)
        {
            /* file does not exist - so the rest is brand new and
               should be converted to lower case */

            for (i = j; pathname[i]; i++)
                buf[i] = tolower(pathname[i]);
            buf[i] = '\0';
            if (dirp != NULL)
            {
                closedir(dirp);
            }
            dirp = NULL;
            break;
        }
    }
    assert(strlen(pathname) == strlen(buf));

add_to_cache:
    while (addresult)
    {
        l = adaptcase_cache_position;
        l = (l == adaptcase_cachesize - 1) ? 0 : l + 1;

        if (adaptcase_cache[l].query != NULL)
        {
            free(adaptcase_cache[l].query);
            adaptcase_cache[l].query = NULL;
        }
        if (adaptcase_cache[l].result != NULL)
        {
            free(adaptcase_cache[l].result);
            adaptcase_cache[l].result = NULL;
        }
        if (adaptcase_cache[l].raw_cache != NULL)
        {
            free(adaptcase_cache[l].raw_cache);
            adaptcase_cache[l].raw_cache = NULL;
        }
        if ( (adaptcase_cache[l].query = malloc(k + 1)) == NULL ||
             (adaptcase_cache[l].result = malloc(k + 1)) == NULL ||
             (adaptcase_cache[l].raw_cache =  malloc(rawmax = rawcache_stepsize)) == NULL ||
             (adaptcase_cache[l].cache_index = malloc((nmax = cacheindex_stepsize) * sizeof(size_t))) == NULL )
        {
            goto cache_error;
        }

        adaptcase_cache[l].n = 0;
        raw_high = 0;

        c = buf[k]; buf[k] = '\0';
        if ((dirp = opendir(buf)) == NULL)
        {
            buf[k] = c;
            goto cache_error;
        }
        buf[k] = c;

        while ((dp = readdir(dirp)) != NULL)
        {
            if (raw_high + DIRENTLEN(dp) + 1 > rawmax)
            {
                if ((adaptcase_cache[l].raw_cache =
                     realloc(adaptcase_cache[l].raw_cache,
                             rawmax+=rawcache_stepsize)) == NULL)
                {
                    goto cache_error;
                }
            }

            if (adaptcase_cache[l].n == nmax - 1)
            {
                if ((adaptcase_cache[l].cache_index =
                     realloc(adaptcase_cache[l].cache_index,
                             (nmax+=cacheindex_stepsize) *
                             sizeof(size_t))) == NULL)
                {
                    goto cache_error;
                }
            }

            memcpy (adaptcase_cache[l].raw_cache + raw_high,
                    dp->d_name, DIRENTLEN(dp) + 1);
            adaptcase_cache[l].cache_index[adaptcase_cache[l].n++] = raw_high;
            raw_high += DIRENTLEN(dp) + 1;
        }
        closedir(dirp);
        current_cache=adaptcase_cache[l].raw_cache;
        qsort(adaptcase_cache[l].cache_index, adaptcase_cache[l].n,
              sizeof(size_t), cache_sort_cmp);

        memcpy(adaptcase_cache[l].query, pathname, k);
        adaptcase_cache[l].query[k] = '\0';
        memcpy(adaptcase_cache[l].result, buf, k);
        adaptcase_cache[l].result[k] = '\0';

        adaptcase_cache_position = l;

#ifdef TRACECACHE
        fprintf  (ftrc, "Sucessfully added cache entry.\n");
#endif
        goto start_over;

    cache_error:
        if (adaptcase_cache[l].query != NULL)
        {
            free(adaptcase_cache[l].query);
            adaptcase_cache[l].query = NULL;
        }
        if (adaptcase_cache[l].result != NULL)
        {
            free(adaptcase_cache[l].result);
            adaptcase_cache[l].result = NULL;
        }
        if (adaptcase_cache[l].raw_cache != NULL)
        {
            free(adaptcase_cache[l].raw_cache);
            adaptcase_cache[l].raw_cache = NULL;
        }
        if (adaptcase_cache[l].cache_index != NULL)
        {
            free(adaptcase_cache[l].cache_index);
            adaptcase_cache[l].cache_index = NULL;
        }
        if (dirp != NULL)
        {
            closedir(dirp);
        }
#ifdef TRACECACHE
        fprintf  (ftrc, "Error in building cache entry.\n");
#endif
        addresult = 0;
        goto cache_failure;
    }

#ifdef TRACECACHE
    fprintf(ftrc, "Return: %s\n", pathname);
    fclose(ftrc);
#endif
    strcpy(pathname, buf);
    return;
}
#endif

#if defined (TEST)
int main(int argc, char **argv)
{
        if (argc < 2)
                return 0;

        adaptcase(argv[1]);
        printf ("%s\n", argv[1]);

        return 0;
}

#endif


