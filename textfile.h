/*
 *  TEXTFILE.H
 *
 *  Released to the public domain.
 *
 *  Prototypes for TEXTFILE.C.
 */

#ifndef __TEXTFILE_H__
#define __TEXTFILE_H__

void import(LINE * l);
char *getfilename(char *buf);
void export(LINE * f);
void writetxt(void);

#endif
