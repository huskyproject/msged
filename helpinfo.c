/*
 *  HELPINFO.C
 *
 *  Written on 10-Jul-94 by John Dennis.  Modifications by Andrew Clarke.
 *  Released to the public domain.
 *
 *  Msged help file decompiler; displays Msged help file information and
 *  contents.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "help.h"

static char *line = NULL;
static FILE *fp;
static HFileHdr Fheader;
static HTopicHdr *topics;
static int setup, CurrTopic;
static int numTopics;

static void helpinfoInit(char *fnm)
{
    int i;
    setup = 0;
    fp = fopen(fnm, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "HELPINFO: Error opening help source file, '%s' : %s\n",
          fnm, strerror(errno));
        return;
    }
    fread(&Fheader, sizeof Fheader, 1, fp);
    numTopics = (Fheader.topics[1] << 8) | Fheader.topics[0];
    topics = calloc(numTopics, sizeof(HTopicHdr));
    if (topics == NULL)
    {
        fprintf(stderr, "HELPINFO: Memory allocation failure!\n");
        return;
    }
    for (i = 0; i < numTopics; i++)
    {
        fread(&topics[i], sizeof *topics, 1, fp);
    }
    setup = 1;
    CurrTopic = 0;
}

static void helpinfoDisplayPage(long offset)
{
    char *s;

    fseek(fp, offset, SEEK_SET);

    if (line == NULL) line = xmalloc(255);

    while (fgets(line, 254, fp) != NULL)
    {
        if (!strncmp(line, "*Page", 5) || !strncmp(line, "*End", 4))
        {
            break;
        }

        if (*line != '\n')
        {
            s = strchr(line, '\n');
            if (s != NULL)
            {
                *s = '\0';
            }

            printf("%s\n", line);
        }
    }
}

static void helpinfoDoHelp(int topic)
{
    long offset[20];
    int page, pages;

    if (line == NULL) line = xmalloc(255);

    if (topic < 0 || topic > numTopics)
    {
        return;
    }
    fseek(fp, topics[topic].offset, SEEK_SET);

    if (fgets(line, 254, fp) == NULL)
    {
        fprintf(stderr, "HELPINFO: Input line too long!\n");
        return;
    }

    if (strncmp(line, "*Begin", 6))
    {
        return;
    }

    pages = 1;
    offset[pages - 1] = ftell(fp);

    while (1)
    {
        if (fgets(line, 254, fp) == NULL)
        {
            fprintf(stderr, "HELPINFO: Input line too long!\n");
            break;
        }

        if (!strncmp(line, "*End", 4))
        {
            break;
        }

        if (!strncmp(line, "*Page", 5))
        {
            pages++;
            offset[pages - 1] = ftell(fp);
        }
    }

    fseek(fp, offset[0], SEEK_SET);

    page = 0;

    while (page < pages)
    {
        helpinfoDisplayPage(offset[page++]);
    }
}

void helpinfo(int argc, char *argv[])
{
    int curr;

    puts("Msged help file decompiler");

    if (argc < 2)
    {
        printf("\nUsage: MSGED -hi <source>\n");
        return;
    }

    helpinfoInit(argv[1]);

    curr = 0;
    while (curr < numTopics)
    {
        helpinfoDoHelp(curr);
        curr++;
    }
}
