#ifndef __RMI_H__
#define __RMI_H__

void int86x(unsigned short int);

struct rminfo
{
    long EDI, ESI, EBP, rsvd, EBX, EDX, ECX, EAX;
    short flags, ES, DS, FS, GS, IP, CS, SP, SS;
};

#endif
