#ifndef __CHARSET_H__
#define __CHARSET_H__

/*
 *  CHARSET.H
 *
 *  Written 1998 by Tobias Ernst. Released to the Public Domain.
 *
 *  A FSC-0054 compliant character set translation engine for MsgEd.
 */

typedef struct _lookuptable
{
    char           from_charset[9];
    char           to_charset[9];
    int            level;
    char           lookuptable[256];
} LOOKUPTABLE;

typedef struct _readwritemap
{
    char           charset_name[9];
    int            n_tables;
    LOOKUPTABLE   *tables;
} READWRITEMAPS;

void         read_charset_maps    (void); /* initialize the charset engine   */
void         destroy_charset_maps (void); /* destroy the charset engine      */

#define READ_DIRECTION 1
#define WRITE_DIRECTION 2

int          have_readtable      (const char *, int);
LOOKUPTABLE *get_readtable       (const char *, int);
LOOKUPTABLE *get_writetable      (const char *, int*);


void         strip_control_chars (char *);
char        *translate_text      (const char *, LOOKUPTABLE *);

#ifndef READMAPSDAT
#ifdef UNIX
#define READMAPSDAT "~/.msged.readmaps"
#define WRITMAPSDAT "~/.msged.writmaps"
#else
#define READMAPSDAT "readmaps.dat"
#define WRITMAPSDAT "writmaps.dat"
#endif
#endif

#endif


