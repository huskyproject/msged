/*
 *  FIDO.H
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 */

#ifndef __FIDO_H__
#define __FIDO_H__

int FidoMsgDelete(unsigned long n);
int FidoAreaSetLast(AREA * a);
long FidoMsgAreaOpen(AREA * a);
int FidoMsgAreaClose(void);
int FidoMsgClose(void);
char *FidoMsgReadText(unsigned long n);
msg *FidoMsgReadHeader(unsigned long n, int type);
int FidoMsgWriteHeader(msg * m, int type);
int FidoMsgWriteText(char *text, unsigned long n, unsigned long mlen);
unsigned long FidoMsgnToUid(unsigned long n);
unsigned long FidoUidToMsgn(unsigned long n);
int FidoMsgLock(void);
int FidoMsgUnlock(void);

#endif
