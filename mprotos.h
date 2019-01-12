/*
 *  MPROTOS.H
 *
 *  Released to the public domain.
 */

#ifndef __MPROTOS_H__
#define __MPROTOS_H__

void cleanup(char *msg,...);
void mygetcwd(char *buf, int len);
msg *readmsg(unsigned long n);
int writemsg(msg * m);
void deletemsg(void);
int showmsg(msg * m, int redraw);
void opening(char *, char *);
void parseareas(char *);
int nextmsg(int);
int selectarea(char *topMsg, int def);
int mainArea(void);
void showheader(msg * m);
FIDO_ADDRESS parsenode(char *s);
LINE *clearbuffer(LINE * buffer);
void import(LINE * l);
void export(LINE * f);
int confirm(char *option);
void save(msg * message);
FIDO_ADDRESS lookup(char *name, char *fn);
void makeReverse(char *revName, char *name);
void newmsg(void);
void reply(void);
void quote(void);
void reply_oarea(void);
void followup(void);
void show_help(void);
int draw_help(char *name);
void choose_attribline(void);
void movemsg(void);
void writetxt(void);
void change(void);
char *show_address(FIDO_ADDRESS * a);
char *show_4d(FIDO_ADDRESS * a);
char *striplwhite(char *s);
char *striptwhite(char *s);
void clearmsg(msg * m);
int setcwd(char *path);
void dispose(msg * message);
void strdel(char *l, int x);
LINE *insline(LINE * nl);
int dir_findnext(struct _dta *dta);
int dir_findfirst(char *filename, int attribute, struct _dta *dta);
void change_curr_addr(void);
void set_switch(void);
int handle_rot(int c);
void scan_all_areas(void);
void scan_unscanned_areas(void);
void area_scan(int);
int newarea(void);
void cursor(char state);
void replyextra(void);
void set_area(int newarea);
int do_list(void);
unsigned int KeyHit(void);
unsigned int ConvertKey(int ch);
unsigned int GetKey(void);
void dolist(void);

#ifdef MSDOS
#include "dosmisc.h"
#endif

#include "wrap.h"
#include "misc.h"

#endif
