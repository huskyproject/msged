/*
 *  SPAWN.H
 *
 *  Prototypes for SPAWN.ASM.  This file released to the public domain.
 *
 *  These swap routines imply the use of Thomas Wagner's EXEC module
 *  somewhere in the executable.  If you don't have this (it's free, and
 *  PD), then simply define NODOSSWAP.
 */

#ifndef __SPAWN_H__
#define __SPAWN_H__

/* Return codes (only upper byte significant) */

#define RC_PREPERR   0x0100
#define RC_NOFILE    0x0200
#define RC_EXECERR   0x0300
#define RC_ENVERR    0x0400
#define RC_SWAPERR   0x0500

/* Swap method and option flags */

#define USE_EMS      0x01
#define USE_XMS      0x02
#define USE_FILE     0x04
#define EMS_FIRST    0x00
#define XMS_FIRST    0x10
#define HIDE_FILE    0x40
#define NO_PREALLOC  0x100
#define CHECK_NET    0x200

#define USE_ALL      (USE_EMS | USE_XMS | USE_FILE)

#define SWAP_FILENAME "$msged.swp"

/* internal flags for prep_swap */

#define CREAT_TEMP      0x0080
#define DONT_SWAP_ENV   0x4000

#if defined(MSDOS) && defined(__WATCOMC__)
#define DOSPREF cdecl
#else
#define DOSPREF
#endif

int DOSPREF do_spawn(int swapping, char *execfname, char *cmdtail, unsigned envlen, char *envp);
int DOSPREF prep_swap(unsigned method, char *swapfname);

#endif
