/*
 *  READMAIL.H
 *
 *  Released to the public domain.
 *
 *  Prototypes for READMAIL.C.
 */

#ifndef __READMAIL_H__
#define __READMAIL_H__

void mygetcwd(char * buf, int len);
int setcwd(char * path);
msg * readmsg(unsigned long n);
int writemsg(msg * m);
void clearmsg(msg * m);
LINE * clearbuffer(LINE * buffer);
void checkrcvd(msg * m, unsigned long n);
void GetOrigin(char * origin);
unsigned long sec_time(void);
LINE * InsertAfter(LINE * l, char * text);

extern int read_verbatim;  /* If this is turned on, the next call to
                              readmsg will NOT strip any kludge lines
                              and will NOT do charset translation.
                              readmsg automatically resets this
                              variable to zero. */

#endif
