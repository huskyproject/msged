/*
 *  VERSION.H
 *
 *  Released to the public domain.
 */

#ifndef __VERSION_H__
#define __VERSION_H__

#define VERNUM "6.3"
#define VERPATCH ".0"
#define VERBRANCH "-current"
#define VERPROJECT "TE"

#ifdef MSDOS
#ifndef __FLAT__
#define PROG "Msged"
#define OSID "DOS/16"
#else
#define PROG "Msged/386"
#define OSID "DOS/386"
#endif
#endif

#ifdef OS2
#define OSID "OS2;OS/2"
#define PROG "Msged/2"
#endif

#ifdef WINNT
#define OSID "W32;WNT;W95"
#define PROG "Msged/NT"
#endif

#ifdef SASC
#define OSID "AMIGA"
#define PROG "Msged/Ami"
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
#define PROG "Msged"
#endif

#endif // ifndef __VERSION_H__
