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
void export_text(msg *, LINE *);
void export(LINE * f); /* wrapper for export_text(f, NULL) */
void writetxt(void);   /* wrapper for export_text(message, NULL) */

#endif
