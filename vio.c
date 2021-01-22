/*
 *  Written by Matthew Parker and released to the public domain.
 */

#ifdef __DJGPP__

#include <conio.h>
#include <dpmi.h>
#include <sys/farptr.h>

#define mypokew(a, b, c) _farpokew(a, (b) << 1, c)
#define mypeekw(a, b) _farpeekw(a, (b) << 1)

#else

#ifdef __FLAT__
#include <i86.h>
#include "rmi.h"
extern struct rminfo RMINF;
#define MKSEG(a) ((unsigned short *)(a << 4))
#else
#include <dos.h>
#define MKSEG(a) MK_FP(vseg, 0)
#endif

#ifndef MK_FP
#define MK_FP(x, y) ((void *)(((unsigned long)(x) << 16) | (unsigned)(y)))
#endif

#define mypokew(a, b, c) *((a) + (b)) = (c)
#define mypeekw(a, b) *((a) + (b))

#endif /* ifdef __DJGPP__ */

#include "vio.h"

static unsigned short vseg = 0xb000;
static unsigned char vmode = 0;
static unsigned short x    = 0;
static unsigned short y    = 0;
static unsigned char color = 7;
static unsigned short ofs  = 0;
static unsigned char xhite = 8;
struct VIOinfo
{
    unsigned char  level;
    unsigned char  level1;
    unsigned short level2;
    unsigned short flags;
    unsigned char  mode;
    unsigned char  mode1;
    unsigned short colors;
    unsigned short pixcol;
    unsigned short pixrow;
    unsigned short txtcol;
    unsigned short txtrow;
} info =
{
    0, 0, 14, 1, 0, 0, 2, 0, 0, 80, 25
};
unsigned short VIOheight(void)
{
    return xhite;
}

unsigned short VIOopen(void)
{
#ifndef __FLAT__
    struct SREGS s;
#endif
#ifdef __DJGPP__
    __dpmi_regs r;
#else
    union REGS r;
    unsigned short temp;
#endif

#if defined (__DJGPP__)
    r.x.ax = 0x0f00;
    __dpmi_int(0x10, &r);
#elif defined (__FLAT__)
    r.w.ax = 0xf00;
    int386(0x10, &r, &r);
#else
    r.x.ax = 0xf00;
    int86(0x10, &r, &r);
#endif
    vmode = r.h.al;

    if(r.h.al == 7)
    {
        return 0;
    }
    else
    {
        info.txtcol = r.h.ah;
        vseg        = 0xb800;
#if defined (__DJGPP__)
        r.x.ax = 0xfe00;
        r.x.es = vseg;
        r.x.di = 0;
        __dpmi_int(0x10, &r);
        vseg = r.x.es;
#elif defined (__FLAT__)
        RMINF.EAX = 0xfe00;
        RMINF.ES  = vseg;
        temp      = RMINF.EDI;
        RMINF.EDI = 0;
        int86x(0x10);
        RMINF.EDI = temp;
        vseg      = RMINF.ES;
#else
        r.x.ax = 0xfe00;
        s.es   = vseg;
        temp   = r.x.di;
        r.x.di = 0;
        int86x(0x10, &r, &r, &s);
        r.x.di = temp;
        vseg   = s.es;
#endif
#if defined (__DJGPP__)
        r.x.ax = 0x1130;
        r.h.bh = 0x1;
        r.x.dx = 0;
        __dpmi_int(0x10, &r);
#elif defined (__FLAT__)
        r.w.ax = 0x1130;
        r.h.bh = 0x1;
        r.w.dx = 0;
        int386(0x10, &r, &r);
#else
        r.x.ax = 0x1130;
        r.h.bh = 0x1;
        r.x.dx = 0;
        int86(0x10, &r, &r);
#endif

        if(r.h.dl == 0)   /* cga */
        {
            info.txtrow = 25;
            return 0;
        }

        xhite       = r.h.cl;
        info.txtrow = r.h.dl + 1;
    }

    return 0;
} /* VIOopen */

void VIOclose(void)
{}

unsigned short VIOcolumns(void)
{
    return info.txtcol;
}

unsigned short VIOrows(void)
{
    return info.txtrow;
}

unsigned short VIOmode(void)
{
    return vmode;
}

unsigned short VIOwherex(void)
{
    return x;
}

unsigned short VIOwherey(void)
{
    return y;
}

void VIOscrollright(int x1, int y1, int x2, int y2, int count)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif
    int far_right = y1 * info.txtcol + x2;
    int width = x2 - x1;
    int depth = y2 - y1 + 2;
    int i, t;

    while(depth--)
    {
        for(i = 0; i < count; i++)
        {
            for(t = 0; t < width; t++)
            {
                mypokew(vid, far_right + t, mypeekw(vid, far_right + t - 1));
            }
            mypokew(vid, far_right + t, (color << 8) | 0x20);
        }
        far_right += info.txtcol;
    }
}

void VIOscrollleft(int x1, int y1, int x2, int y2, int count)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif
    int far_right = y1 * info.txtcol + x2;
    int width = x2 - x1;
    int depth = y2 - y1 + 2;
    int i, t;

    while(depth--)
    {
        for(i = 0; i < count; i++)
        {
            for(t = 0; t < width; t++)
            {
                mypokew(vid, far_right + t - 1, mypeekw(vid, far_right + t));
            }
            mypokew(vid, far_right + width, (color << 8) | 0x20);
        }
        far_right += info.txtcol;
    }
}

void VIOscrollup(int x1, int y1, int x2, int y2, int count)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif
    int far_right    = y1 * info.txtcol + x1;
    int width        = x2 - x1;
    int depth        = y2 - y1;
    int screen_width = info.txtcol;
    int i;

    while(count--)
    {
        while(depth--)
        {
            for(i = 0; i < width; i++)
            {
                mypokew(vid, far_right + i, mypeekw(vid, far_right + screen_width + i));
            }
            far_right += screen_width;
        }

        for(i = 0; i < width; i++)
        {
            mypokew(vid, far_right + i, (color << 8) | 0x20);
        }
    }
} /* VIOscrollup */

void VIOscrolldown(int x1, int y1, int x2, int y2, int count)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif
    int far_right    = y2 * info.txtcol + x1;
    int width        = x2 - x1;
    int depth        = y2 - y1;
    int screen_width = info.txtcol;
    int i;

    while(count--)
    {
        while(depth--)
        {
            for(i = 0; i < width; i++)
            {
                mypokew(vid, far_right + i, mypeekw(vid, far_right - screen_width + i));
            }
            far_right -= screen_width;
        }

        for(i = width; i > 0; i--)
        {
            mypokew(vid, far_right + i - 1, (color << 8) | 0x20);
        }
    }
} /* VIOscrolldown */

void VIOclear(int x1, int y1, int x2, int y2)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif
    int far_right = y1 * info.txtcol + x1;
    int width     = x2 - x1 + 1;
    int depth     = y2 - y1 + 1;
    int i;

    while(depth--)
    {
        for(i = 0; i < width; i++)
        {
            mypokew(vid, far_right + i, (color << 8) | 0x20);
        }
        far_right += info.txtcol;
    }
}

void VIOputc(const char c)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif

    mypokew(vid, ofs, c + (color << 8));
    ofs++;
    x++;

    if(x >= info.txtcol)
    {
        y++;
        x = x - info.txtcol;
    }
}

void VIOputs(const char * s)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif

    while(*s != '\0')
    {
        mypokew(vid, ofs, *s + (color << 8));
        s++;
        x++;
        ofs++;
    }

    if(x >= info.txtcol)
    {
        y++;
        x = x - info.txtcol;
    }
}

unsigned short VIOgetca(const int x, const int y)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif
    int far_right = y * info.txtcol + x;
    return mypeekw(vid, far_right);
}

void VIOgetra(int x1, int y1, int x2, int y2, unsigned short * b)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif
    int far_right = y1 * info.txtcol + x1;
    int width     = x2 - x1 + 1;
    int depth     = y2 - y1 + 1;
    int i;

    while(depth--)
    {
        for(i = 0; i < width; i++)
        {
            *b = mypeekw(vid, far_right + i);
            b++;
        }
        far_right += info.txtcol;
    }
}

void VIOputr(int x, int y, int w, int h, unsigned short * b)
{
#if defined (__DJGPP__)
    int vid = __dpmi_segment_to_descriptor(vseg);
#else
    unsigned short * vid = MKSEG(vseg);
#endif
    int far_right     = y * info.txtcol + x;
    int screen_length = info.txtcol;
    int i;

    while(h--)
    {
        for(i = 0; i < w; i++)
        {
            mypokew(vid, far_right + i, *b);
            b++;
        }
        far_right += screen_length;
    }
}

void VIOsetfore(const int c)
{
    color = (unsigned char)((c & 0x0f) | (color & 0xf0));
}

void VIOsetback(const int c)
{
    color = (unsigned char)(((c & 0x0f) << 4) | (color & 0x0f));
}

unsigned short VIOgetfore(void)
{
    return (int)(color & 0x0f);
}

unsigned short VIOgetback(void)
{
    return (int)((color & 0xf0) >> 4);
}

void VIOgotoxy(int x1, int y1)
{
    x   = x1;
    y   = y1;
    ofs = y1 * info.txtcol + x1;
}

void VIOupdate(void)
{
#ifdef __DJGPP__
    __dpmi_regs r;
#else
    union REGS r;
#endif
    r.h.ah = 2;
    r.h.bh = 0;
    r.h.dl = x;
    r.h.dh = y;
#if defined (__DJGPP__)
    __dpmi_int(0x10, &r);
#elif defined (__FLAT__)
    int386(0x10, &r, &r);
#else
    int86(0x10, &r, &r);
#endif
}

void VIOcursor(int * x, int * y, int * shape)
{
#ifdef __DJGPP__
    __dpmi_regs r;
#else
    union REGS r;
#endif
    r.h.ah = 3;
    r.h.bh = 0;
#if defined (__DJGPP__)
    __dpmi_int(0x10, &r);
#elif defined (__FLAT__)
    int386(0x10, &r, &r);
#else
    int86(0x10, &r, &r);
#endif
    *x = (unsigned int)r.h.dl;
    *y = (unsigned int)r.h.dh;
#if defined (__FLAT__) && !defined (__DJGPP__)
    *shape = r.w.cx;
#else
    *shape = r.x.cx;
#endif
}

unsigned short VIOsegment(void)
{
    return vseg;
}

void VIOsetSegment(unsigned int s)
{
    vseg = s;
}

void VIOsetRows(int r)
{
    info.txtrow = r;
}

void VIOsetCols(int c)
{
    info.txtcol = c;
}
