#ifndef READTC_H
#define READTC_H

#define QUERY_ALTCHARSET 1

void query_termcap(int what);

extern char *tt_specials;
extern char *tt_alternate_start;
extern char *tt_alternate_end;
extern char *tt_showcursor;
extern char *tt_hidecursor;

#endif
