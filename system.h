/*
 *  SYSTEM.H
 *
 *  Released to the public domain.
 *
 *  Prototypes for SYSTEM.C.
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

void PushHotGroup(HotGroup * New);
void PopHotGroup(void);
int LocateHotItem(int x, int y, unsigned long wid);
unsigned int MnuGetMsg(EVT * event, unsigned long wid);
void RegisterKeyProc(int (* fnc)(int ch));

#endif
