/*
 *  QUICK.H
 *
 *  Written on 30-Jul-90 by jim nutt and released to the public domain.
 *
 *  Support for QuickBBS-style message bases.
 */

#ifndef __QUICK_H__
#define __QUICK_H__

int QuickMsgDelete(unsigned long n);
int QuickAreaSetLast(AREA * a);
long QuickMsgAreaOpen(AREA * a);
int QuickMsgAreaClose(void);
int QuickMsgClose(void);
char *QuickMsgReadText(unsigned long n);
msg *QuickMsgReadHeader(unsigned long n, int type);
int QuickMsgWriteHeader(msg * m, int type);
int QuickMsgWriteText(char *text, unsigned long n, unsigned long mlen);
unsigned long QuickMsgnToUid(unsigned long n);
unsigned long QuickUidToMsgn(unsigned long n);

#endif
