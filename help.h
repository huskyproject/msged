/*
 *  HELP.H
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Help subsystem structures and function prototypes.
 */

#ifndef __HELP_H__
#define __HELP_H__

typedef struct _hfilehdr
{
    char signature[3];          /* must be "cz<null>" */
    unsigned char topics[2];    /* number of topics */
}
HFileHdr;

typedef struct _topichdr
{
    long offset;                /* offset to the topic */
}
HTopicHdr;

typedef struct _topic
{
    long pages;
}
HTopic;

void HelpInit(char *fileName);
void DoHelp(int topic);

#endif
