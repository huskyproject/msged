/*
 *  BMG.H
 *
 *  Written by Paul Edwards and released to the public domain
 *
 *  Emulate Boyer-More-Gosper routines, without actually doing it, because
 *  Msged doesn't use the proper functionality, for unknown reasons.
 */

#ifndef __BMG_H__
#define __BMG_H__

void bmg_setsearch(char *pattern);
char *bmg_find(char *text, char *search);
char *bmg_search(char *text);

#endif
