/*
 *  CRITICAL.H
 *
 *  Written by Matthew Parker and released to the public domain.
 *
 *  MS-DOS critical error handler for Msged.
 */

#ifndef __CRITICAL_H__
#define __CRITICAL_H__

#if defined (MSDOS) && defined (__WATCOMC__)
#define DOSPREF cdecl
#else
#define DOSPREF
#endif

void DOSPREF install24h(void);
void DOSPREF remove24h(void);

#endif
