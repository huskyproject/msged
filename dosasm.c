/* Written by Matthew Parker and released to the public domain. */

#if defined (__FLAT__) && !defined (__DJGPP__)

#include <i86.h>
#include "rmi.h"

struct rminfo RMINF;

#else

#ifdef __DJGPP__
#include <dpmi.h>
#else
#include <dos.h>
#endif

#include <conio.h>

#endif

void dospause(void)
{
#if defined (__DJGPP__)
    __dpmi_regs r;
    __dpmi_int(0x28, &r);
#elif defined (__FLAT__)
    int86x(0x28);
#else
    union REGS r;
    int86(0x28, &r, &r);
#endif
}

void dvpause(void)
{
#if defined (__DJGPP__)
    __dpmi_regs r;
#ifdef TOPVIEW
    r.x.ax = 0x101a;
    __dpmi_int(0x15, &r);
#endif
    r.x.ax = 0x1000;
    __dpmi_int(0x15, &r);
#ifdef TOPVIEW
    r.x.ax = 0x1025;
    __dpmi_int(0x15, &r);
#endif
#elif defined (__FLAT__)
#ifdef TOPVIEW
    RMINF.EAX = 0x101a;
    int86x(0x15);
#endif
    RMINF.EAX = 0x1000;
    int86x(0x15);
#ifdef TOPVIEW
    RMINF.EAX = 0x1025;
    int86x(0x15);
#endif
#else
    union REGS r;
#ifdef TOPVIEW
    r.x.ax = 0x101a;
    int86(0x15, &r, &r);
#endif
    r.x.ax = 0x1000;
    int86(0x15, &r, &r);
#ifdef TOPVIEW
    r.x.ax = 0x1025;
    int86(0x15, &r, &r);
#endif
#endif /* if defined (__DJGPP__) */
} /* dvpause */

void dpmipause(void)
{
#if defined (__DJGPP__)
    __dpmi_regs r;
    r.x.ax = 0x1680;
    __dpmi_int(0x2f, &r);
#elif defined (__FLAT__)
    RMINF.EAX = 0x1680;
    int86x(0x2f);
#else
    union REGS r;
    r.x.ax = 0x1680;
    int86(0x2f, &r, &r);
#endif
}

int dpmicheck(void) /* checks for a DPMI host like OS/2 or Windows */
{
#if defined (__DJGPP__)
    return 1;  /* DJGPP always runs in DPMI mode ... */

#elif defined (__FLAT__)
    RMINF.EAX = 0x1687;
    int86x(0x2f);
    return !(RMINF.EAX & 0xFFFF);

#else
    union REGS r;
    r.x.ax = 0x1687;
    int86(0x2f, &r, &r);
    return !r.x.ax;

#endif
}

int dvcheck(void)
{
#if defined (__DJGPP__)
    __dpmi_regs r;
    r.x.cx = 0x4445;
    r.x.dx = 0x5351;
    r.x.ax = 0x2b01;
    __dpmi_int(0x21, &r);
    return !(r.h.al == 0xff);

#elif defined (__FLAT__)
    RMINF.ECX = 0x4445;
    RMINF.EDX = 0x5351;
    RMINF.EAX = 0x2b01;
    int86x(0x21);
    return !((RMINF.EAX & 0xff) == 0xff);

#else
    union REGS r;
    r.x.cx = 0x4445;
    r.x.dx = 0x5351;
    r.x.ax = 0x2b01;
    int86(0x21, &r, &r);
    return !(r.h.al == 0xff);

#endif
} /* dvcheck */

int kbdhit(void)
{
#if defined (__FLAT__) && !defined (__DJGPP__)
    RMINF.EAX = 0x100;
    int86x(0x16);
    return !(RMINF.flags & INTR_ZF);

#else
    return kbhit() ? 1 : 0;

#endif
}

unsigned int obtkey(void)
{
#if defined (__DJGPP__)
    __dpmi_regs r;
    r.x.ax = 0;
    __dpmi_int(0x16, &r);

    if(r.h.al == 0)
    {
        return r.x.ax;
    }
    else
    {
        return r.h.al;
    }

#elif defined (__FLAT__)
    RMINF.EAX = 0;
    int86x(0x16);

    if((RMINF.EAX & 0xff) == 0)
    {
        return RMINF.EAX & 0xffff;
    }
    else
    {
        return (unsigned)(RMINF.EAX & 0xff);
    }

#else  /* if defined (__DJGPP__) */
    union REGS r;
    r.x.ax = 0;
    int86(0x16, &r, &r);

    if(r.h.al == 0)
    {
        return r.x.ax;
    }
    else
    {
        return r.h.al;
    }

#endif /* if defined (__DJGPP__) */
} /* obtkey */

unsigned int dosavmem(void)
{
#if defined (__DJGPP__)
    __dpmi_regs r;
#else
    union REGS r;
    struct SREGS s;
#endif
    r.h.ah = 0x48;
#if defined (__DJGPP__)
    r.x.bx = 0xffff;
    __dpmi_int(0x21, &r);

    if(r.x.flags)
#elif defined (__FLAT__)
    r.w.bx = 0xffff;
    int386(0x21, &r, &r);

    if(r.x.cflag)
#else
    r.x.bx = 0xffff;
    int86(0x21, &r, &r);

    if(r.x.cflag)
#endif
    {
#if defined (__FLAT__) && !defined (__DJGPP__)
        return r.w.bx;

#else
        return r.x.bx;

#endif
    }
    else
    {
#if defined (__DJGPP__)
        r.x.es = r.x.ax;
#elif defined (__FLAT__)
        segread(&s);
        s.es = r.w.ax;
#else
        s.es = r.x.ax;
#endif
        r.h.ah = 0x49;
#if defined (__DJGPP__)
        __dpmi_int(0x21, &r);
#elif defined (__FLAT__)
        int386x(0x21, &r, &r, &s);
#else
        int86x(0x21, &r, &r, &s);
#endif
        return 0xffff;
    }
} /* dosavmem */

#if defined (__FLAT__) && !defined (__DJGPP__)

void int86x(unsigned short inty)
{
    union REGS regs;
    struct SREGS sregs;

    segread(&sregs);
    regs.w.ax  = 0x0300;
    regs.w.bx  = inty;
    regs.w.cx  = 0;
    sregs.es   = FP_SEG(&RMINF);
    regs.x.edi = FP_OFF(&RMINF);
    int386x(0x31, &regs, &regs, &sregs);
}

#endif
