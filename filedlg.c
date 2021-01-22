/*
 * FILEDLG.C
 *
 * File selection popup dialog box routines. Released to the public domain.
 * Attention: Routines are not reentrant.
 *
 * Originally written on Dec. 1997 by Frank Adam
 *
 * Improvements Oct. 1998 and Nov. 1998 by Frank Adam
 *  - Added file sorting
 *  - Added filtering
 *  - Added dynamic allocation
 *  - Changes wrt porting
 *
 * Modifications Apr. 1999 by Tobias Ernst
 * - Moved code to it's own module
 * - Changed dialog box layout
 * - Better support for long file names
 * - Made all colors configurable
 * - Users can now enter a full path in the "file/mask" entry box. If it
 *   is a file name, it will be used. If it is a path with a mask following,
 *   we will change to that path and then use the mask.
 * - Support the "outfile" keyword as default message box content.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef __TURBOC__
#include <dir.h>
#endif

#ifdef __DJGPP__
#include <unistd.h>
#endif

#ifdef __CYGWIN__
#include <unistd.h>
#endif

#ifdef UNIX
#include <unistd.h>
#endif

#ifdef __MINGW32__
#define chdir _chdir
#endif

#ifdef __IBMC__
#define chdir _chdir
#define getcwd _getcwd
#endif

#ifdef __WATCOMC__
#include <sys/types.h>
#include <direct.h>
#endif

#include "addr.h"
#include "config.h"
#include "nedit.h"
#include "msged.h"
#include "winsys.h"
#include "menu.h"
#include "dialogs.h"
#include "memextra.h"
#include "strextra.h"
#include "main.h"
#include "help.h"
#include "keys.h"
#include "dirute.h"
#include "mprotos.h"
#include "unused.h"
#include "mctype.h"
/* #include <smapi/msgapi.h>
   commented out on 2004-08-24 by tobi
   seems to be not needed, but causes compilation problems under cygwin
   if nobody needs this within the next few months, these lines
   should be deleted altogether*/
struct UFILES
{
    int    attrib;
    char * name;
};

struct CURFILE
{
    int  attrib;
    char name[FILENAME_MAX + 1];
};

#define DRVATTR -1
#define MAXDRIVES 26

static char avdrives[MAXDRIVES + 1];
static char FileFilter[FILENAME_MAX + 1];
static struct CURFILE curfile;
static struct UFILES ** files;
static int fmax = 0;             /* maximum files allocated for at any time */
static WND * IEDhCurr, * IEDhWnd;  /* pointers to current and previous windows */
/* dialog box geometry info */
const int fdp_masklength = 29, fdp_maskx = 9, fdp_masky = 1, fdp_listx = 9, fdp_listy = 4,
          fdp_listx2 = 29, fdp_listy2 = 19, fdp_xsize = 40, fdp_ysize = 21, fdp_curdirlength = 29,
          fdp_curdirx = 9, fdp_curdiry = 2, fdp_listlabelx = 1, fdp_listlabely = 4,
          fdp_masklabelx = 1,
          fdp_masklabely = 1, fdp_curdirlabelx = 1, fdp_curdirlabely = 2;
/*
 * Allocates the file array.
 * To be called with 0 for initial, or fmax + X for subsequent reallocs
 */
static int AllocFiles(size_t request)
{
    int i;
    struct UFILES ** tmp = files;

    if(!request)
    {
        files = xmalloc(sizeof(*files) * 100);

        if(NULL != files)
        {
            for(i = 0; i < 100; i++)
            {
                files[i]       = xmalloc(sizeof(struct UFILES));
                files[i]->name = NULL;

                if(NULL == files[i])
                {
                    break;
                }
            }
            fmax = i - 1;
        }
    }
    else
    {
        files = xrealloc(files, sizeof(*files) * (fmax + request));

        if(NULL == files)
        {
            files = tmp; /* No more mem  */
        }
        else
        {
            for(i = fmax; i < request + fmax; i++)
            {
                files[i]       = xmalloc(sizeof(struct UFILES));
                files[i]->name = NULL;

                if(NULL == files[i])
                {
                    break;
                }
            }
            fmax = i - 1;
        }
    }

    return fmax;
} /* AllocFiles */

/* Called on exit */
/* redundant "if"s for debugging, can be removed when working OK */
static void KillFiles(void)
{
    int i;

    if(files != NULL)
    {
        for(i = 0; i < fmax; i++)
        {
            if(files[i] != NULL)
            {
                if(files[i]->name != NULL)
                {
                    xfree(files[i]->name);
                }

                xfree(files[i]);
            }
        }
    }
}

/*
 * Sort the files..
 * This gets a bit complicated, we want the drives at top, that's done
 * outside the sort, the directories straight after, that's done with the
 * attributes, and the files on the bottom. BUT, numeric filenames will
 * precede upper cases, hence the rather overdone comparison below.
 */
static int FileSort(const void * a, const void * b)
{
    struct UFILES * a1 = *((struct UFILES **)a);
    struct UFILES * b1 = *((struct UFILES **)b);

    if(a1->attrib & DIR_DIRECT || b1->attrib & DIR_DIRECT)
    {
        if(a1->attrib & DIR_DIRECT && !(b1->attrib & DIR_DIRECT))
        {
            return -1;
        }
        else if(b1->attrib & DIR_DIRECT && !(a1->attrib & DIR_DIRECT))
        {
            return 1;
        }

        /* else return strcmp(a1->name,b1->name);  */
    }

    if(m_isdigit(*a1->name) && !m_isdigit(*b1->name))
    {
        return 1;
    }
    else if(m_isdigit(*b1->name) && !m_isdigit(*a1->name))
    {
        return -1;
    }

    return stricmp(a1->name, b1->name);
} /* FileSort */

/* Allocate and add a dirname, return true/false */
static int AddDir(int fcount, const char * dirname)
{
    size_t l = strlen(dirname);

    files[fcount]->name = xmalloc(l + 3);

    if(NULL == files[fcount]->name)
    {
        return 0;
    }

    strcpy(files[fcount]->name, "[");
    strcpy(files[fcount]->name + 1, dirname);
    strcat(files[fcount]->name + 1 + l, "]");
    files[fcount]->attrib = DIR_DIRECT;
    return 1;
}

/* Gets the directories...truely :-) */
static int GetDirs(char * curdir, int start)
{
    int count = start, done;
    char sdir[FILENAME_MAX + 1];
    struct _dta f;

    strcpy(sdir, curdir);

#ifdef MSDOS
    strcat(sdir, "*.*");
#else
    strcat(sdir, "*");
#endif

    done = dir_findfirst(sdir, DIR_DIRECT, &f);

    while(!done)
    {
        if(f.attrib & DIR_DIRECT && *f.name != '.')
        {
            if(!AddDir(count, f.name))
            {
                break;
            }

            files[count]->attrib = f.attrib;
            count++;
        }

        if(count >= fmax) /* out of space ? Try to alloc more */
        {
            if(!AllocFiles(50))
            {
                break;
            }

            if(count >= fmax)
            {
                break;               /* test again to be sure ? */
            }
        }

        done = dir_findnext(&f);
    }
    return count;
} /* GetDirs */

/*
 * Shows the current file filter, called when apropriate.
 * Otherwise poor senile user just won't remember it. I didn't. :-)
 */
static void ShowFileFilter(void)
{
    WndFillField(fdp_maskx, fdp_masky, fdp_masklength + 1, ' ', cm[DL_ENRM]);
    WndPrintf(fdp_maskx, fdp_masky, cm[DL_ENRM], "%s", FileFilter);
}

/*
 * Show the current directory.
 */
static void ShowCurDir(char * curdir)
{
    char cp[FILENAME_MAX + 1];
    int i, j;
    size_t l;

    l = strlen(curdir);

    if(l > fdp_curdirlength)
    {
        strcpy(cp, "...");
        j = 3;
        i = l - (fdp_curdirlength - 3);
    }
    else
    {
        i = j = 0;
    }

    for( ; curdir[i]; i++, j++)
    {
        if(curdir[i] == '/' && drive_letters)
        {
            cp[j] = '\\';
        }
        else
        {
            cp[j] = curdir[i];
        }
    }
    cp[j] = '\0';
    WndFillField(fdp_curdirx, fdp_curdiry, fdp_curdirlength + 1, ' ', cm[DL_WTXT]);
    WndPrintf(fdp_curdirx, fdp_curdiry, cm[DL_WTXT], "%s", cp);
} /* ShowCurDir */

/*
 * Retrieves and builds the file list.
 * Curdir is passed in with trailing slash.
 */
int GetFiles(char * curdir)
{
    struct _dta f;
    int done, count = 0, drvflag = 0, drvcount = 0;
    char sdir[FILENAME_MAX + 1]; /* MAXPATH */

    strcpy(sdir, curdir);

    if(NULL == FileFilter || !*FileFilter) /* let's not search for nothing. */
    {
#ifdef MSDOS
        strcpy(FileFilter, "*.*");
#else
        strcpy(FileFilter, "*");
#endif
    }

    strcat(sdir, FileFilter);  /* slash to be fixed at call */
    ShowFileFilter();  /* Show the filter here, the search could be Nil */

    /* So this should avoid some confusion. */
    if(strlen(curdir) > ((drive_letters) ? 3 : 1)) /* then must be a sub-dir */
    {
        AddDir(count, "..");
        files[count++]->attrib = DIR_DIRECT;
    }
    else if(drive_letters && curdir[0] && curdir[1] == ':')
    {
        while(avdrives[drvflag] != '\0')
        {
            if(avdrives[drvflag] != toupper(*curdir))  /* ..except this one */
            {
                files[count]->name = xmalloc(sizeof(char) * 6);

                if(NULL == files[count]->name)
                {
                    return -1;
                }

                strcpy(files[count]->name, "");
                /* We want drives up top, see below */
                files[count++]->attrib = DRVATTR;
            }

            drvflag++;
        }
    }

    count = GetDirs(curdir, count);
    /* Retrieve all dirs regardless of filter */
    /* Ok, now get the files */
    done = dir_findfirst(sdir,
                         DIR_DIRECT | DIR_NORMAL | DIR_ARCHVD | DIR_HIDDEN | DIR_SYSTEM | DIR_READON,
                         &f);

    while(!done)
    {
        if(!(f.attrib & DIR_DIRECT))
        {
            files[count]->name = strdup(f.name);

            if(NULL == files[count]->name)
            {
                break;
            }

            files[count]->attrib = f.attrib;
            count++;
        }

        if(count >= fmax) /* out of space ? Try to alloc more */
        {
            if(!AllocFiles(50))
            {
                break;                  /* Oh well, that's all then */
            }

            if(count >= fmax)
            {
                break;
            }

            /* test again in case we just got more pointers
               but no actual file structs. */
        }

        done = dir_findnext(&f);
    }
    /* Sort them, this places all empty strings at top of list(ie: drives) */
    /* Capitals (ie: dirs) will be next, followed by the files */
    qsort((void *)files, count, sizeof(files[0]), FileSort);

    if(drive_letters && drvflag) /* Now we can add the drives */
    {
        drvflag = drvcount = 0;

        /* NOTE: drvflag is now used for something else */
        while(avdrives[drvflag] != '\0')
        {
            if(avdrives[drvflag] != toupper(*curdir))  /* ..except this one */
            {
                strcpy(files[drvcount]->name, "< :\\>");
                files[drvcount]->name[1]  = avdrives[drvflag];
                files[drvcount++]->attrib = DRVATTR;
            }

            drvflag++;
        }
    }

    return count - 1;
} /* GetFiles */

/*
 * TrimEdge
 * Used in showfile to remove leading and trailing edges from dirs and drives
 */
static void TrimEdge(char * retdir, int attrib)
{
    char * p = retdir;
    size_t l = strlen(p);

    if(attrib == DRVATTR || attrib & DIR_DIRECT)
    {
        p[l - 1] = '\0';
        memmove(p, p + 1, l);
    }
}

/*
 * Show the files..
 * This handles most of the user i/o in conjunction with the caller
 * may not be the best method of doing things, but this is how it developed
 * i can certainly live with it.
 */
int ShowFiles(int maxfiles, struct CURFILE * retdir, int * current)
{
    int top = fdp_listy + 1, depth = fdp_listy2 - fdp_listy - 2, key = 0, i;
    int fnlen = fdp_listx2 - fdp_listx - 4;
    int curtop, prnflag = 0, inppos, drop = 0, done;
    int nattr = cm[DL_ENRM], hattr = cm[DL_ESEL];
    char temp[FILENAME_MAX + 1];
    static int cur;
    int directzap = 1; /* directly go to the enter field? */

    /* Init things */
    curtop = *current;

    if(!curtop)
    {
        cur = 0;
    }

    WndPrintf(fdp_listlabelx, fdp_listlabely, cm[DL_WTXT], "Select:");
    WndPrintf(fdp_masklabelx, fdp_masklabely, cm[DL_WTXT], "File:");
    WndPrintf(fdp_curdirlabelx, fdp_curdirlabely, cm[DL_WTXT], "CurDir:");
    /* WndPrintf(6, top-1, cm[DL_WTXT] | _BLINK, ":"); */
    WndClear(fdp_listx + 1, top, fdp_listx2 - 1, top + depth, nattr);

    for(i = curtop; i <= maxfiles && i <= (curtop + depth); i++)
    {
        WndPrintf(fdp_listx + 1, i + top, nattr, " %-*.*s ", fnlen, fnlen, files[i]->name);
    }

    /* Ok, wait for user */
    while(1)
    {
        if(!(directzap && retdir->name[0]))
        {
            WndPrintf(fdp_listx + 1, top + (cur - curtop), hattr, ">%-*.*s<", fnlen, fnlen,
                      files[cur]->name);
            key = GetKey();
            WndPrintf(fdp_listx + 1, top + (cur - curtop), nattr, " %-*.*s ", fnlen, fnlen,
                      files[cur]->name);
            directzap = 0;
        }
        else
        {
            key = Key_Tab;
            strcpy(temp, retdir->name);
        }

        switch(key)
        {
            case Key_F1:

                if(ST->helpfile != NULL)
                {
                    DoHelp(5);
                }

                break;

            case Key_Tab:

                /* This drops us to the file/filter box
                 * User can enter a valid filename or filter.
                 * Esc will take us back to the flist area.
                 * The input is parsed elsewhere
                 */
                if(!directzap)
                {
                    *temp  = '\0';
                    inppos = 0;
                }
                else
                {
                    directzap = 0;
                    inppos    = strlen(temp);
                }

                done = 0;

                /*WndPrintf(6,top-1,cm[DL_WTXT],":");
                   WndPrintf(13,19,cm[DL_WTXT] | _BLINK,":"); */
                while(!done) /* PITA or bug,but getline assumes edit
                                is done if cursor goes passed the end */
                {
                    drop = WndGetLine(fdp_maskx,
                                      fdp_masky,
                                      fdp_masklength,
                                      temp,
                                      nattr,
                                      &inppos,
                                      0,
                                      0,
                                      1,
                                      NULL);

                    switch(drop)
                    {
                        case Key_Esc:
                            drop = 0;
                            done = 1;
                            /*WndPrintf(13,19,cm[DL_WTXT],":");
                               WndPrintf(6,top-1,cm[DL_WTXT] | _BLINK,":"); */
                            break;

                        case Key_Ent:

                            if(!*temp || *temp == '\t')
                            {
                                drop = 0;
                            }

                            done = 1; /* Also traps accidental TAB */
                            break;

                        default:
                            drop = 0;
                    }
                }
                ShowFileFilter(); /* Refresh the current filter display */

                if(!drop)
                {
                    break;
                }

            case Key_Ent:

                if(!drop)
                {
                    strcpy(retdir->name, files[cur]->name);
                    retdir->attrib = files[cur]->attrib;
                    *current       = curtop;
                    TrimEdge(retdir->name, retdir->attrib);
                    return key;
                }
                else
                {
                    drop = 0;
                    strcpy(retdir->name, temp);
                    retdir->attrib = 0;
                    return 13;
                }

            case Key_Esc:
                return 27;

            case Key_Up:

                if(cur > 0)
                {
                    if(cur > curtop)
                    {
                        cur--;
                    }
                    else
                    {
                        cur--;
                        curtop--;
                        prnflag = 1;
                    }
                }

                break;

            case Key_Dwn:

                if(cur < maxfiles)
                {
                    if(cur < curtop + depth)
                    {
                        cur++;
                    }
                    else
                    {
                        cur++;
                        curtop++;
                        prnflag = 1;
                    }
                }

                break;

            case Key_Home:
                curtop  = 0;
                cur     = 0;
                prnflag = 1;
                break;

            case Key_End:
                curtop = maxfiles - depth;

                if(curtop < 0)
                {
                    curtop = 0;
                }

                cur = curtop + depth;

                if(cur > maxfiles)
                {
                    cur = maxfiles;
                }

                prnflag = 1;
                break;

            case Key_PgUp:

                if(curtop == 0 || maxfiles < depth)
                {
                    break;
                }

                if(curtop < depth)
                {
                    curtop = 0;
                    cur    = 0;
                }
                else
                {
                    curtop -= depth;
                    cur     = curtop;
                }

                prnflag = 1;
                break;

            case Key_PgDn:

                if(curtop + depth > maxfiles)
                {
                    break;
                }

                curtop += depth;
                cur     = curtop;
                prnflag = 1;
                break;

            default:

                if(m_isalnum(key))
                {
                    if(cur + depth < maxfiles)
                    {
                        i = cur + 1;
                    }
                    else
                    {
                        i = 0;
                    }

                    for( ; i <= maxfiles; i++)
                    {
                        if(toupper(key) == toupper(*files[i]->name))
                        {
                            curtop  = cur = i;
                            prnflag = 1;
                            break;
                        }
                    }
                }
        } /* end switch */

        if(prnflag) /* This is set if a display refresh is required */
        {
            prnflag = 0;
            WndClear(fdp_listx + 1, top, fdp_listx2 - 1, top + depth, cm[DL_WTXT]);

            for(i = curtop; i <= curtop + depth && i <= maxfiles; i++)
            {
                WndPrintf(fdp_listx + 1,
                          top + (i - curtop),
                          nattr,
                          " %-*.*s ",
                          fnlen,
                          fnlen,
                          files[i]->name);
            }
        }
    } /* end while */
} /* ShowFiles */

/*
 * Retrieves availabale drives
 */
static void GetAvDrives(void)
{
    char * c;
    int i;

    if(drive_letters)
    {
        c = dir_getdrivelist();

        for(i = 0; c && c[i] && i < MAXDRIVES; i++)
        {
            avdrives[i] = (char)toupper((int)(c[i]));
        }
        avdrives[i] = '\0';
        xfree(c);
    }
    else
    {
        avdrives[0] = '\0';
    }
}

/*
 * Normalize path to forward slashes
 */
static void FixPath(char * path)
{
    char * p = path;

    while(*p)
    {
        if(*p == '\\')
        {
            *p = '/';
        }

        p++;
    }
}

/*
 * Add a slash to directory path if necessary
 */
static void AddDirSlash(char * path)
{
    size_t l = strlen(path);

    if(l)
    {
        if(path[l - 1] != '/' && path[l - 1] != '\\')
        {
            path[l]     = '/';
            path[l + 1] = '\0';
        }
    }
}

static void ImpExpDlgInit(const char * title)
{
    IEDhCurr = WndTop();
    IEDhWnd  = WndPopUp(fdp_xsize, fdp_ysize, DBDR | SHADOW, cm[DL_BTXT], cm[DL_WTXT]);
    WndBox(fdp_listx, fdp_listy, fdp_listx2, fdp_listy2, cm[DL_BTXT], SBDR);
    WndCurr(IEDhWnd);
    WndTitle(title, cm[DL_WTXT]);
    WndFillField(fdp_maskx, fdp_masky, fdp_masklength, ' ', cm[DL_ENRM]);
    TTCurSet(0);
}

static void ImpExpDlgDone(void)
{
    WndClose(IEDhWnd);
    WndCurr(IEDhCurr);
    TTCurSet(1);
}

/*
 * A getcwd routine that always adds the drive letter.
 * Unfortunately, some getcwd implementations include the drive letter,
 * while others don't ...
 */
static char * dlgetcwd(char * storehere, int buflen)
{
    getcwd(storehere, buflen);

    if(drive_letters)
    {
        if(storehere[0] == '\0' || storehere[1] != ':')
        {
            memmove(storehere + 2, storehere, buflen - 3);
            storehere[buflen - 1] = '\0';
            storehere[0]          = (char)(dir_getdrive() + 'A');
            storehere[1]          = ':';
        }
    }

    return storehere;
}

/*
 * Analyse user input if the input was a complete path with filename.
 * Also used for parsing the default settings.
 */
static int process_fileinput(char * retpath, char * curdir)
{
    char * sp;
    int savech;
    int retval = 0;

    FixPath(curfile.name);

    /* split in path and file */
    sp = strrchr(curfile.name, '/');

    if(sp != NULL)
    {
        /* user entered a path. */
        if(sp == curfile.name || (drive_letters && *(sp - 1) == ':'))
        {
            /* special treatment for root */
            savech = sp[1];
            sp[1]  = '\0';
        }
        else
        {
            savech = -1;
            *sp    = '\0';
        }

        if(!chdir(curfile.name))
        {
            /* change dir succeeded -
               change drive letters as well */
            if(drive_letters)
            {
                if(curfile.name[1] == ':')
                {
                    dir_setdrive(toupper(*curfile.name) - 'A');
                }
            }

            /* the path existed. good. use it. */
            dlgetcwd(curdir, FILENAME_MAX);
            AddDirSlash(curdir);
            sp++;

            if(savech != -1)
            {
                *sp = (char)savech;
            }
        }
        else
        {
            sp = NULL;
            fprintf(stderr, "\a");
            fflush(stderr);
            curfile.name[0] = '\0';
            /* path does not exist. beep and do nothing */
        }
    }
    else
    {
        sp = curfile.name;
    }

    /* Now we have evaluted the path component in the user
       entry. Now let's see about the rest. It could be a file
       mask, a subdirectory name, or a file name. */
    if(sp)
    {
        if(strchr(sp, '?') || strchr(sp, '*'))
        {
            /* it is a file mask */
            strcpy(FileFilter, sp);
            curfile.name[0] = '\0';
        }
        else if(!chdir(sp))
        {
            /* it is another subdirectory */
            dlgetcwd(curdir, FILENAME_MAX);
            AddDirSlash(curdir);
            curfile.name[0] = '\0';
        }
        else
        {
            /* it is a file name that the user wishes to use */
            retval = 1;
            strcpy(retpath, curdir);
            strcat(retpath, sp);

            if(sp != curfile.name)
            {
                memmove(curfile.name, sp, strlen(sp) + 1);
            }
        }
    }
    else
    {
        curfile.name[0] = '\0';
    }

    return retval;
} /* process_fileinput */

/*
 * The main routine
 */
int FileDialog(char * retpath, const char * title)
{
    int key = 0, max, cur = 0, retval = 0;
    char curdir[FILENAME_MAX + 1];
    char homedir[FILENAME_MAX + 1], * sp;
    int homedisk = (drive_letters) ? dir_getdrive() : 0;

    if(AllocFiles(0) <= 0)      /* init the file array */
    {
        return -1;
    }

    GetAvDrives();
    dlgetcwd(homedir, FILENAME_MAX); /* save home location */

    strcpy(curdir, homedir);

    if(!(*retpath))
    {
        curfile.name[0] = '\0';
    }
    else
    {
        /* in case retpath does not include a path */
        strcpy(curfile.name, retpath);
        process_fileinput(retpath, curdir);
    }

    AddDirSlash(curdir);
    ImpExpDlgInit(title);
    max = GetFiles(curdir);

    while(key != 27)
    {
        FixPath(curdir);
        ShowCurDir(curdir);
        key = ShowFiles(max, &curfile, &cur); /* Go get user input */

        if(key == Key_Ent)
        {
            if(curfile.attrib == DRVATTR)  /* User asking for drive */
            {
                dir_setdrive(toupper(*curfile.name) - 'A');
                chdir("/");
                sprintf(curdir, "%c:/", *curfile.name);
                curfile.name[0] = '\0';
            }
            else if(curfile.attrib & DIR_DIRECT)   /* User asking for dir */
            {
                if(*curfile.name != '.') /* build a new path */
                {
                    dlgetcwd(curdir, FILENAME_MAX);
                    AddDirSlash(curdir);
                    strcat(curdir, curfile.name); /* Add new dirname.. */
                    strcat(curdir, "/");    /* ..and a trailer */
                    chdir(curdir);
                    dlgetcwd(curdir, FILENAME_MAX);
                    AddDirSlash(curdir);
                    curfile.name[0] = '\0';
                }
                else  /* ".."  bit easier */
                {
                    sp = strrchr(curdir, '/');

                    if(sp != NULL)
                    {
                        *sp = '\0';
                        sp  = strrchr(curdir, '/');

                        if(sp != NULL)
                        {
                            if(sp == curdir)
                            {
                                sp[1] = '\0';
                            }
                            else
                            {
                                *sp = '\0';
                            }

                            chdir(curdir);
                            dlgetcwd(curdir, FILENAME_MAX);
                            AddDirSlash(curdir);
                            curfile.name[0] = '\0';
                        }
                    }
                }
            } /* end else if dirattrib */
            /* user entered a file or filter, or selected a file. */
            else if(*curfile.name)
            {
                if(process_fileinput(retpath, curdir))
                {
                    retval = 1; /* it is a valid file */
                    break;
                }
            }

            /* If we get here something changed so re-scan */
            max = GetFiles(curdir);
            cur = 0;
        } /* end if Key_Ent */
        else /* Key must be 27 we're done */
        {
            retval        = 0;
            *retpath      = '\0';
            *curfile.name = '\0';
            break;
        }
    }  /* end while */
    KillFiles();
    ImpExpDlgDone();

    /* Restore startup locations */
    if(drive_letters)
    {
        dir_setdrive(homedisk);
    }

    chdir(homedir);
    return retval;
} /* FileDialog */
