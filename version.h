/*
 *  VERSION.H
 *
 *  Released to the public domain.
 */

#ifndef __VERSION_H__
#define __VERSION_H__

#define BETA 1   /* beta test release? */

#if BETA
#define CLOSED   " 05 (pre)"
#define PIDBETA  ".05.pre"
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
#else
#define PROG     "Msged/386"
#endif
#endif

#ifdef OS2
#define PROG     "Msged/2"
#endif

#ifdef WINNT
#define PROG     "Msged/NT"
#endif

#ifdef SASC
#define PROG     "Msged/Ami"
#endif

#ifdef UNIX
#ifdef UNAME
#define PROG "Msged/"UNAME
#else
#define PROG     "Msged/UNX"
#endif
#endif

#ifndef PROG
#define PROG     "Msged"
#endif

#endif
