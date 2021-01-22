/*
 *  A series of routines to provide access to Microsoft (and compatible)
 *  mice.  Consult your mouse documentation for detailed information
 *  regarding each mouse driver function.
 *
 *  Written by Bob Jarvis.  Presumed to be in the public domain.
 *  Modified for Msged use by Matthew Parker and Andrew Clarke.
 */

#include <dos.h>
#if defined (__DJGPP__)
#include <dpmi.h>
#elif defined (__FLAT__)
#include "rmi.h"
extern struct rminfo RMINF;
#endif
#define MSMOUSE 0x33

int mouse_present = 0;  /* globally visible */
/*
 *  Uses driver function 0 to initialize the mouse software to its
 *  default settings.  If no mouse is present it returns 0.  If a
 *  mouse is present, it returns -1, and returns the number of mouse
 *  buttons in *mousetype.  Also initializes the global variable
 *  mouse_present (0 = no mouse, else a mouse is present).
 */
int ms_reset(int * mousetype)
{
#if defined (__DJGPP__)
    __dpmi_regs workregs;
    workregs.x.ax = 0;
    __dpmi_int(MSMOUSE, &workregs);
    *mousetype    = workregs.x.bx;
    mouse_present = workregs.x.ax;
#elif defined (__FLAT__)
    RMINF.EAX = 0;
    int86x(MSMOUSE);
    *mousetype    = RMINF.EBX & 0xffff;
    mouse_present = RMINF.EAX & 0xffff;
#else
    union REGS workregs;
    workregs.x.ax = 0;
    int86(MSMOUSE, &workregs, &workregs);
    *mousetype    = workregs.x.bx;
    mouse_present = workregs.x.ax;
#endif
    return mouse_present;
}

/*
 *  Makes the mouse cursor visible.
 */
int ms_show_cursor(void)
{
#if defined (__DJGPP__)
    __dpmi_regs workregs;
    workregs.x.ax = 1;
    __dpmi_int(MSMOUSE, &workregs);
#elif defined (__FLAT__)
    RMINF.EAX = 1;
    int86x(MSMOUSE);
#else
    union REGS workregs;
    workregs.x.ax = 1;
    int86(MSMOUSE, &workregs, &workregs);
#endif
    return -1;
}

/*
 *  Hides the mouse cursor.  Should be called before changing any
 *  portion of the screen under the mouse cursor.
 */
int ms_hide_cursor(void)
{
#if defined (__DJGPP__)
    __dpmi_regs workregs;
    workregs.x.ax = 2;
    __dpmi_int(MSMOUSE, &workregs);
#elif defined (__FLAT__)
    RMINF.EAX = 2;
    int86x(MSMOUSE);
#else
    union REGS workregs;
    workregs.x.ax = 2;
    int86(MSMOUSE, &workregs, &workregs);
#endif
    return -1;
}

/*
 *  Obtains information about the mouse position and button status.
 *  Places the current horizontal and vertical positions in *horizpos
 *  and *vertpos, respectively.  Returns the mouse button status, which
 *  is mapped at the bit level as follows:
 *
 *      bit 0 - left button
 *      bit 1 - right button
 *      bit 2 - middle button
 *
 *  (0 = button up, 1 = button down)
 */
int ms_get_mouse_pos(int * horizpos, int * vertpos)
{
#if defined (__DJGPP__)
    __dpmi_regs workregs;
    workregs.x.ax = 3;
    __dpmi_int(MSMOUSE, &workregs);
    *horizpos = workregs.x.cx / 8;
    *vertpos  = workregs.x.dx / 8;
    return workregs.x.bx;

#elif defined (__FLAT__)
    RMINF.EAX = 3;
    int86x(MSMOUSE);
    *horizpos = (RMINF.ECX & 0xffff) / 8;
    *vertpos  = (RMINF.EDX & 0xffff) / 8;
    return RMINF.EBX & 0xffff;

#else
    union REGS workregs;
    workregs.x.ax = 3;
    int86(MSMOUSE, &workregs, &workregs);
    *horizpos = workregs.x.cx / 8;
    *vertpos  = workregs.x.dx / 8;
    return workregs.x.bx;

#endif
} /* ms_get_mouse_pos */
