#ifndef __FCONF_H
#define __FCONF_H

void check_fidoconfig(char * option_string);

/* the following routines are defined in config.c, but we define them
   here instead of in config.h because the prototypes require the
   whole bunch of msged.h headers to be included ... */
void AddArea(AREA * a);
void applyflags(AREA * a, char * flagstring);

extern char * areafileflags;


#endif
