/*
 *  MISC.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Miscellaneous functions for Msged.
 */

#define TEXTLEN 80

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "memextra.h"
#include "winsys.h"
#include "menu.h"
#include "nshow.h"
#include "main.h"
#include "dialogs.h"
#include "help.h"
#include "misc.h"
#include "charset.h"

void change_curr_addr(void)
{
    int ch, num;
    int x1, y1, x2, y2;
    static char line[TEXTLEN];
    char **addrch;

    if (SW->aliascount < 2)
    {
        return;
    }
    addrch = xcalloc(SW->aliascount + 2, sizeof(char *));

    for (num = 0; num < SW->aliascount; num++)
    {
        sprintf(line, "%-20.20s", show_address(&alias[num]));
        addrch[num] = xstrdup(line);
    }

    addrch[num] = NULL;

    x1 = (maxx / 2) - 11;
    x2 = (maxx / 2) + 11;
    y1 = 8;

    if (num > maxy - 1)
    {
        y2 = maxy - 1;
    }
    else
    {
        y2 = y1 + num + 1;
    }

    ch = DoMenu(x1 + 1, y1 + 1, x2 - 1, y2 - 1, addrch, 0, SELBOX_ADDRESS, "");

    switch (ch)
    {
    case -1:
        break;

    default:
        CurArea.addr = alias[ch];
        ShowNewArea();
        ShowMsgHeader(message);
        break;
    }

    for (num = 0; num < SW->aliascount; num++)
    {
        xfree(addrch[num]);
    }
    xfree(addrch);
}

void change_nodelist(void)
{
    int ch, num;
    int x1, y1, x2, y2;
    static char line[TEXTLEN];
    char **nodls;

    if (SW->nodelists < 2)
    {
        return;
    }
    nodls = xcalloc(SW->nodelists + 2, sizeof(char *));

    for (num = 0; num < SW->nodelists; num++)
    {
        sprintf(line, "%-20.20s", node_lists[num].name);
        nodls[num] = xstrdup(line);
    }

    nodls[num] = NULL;

    x1 = (maxx / 2) - 11;
    x2 = (maxx / 2) + 11;
    y1 = 8;

    if (num > maxy - 1)
    {
        y2 = maxy - 1;
    }
    else
    {
        y2 = y1 + num + 1;
    }

    ch = DoMenu(x1 + 1, y1 + 1, x2 - 1, y2 - 1, nodls, 0, SELBOX_NODELIST, "");

    switch (ch)
    {
    case -1:
        break;

    default:
        ST->nodebase = node_lists[ch].base_name;
        ST->sysop = node_lists[ch].sysop;
        break;
    }

    for (num = 0; num < SW->nodelists; num++)
    {
        xfree(nodls[num]);
    }
    xfree(nodls);
}

void change_username(void)
{
    int x1, y1, x2, y2, len = 0;
    int i, j, ch;
    char *list[11];

    if (user_list[1].name == NULL)
    {
        return;
    }

    for (i = 0; i < 11; i++)
    {
        if (user_list[i].name == NULL)
        {
            break;
        }

        list[i] = xstrdup(user_list[i].name);

        j = strlen(list[i]);
        if (j >= len)
        {
            len = j;
        }
        if (j >= maxx - 2)
        {
            *(list[i] + maxx - 4) = '\0';
        }
    }

    list[i] = NULL;

    if (len >= maxx - 2)
    {
        len = maxx - 4;
    }

    x1 = (maxx / 2) - (len / 2) - 1;
    x2 = (maxx / 2) + (len / 2) + 1;
    y1 = 7;
    y2 = y1 + i + 1;

    ch = DoMenu(x1 + 1, y1 + 1, x2 - 1, y2 - 1, list, 0, SELBOX_USERNAME, "");

    switch (ch)
    {
    case -1:
        break;

    default:
        release(ST->username);
        ST->username = xstrdup(user_list[ch].name);

        release(ST->lastread);

        if (user_list[ch].lastread != NULL)
        {
            ST->lastread = xstrdup(user_list[ch].lastread);
        }
        else
        {
            ST->lastread = xstrdup(user_list[0].lastread);
        }

        SW->useroffset = user_list[ch].offset;
        break;
    }
    i = 0;
    while (list[i])
    {
        xfree(list[i++]);
    }
}

void set_switch(void)
{
    WND *hCurr, *hWnd;
    int ret;

    ReadSettings();

    hCurr = WndTop();
    hWnd = WndPopUp(52, 20, INSBDR | SHADOW, cm[DL_BTXT], cm[DL_WTXT]);

    ret = DoDialog(&settings, 1);

    WndClose(hWnd);
    WndCurr(hCurr);

    if (ret == ID_OK)
    {
        WriteSettings();
        ShowNewArea();
        ShowMsgHeader(message);
        message = KillMsg(message);
    }
}

void show_help(void)
{
    if (ST->helpfile != NULL)
    {
        DoHelp(0);
    }
}

int handle_rot(int c)
{
    if (rot13 == 1)
    {
        if (isalpha((unsigned char)c))
        {
            if (toupper(c) >= 'A' && toupper(c) <= 'M')
            {
                c += 13;
            }
            else
            {
                c -= 13;
            }
        }
    }
    else
    {
        if ((unsigned char)c > '!')
        {
            if ((unsigned char)c + 47 > '~')
            {
                c = (unsigned char)c - 47;
            }
            else
            {
                c = (unsigned char) + 47;
            }
        }
    }
    return c;
}

/*
 *  This function is used when uudecoding is required using the
 *  built-in editor as GetString used in uudecode() leaves the cursor
 *  invisible when it returns.
 */

#define UUDECODE(c) (char)(((c) - ' ') & 077)

void uudecode(void)
{
    unsigned char temp[4], bin[3];
    static char buff[PATHLEN];
    int len, i, bytes;
    FILE *ofp;
    LINE *cur;

    if (message == NULL)
    {
        return;
    }

    *buff = '\0';

    cur = message->text;

    while (cur != NULL)
    {
        if (sscanf(cur->text, "begin %o %s", &i, buff) > 0)
        {
            break;
        }
        cur = cur->next;
    }

    if (cur == NULL)
    {
        return;
    }

    if (!GetString("UUdecode", "Enter filename", buff, sizeof buff))
    {
        return;
    }

    ofp = fopen(buff, "wb");

    if (ofp == NULL)
    {
        return;
    }

    if (cur != NULL)
    {
        cur = cur->next;

        while (cur != NULL && *cur->text != '\n' && *cur->text != '`' &&
          strncmp(cur->text, "end", 3))
        {
            len = UUDECODE(*(cur->text));

            if (!len)
            {
                break;
            }

            i = 0;
            while (len > 0)
            {
                temp[0] = UUDECODE(*(cur->text + i + 1));
                temp[1] = UUDECODE(*(cur->text + i + 2));
                temp[2] = UUDECODE(*(cur->text + i + 3));
                temp[3] = UUDECODE(*(cur->text + i + 4));

                bytes = 0;
                if (len > 0)
                {
                    *bin = (unsigned char)
                      ((*temp << 2) | (*(temp + 1) >> 4));
                    bytes++;
                    len--;
                    if (len > 0)
                    {
                        *(bin + 1) = (unsigned char)
                          ((*(temp + 1) << 4) | (*(temp + 2) >> 2));
                        bytes++;
                        len--;
                        if (len > 0)
                        {
                            *(bin + 2) = (unsigned char)
                              ((*(temp + 2) << 6) | (*(temp + 3)));
                            bytes++;
                            len--;
                        }
                    }
                }

                if (fwrite(bin, 1, bytes, ofp) != bytes)
                {
                    ChoiceBox("", "WARNING: Could not write to file", "  Ok  ", NULL, NULL);
                    fclose(ofp);
                    return;
                }

                i += 4;
            }

            cur = cur->next;
        }
    }
    fclose(ofp);
}

void e_uudecode(void)
{
    uudecode();
    TTCurSet(1);
}

void hex_dump(void)
{
    static char buff[PATHLEN];
    static char prtln[100];
    int c, i, pos1 = 0, pos2 = 0;
    FILE *ofp;
    LINE *cur;
    long x;

    if (message == NULL)
    {
        return;
    }

    x = 0L;
    *buff = '\0';

    cur = message->text;
    if (cur == NULL)
    {
        return;
    }

    if (!GetString("", "Enter filename", buff, sizeof buff))
    {
        return;
    }

    ofp = fopen(buff, "w");

    if (ofp == NULL)
    {
        ChoiceBox("", "WARNING: Could not write to file", "  Ok  ", NULL, NULL);
        return;
    }

    while (cur != NULL)
    {
        i = 0;
        c = cur->text[i++];
        while (c != '\0')
        {
            if (x % 16 == 0)
            {
                memset(prtln, ' ', sizeof prtln);
                sprintf(prtln, "%0.6lX   ", x);
                pos1 = 8;
                pos2 = 45;
            }
            sprintf(prtln + pos1, "%0.2X", c);
            if (isprint(c))
            {
                sprintf(prtln + pos2, "%c", c);
            }
            else
            {
                sprintf(prtln + pos2, ".");
            }
            pos1 += 2;
            *(prtln + pos1) = ' ';
            pos2++;
            if (x % 4 == 3)
            {
                *(prtln + pos1) = ' ';
                pos1++;
            }
            if (x % 16 == 15)
            {
                fprintf(ofp, "%s\n", prtln);
            }
            x++;
            c = cur->text[i];
            i++;
        }

        cur = cur->next;
    }

    if (x % 16 != 0)
    {
        fprintf(ofp, "%s\n", prtln);
    }
    fclose(ofp);
}

/* select the charset to use */

void sel_chs(void)
{
    static char *stdrecode = "Use @CHRS kludge (recommended!)";
    static char *norecode  = "Don't do any translation at all";
    char *kludgelist;
    int kn, ksz;
    int num, ch = -1;
    int x1, y1, x2, y2;
    char **kludgemenu;

    kludgelist = get_known_charset_table(&kn, &ksz);
    if (kludgelist == NULL)
    {
        return;
    }

    kludgemenu = xcalloc(kn + 4, sizeof(char *));

    kludgemenu[0] = stdrecode;
    kludgemenu[1] = norecode;

    for (num = 0; num < kn; num++)
    {
        kludgemenu[num + 2] = kludgelist + num * ksz;
    }
    
    kludgemenu[num + 2] = NULL;

    x1 = (maxx / 2) - 17;
    x2 = (maxx / 2) + 17;
    y1 = 8;

    if (num + 2 > maxy - 1)
    {
        y2 = maxy - 1;
    }
    else
    {
        y2 = y1 + num + 3;
    }

    while (ch < 0)
        ch = DoMenu(x1 + 1, y1 + 1, x2 - 1, y2 - 1, kludgemenu, 0,
                    SELBOX_CHARSET, "Override Character Set Recognition");

    switch (ch)
    {
    case 0:
        release(ST->enforced_charset);
        break;

    case 1:
        release(ST->enforced_charset);
        ST->enforced_charset = xstrdup(get_local_charset());
        break;

    default:
        release(ST->enforced_charset);
        ST->enforced_charset = xstrdup(kludgemenu[ch]);
        break;
    }

    message = KillMsg(message); /* force message re-read */

    xfree(kludgemenu);
}
