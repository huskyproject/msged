/*
 *  HELPCMP.C
 *
 *  Written on 10-Jul-94 by John Dennis.  Modifications by Paul Edwards
 *  and Andrew Clarke.  Released to the public domain.
 *
 *  Msged help file compiler.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "help.h"

static FILE *ifp, *ofp;
static HFileHdr Fheader;
static HTopicHdr *topichdrs;

void helpcmp(int argc, char *argv[])
{
    long last;
    char line[255];
    int topics, curr, done;

    puts("Msged help file compiler");

    if (argc < 3)
    {
        printf("\nUsage: MSGED -hc <source> <target>\n");
        return;
    }

    ifp = fopen(argv[1], "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "HELPCMP: Error opening help source file, '%s' : %s\n",
          argv[1], strerror(errno));
        return;
    }

    ofp = fopen(argv[2], "wb");
    if (ofp == NULL)
    {
        fprintf(stderr, "HELPCMP: Error opening help target file, '%s' : %s\n",
          argv[2], strerror(errno));
        return;
    }

    done = 0;
    topics = 0;

    printf("\nHELPCMP: Compiling, pass one (reading)...");

    while (!done)
    {
        if (fgets(line, 254, ifp) == NULL)
        {
            break;
        }
        if (!strncmp(line, "*Begin", 6))
        {
            topics++;
        }
    }

    printf(" done.\n");

    if (topics >= 1)
    {
        printf("HELPCMP: Compiling, pass two (writing)...");

        topichdrs = calloc(topics, sizeof(HTopicHdr));
        if (topichdrs == NULL)
        {
            printf(" error!\n");
            fprintf(stderr, "HELPCMP: Memory allocation failure!\n");
            return;
        }

        memcpy(Fheader.signature, "cz", 3);
        Fheader.topics[0] = (unsigned char)(topics & 0xff);
        Fheader.topics[1] = (unsigned char)((topics >> 8) & 0xff);
        curr = 0;
        done = 0;

        fseek(ifp, 0L, SEEK_SET);

        fwrite(&Fheader, sizeof(HFileHdr), 1, ofp);
        fwrite(topichdrs, sizeof(HTopicHdr), topics, ofp);

        while (!done)
        {
            last = ftell(ofp);

            if (fgets(line, 254, ifp) == NULL)
            {
                break;
            }

            if (!strncmp(line, "*Begin", 6))
            {
                topichdrs[curr++].offset = last;
            }

            fprintf(ofp, "%s", line);
        }

        fseek(ofp, (long)sizeof(HFileHdr), SEEK_SET);
        fwrite(topichdrs, sizeof(HTopicHdr), topics, ofp);

        printf(" done.\n");
    }

    fclose(ofp);
    fclose(ifp);
}
