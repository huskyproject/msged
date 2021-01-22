/*
 *  QUOTE.H
 *
 *  Released to the public domain.
 *
 *  Prototypes for QUOTE.C.
 */

#ifndef __QUOTE_H__
#define __QUOTE_H__

int is_quote(char * text);
int is_same_quote(LINE * l, LINE * o);
int is_blank(LINE * l);
char * replace_noise(char * text);
LINE * makequote(LINE * l, char * isfrom);

#endif
