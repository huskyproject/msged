#ifndef __ENVIRON_H
#define __ENVIRON_H
/*
 * ENVIRON.H
 *
 * Written 1998 by Tobias Ernst and release to the Public Domain
 *
 * Environment variable (%MAILBOXDRIVE% -> E: and so on) expander.
 */
char * env_expand(char * line);

#endif
