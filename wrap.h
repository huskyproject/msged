/*
 *  WRAP.H
 *
 *  Released to the public domain.
 *
 *  Prototypes for WRAP.C.
 */

#ifndef __WRAP_H__
#define __WRAP_H__

char *e_getbind(unsigned int key);
char *e_getlabels(int i);
void e_assignkey(unsigned int key, char *label);
LINE *lineins(LINE * cl);
void ScrollDown(int y1, int y2);
void ScrollUp(int y1, int y2);
void RedrawPage(LINE * start, int wherey);
LINE *InsertLine(LINE * current);
void UnmarkLineBuf(void);
void SetLineBuf(void);
int iswhspace(char c);
int trailspace(char *text);
int isquote(char *text);
char *FindQuoteEnd(char *txt);
char *GetWrapPoint(char *text, int rm);
int wrap(LINE * cl, int x, int y, int rm);
void CheckXvalid(void);
int WordStart(void);
int editmsg(msg * m, int quote);
int ed_error(char *fc, char *fmt,...);
void count_bytes(LINE *l, long *bytes, long *quotes);
void adapt_margins(void);
#endif
