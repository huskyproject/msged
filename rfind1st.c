/*
 *  RFIND1ST.C - Our own non-compiler specific find first/next calls
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

#include <stdio.h>
#include <stdlib.h>
#include "rdirent.h"

#ifdef PACIFIC
int _doserrno;
int bdos(int func, unsigned reg_dx, unsigned char reg_al);

#endif

#ifndef OS2
/*
 *  rfind_1st; Find first matching file.
 *
 *  Parameters:
 *
 *  name   Drive, path and filename of file to be found. May include
 *         wildcards.
 *  attr   Attribute of file to search for. Attributes are described in
 *         the MS-DOS manual. The search strategy is described under DOS
 *         call 0x4Eh.
 *  dta    Disk transfer area buffer. If NULL, one will be malloc'ed.
 *
 *  Returns: Pointer to a struct DSTRUCT. If error, NULL is returned and
 *  _doserrno is set to the error number.
 */
struct DSTRUCT * rfind_1st(char * name, unsigned attr, struct DSTRUCT * dta)
{
    struct DSTRUCT * my_dta;
    union REGS regs;
    struct SREGS sregs;

    if(dta == NULL)
    {
        my_dta = malloc(sizeof *my_dta);
    }
    else
    {
        my_dta = dta;
    }

    regs.x.dx = (unsigned)my_dta;
    sregs.ds  = FP_SEG(my_dta);
    regs.x.ax = 0x1A00;
    int86x(0x21, &regs, &regs, &sregs);
    regs.x.ax = 0x4E00;
    regs.x.cx = attr;
    sregs.ds  = FP_SEG(name);
    regs.x.dx = (unsigned)name;
    int86x(0x21, &regs, &regs, &sregs);

    if(regs.x.cflag)
    {
        _doserrno = regs.x.ax;

        if(dta == NULL && my_dta != NULL)
        {
            free(my_dta);
        }

        return NULL;
    }

    return my_dta;
} /* rfind_1st */

/*
 *  rfind_nxt(); Find next matching file.
 *
 *  Parameter:
 *  dta     Pointer to DSTRUCT structure to use.
 *
 *  Returns: Pointer to struct DSTRUCT, or NULL if no more matching
 *  files were found.
 */
struct DSTRUCT * rfind_nxt(struct DSTRUCT * dta)
{
    union REGS regs;
    struct SREGS sregs;

    if(dta == NULL)
    {
        return NULL;
    }

    regs.x.dx = (unsigned)dta;
    sregs.ds  = FP_SEG(dta);
    regs.x.ax = 0x1A00;
    int86x(0x21, &regs, &regs, &sregs);
    regs.x.ax = 0x4F00;
    int86(0x21, &regs, &regs);

    if(regs.x.cflag)
    {
        _doserrno = regs.x.ax;
        return NULL;
    }

    return dta;
}

#else  /* ifndef OS2 */

#if OS2 < 2
typedef USHORT UWORD
#else
typedef ULONG UWORD
#endif

    static HDIR hdir_ptr = DSIR_CREATE;
#if OS2 < 2
static FILEFINDBUF flist;
#else
static FILEFINDBUF3 flist;
#endif

static PSZ fname;
static UWORD count = 1;
struct DSTRUCT * rfind_1st(char * name, unsigned attr, struct DSTRUCT * dta)
{
    struct DSTRUCT * my_dta;
    short retval;

    if(dta == NULL)
    {
        my_dta = malloc(sizeof *my_dta);
    }
    else
    {
        my_dta = dta;
    }

    fname = (PSZ)name;

#if OS2 < 2

    if(DosFindFirst(fname, &hdir_ptr, attr, &flist, sizeof(flist), &count, 0L))
#else

    if(DosFindFirst(fname, &hdir_ptr, attr, &flist, sizeof(flist), &count, FIL_STANDARD))
#endif
    {
        return NULL;
    }
    else
    {
        my_dta->ATTRIBUTE = (BYTE)(flist.attrFile & 0xFF);
        my_dta->TIME      = flist.ftimeCreation;
        my_dta->DATE      = flist.fdateCreation;
        my_dta->FSIZE     = flist.cbFile;
        strcpy(my_dta->NAME, flist.achName);
        return my_dta;
    }
} /* rfind_1st */

struct DSTRUCT * rfind_nxt(struct DSTRUCT * dta)
{
    struct DSTRUCT * my_dta;

    if(DosFindNext(hdir_ptr, &flist, sizeof(flist), &count))
    {
        return NULL;
    }
    else
    {
        my_dta->ATTRIBUTE = (BYTE)(flist.attrFile & 0xFF);
        my_dta->TIME      = flist.ftimeCreation;
        my_dta->DATE      = flist.fdateCreation;
        my_dta->FSIZE     = flist.cbFile;
        strcpy(my_dta->NAME, flist.achName);
        return my_dta;
    }
}

#endif /* ifndef OS2 */
