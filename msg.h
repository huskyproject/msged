/*
 *  MSG.H
 *
 *  Released to the public domain.
 *
 *  Squish MSGAPI routines for Squish messagebases only.
 */

#ifndef __MSG_H__
#define __MSG_H__

int SquishMsgDelete(unsigned long n);
int SquishAreaSetLast(AREA * a);
int SquishMsgAreaClose(void);
int SquishMsgClose(void);
int JamMsgClose(void);
int SquishMsgWriteText(char *text, unsigned long msgn, unsigned long mlen);
int JamMsgWriteText(char *text, unsigned long msgn, unsigned long mlen);
int SquishMsgWriteHeader(msg * m, int type);
char *SquishMsgReadText(unsigned long n);
msg *SquishMsgReadHeader(unsigned long n, int type);
long SquishMsgAreaOpen(AREA * a);
unsigned long SquishUidToMsgn(unsigned long n);
unsigned long SquishMsgnToUid(unsigned long n);
void MsgApiInit(void);
void MsgApiTerm(void);
int SquishMsgLock(void);
int SquishMsgUnlock(void);

#endif
