/*
 *  VIO.H
 *
 *  Written by jim nutt and released to the public domain.
 *
 *  Prototypes for VIO.H.
 */

#ifndef __VIO_H__
#define __VIO_H__

/* initialization and termination functions */

unsigned short VIOopen(void);
void VIOclose(void);

/* scrolling functions */

void VIOscrollright(int x1, int y1, int x2, int y2, int count);
void VIOscrollleft(int x1, int y1, int x2, int y2, int count);
void VIOscrollup(int x1, int y1, int x2, int y2, int count);
void VIOscrolldown(int x1, int y1, int x2, int y2, int count);

/* screen clear */

void VIOclear(int x1, int y1, int x2, int y2);

/* write to screen */

void VIOputc(const char c);
void VIOputs(const char *s);
void VIOputr(int x, int y, int w, int h, unsigned short *b);

/* read from screen */

unsigned short VIOgetca(const int x, const int y);
void VIOgetra(int x1, int y1, int x2, int y2, unsigned short *b);

/* set colors */

void VIOsetfore(const int c);
void VIOsetback(const int c);

/* get current color settings */

unsigned short VIOgetfore(void);
unsigned short VIOgetback(void);

/* set the write cursor */

void VIOgotoxy(int x, int y);

/* update the screen and visible cursor */

void VIOupdate(void);

/* get the current write cursor position */

unsigned short VIOwherex(void);
unsigned short VIOwherey(void);

/* get screen information */

unsigned short VIOsegment(void);

unsigned short VIOcolumns(void);
unsigned short VIOrows(void);
unsigned short VIOmode(void);
unsigned short VIOheight(void);

/* set segment information */

void VIOsetSegment(unsigned int s);
void VIOsetRows(int r);
void VIOsetCols(int c);

/* get BIOS cursor location */

void VIOcursor(int *x, int *y, int *shape);

#endif
