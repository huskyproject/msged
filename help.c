/*
 *  HELP.C
 *
 *  Written by John Dennis and released to the public domain.
 *
 *  Help subsystem code.
 */

#include <stdio.h>
#include <string.h>
#include "memextra.h"
#include "winsys.h"
#include "menu.h"
#include "main.h"
#include "keys.h"
#include "help.h"

static FILE *help;
static HFileHdr Fheader;
static HTopicHdr *topics;
static char line[255];
static int setup;
static int CurrTopic;
static int numTopics;

#ifdef __DJGPP__
#include <io.h>
#include <fcntl.h>
#endif

#ifdef __TURBOC__
#include <share.h>
#endif

void HelpInit(char *fileName)
{
    int i;

#ifdef __TURBOC__
    help = _fsopen(fileName, "rb", SH_DENYNONE);
#elif defined(__DJGPP__)
    int handle = sopen(fileName, O_RDONLY|O_BINARY, SH_DENYNO, 0);
    if (handle == -1)
    {
        return;
    }
    help = fdopen(handle, "rb");
#else
    help = fopen(fileName, "rb");
#endif    
    if (help == NULL)
    {
        return;
    }

    setup = 0;

    fread(&Fheader, sizeof(HFileHdr), 1, help);
    numTopics = (Fheader.topics[1] << 8) | Fheader.topics[0];
    topics = xcalloc(numTopics, sizeof *topics);
    for (i = 0; i < numTopics; i++)
    {
        fread(&topics[i], sizeof (HTopicHdr), 1, help);
    }
    setup = 1;
    CurrTopic = 0;
}

void DisplayPage(long offset, int max)
{
    char *s;
    int done;
    int line_num;

    done = 0;
    line_num = 0;

    fseek(help, offset, SEEK_SET);

    WndClear(0, 0, 54, 14, cm[HP_NTXT]);

    while (!done)
    {
        if (line_num == max)
        {
            break;
        }

        if (fgets(line, 254, help) == NULL)
        {
            break;
        }

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

            if (!strncmp(line, "*High", 5))
            {
                s = line + 5;
                WndWriteStr(0, line_num, cm[HP_TTXT], s);
            }
            else
            {
                WndWriteStr(0, line_num, cm[HP_NTXT], line);
            }
        }
        line_num++;
    }
}

void DoHelp(int topic)
{
    WND *hWnd, *hCurr;
    long offset[20];
    int depth, page, pages, ch, done;

    if (help == NULL)
    {
        return;
    }

    if (topic < 0 || topic > numTopics)
    {
        return;
    }

    fseek(help, topics[topic].offset, SEEK_SET);

    if (fgets(line, 254, help) == NULL)
    {
        return;
    }

    if (strncmp(line, "*Begin", 6))
    {
        return;
    }

    done = 0;
    pages = 1;
    offset[pages - 1] = ftell(help);

    while (!done)
    {
        if (fgets(line, 254, help) == NULL)
        {
            return;
        }

        if (!strncmp(line, "*End", 4))
        {
            break;
        }

        if (!strncmp(line, "*Page", 5))
        {
            pages++;
            offset[pages - 1] = ftell(help);
        }
    }

    fseek(help, offset[0], SEEK_SET);

    hCurr = WndTop();
    hWnd = WndPopUp(60, 18, INSBDR | SHADOW, cm[HP_BTXT], cm[HP_NTXT]);

    WndTitle(" Help ", cm[HP_TTXT]);

    done = 0;
    page = 0;
    depth = 14;

    DisplayPage(offset[page], depth);

    while (!done)
    {
        ch = TTGetChr();
        switch (ch)
        {
        case Key_PgDn:
            if (page + 1 < pages)
            {
                page++;
                DisplayPage(offset[page], depth);
            }
            break;

        case Key_PgUp:
            if (page > 0)
            {
                page--;
                DisplayPage(offset[page], depth);
            }
            break;

        case Key_Esc:
            done = TRUE;
            break;

        default:
            break;
        }
    }
    WndClose(hWnd);
    WndCurr(hCurr);
}
