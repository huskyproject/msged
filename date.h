/*
 *  DATE.H
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Parse various string date formats into a UNIX-style timestamp.
 */

#ifndef __DATE_H__
#define __DATE_H__

time_t parsedate(char *ds);
char *firstname(char *name);
char *lastname(char *name);
char *itime(time_t now);
char *atime(time_t now);
char *mtime(time_t now);
char *qtime(time_t now);
char *attrib_line(msg * m, msg * old, int olda, char *format);

#endif
