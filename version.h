/*
 *  VERSION.H
 *
 *  Released to the public domain.
 */

#ifndef __VERSION_H__
#define __VERSION_H__

#define BETA 1   /* beta test release? */

#if BETA
#define CLOSED   " 05"
#define PIDBETA  ".05"
#else
#define CLOSED   ""
#define PIDBETA  ""
#endif

/*
 *  PID version is not allowed to have trailing 0's, but we don't mind them
 *  on the tearline.  :-)
 */

#ifndef PIDVER
#define PIDVER   "TE"
#endif

#ifndef VERSION
#define VERSION  "TE"
#endif

#ifdef MSDOS
#ifndef __FLAT__
#define PROG     "Msged"
#define OSID "DOS/16"
#else
#define PROG     "Msged/386"
#define OSID "DOS/386"
#endif
#endif

#ifdef OS2
#define OSID "OS2;OS/2"
#define PROG     "Msged/2"
#endif

#ifdef WINNT
#define OSID "W32;WNT;W95"
#define PROG     "Msged/NT"
#endif

#ifdef SASC
#define OSID "AMIGA"
#define PROG     "Msged/Ami"
#endif

#ifdef UNIX
#ifndef UNAME
#define UNAME "UNX"
#endif
#define PROG "Msged/"UNAME
#define OSID "UNIX;"UNAME
#endif

#ifndef PROG
#define OSID "UNKNOWN;DOS/16"
#define PROG     "Msged"
#endif

#endif
