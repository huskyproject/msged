/*
 *  CHARSET.C
 *
 *  Written 1998 by Tobias Ernst. Released to the Public Domain.
 *
 *  A FSC-0054 compliant character set translation engine for MsgEd.
 */

#include <time.h>
#include <stdio.h>
#include <string.h>

#include "addr.h"
#include "memextra.h"
#include "strextra.h"
#include "nedit.h"
#include "charset.h"
#include "msged.h"
#include "config.h"


static READWRITEMAPS *readmaps=NULL, *writemaps=NULL;
static int toasc_encountered=0;

/* The maskout_table is a lookup table that simply replaces all
   characters with an ASCII code >= 128 with question marks. It is
   necessary to do this translation when reading a mail without
   charset kludge, because untranslated special characters could
   create strange effects in the UNIX version running in an xterm. */

static LOOKUPTABLE maskout_table;


READWRITEMAPS *read_map(const char *filename)
{
    READWRITEMAPS *map = xmalloc(sizeof(READWRITEMAPS));
    FILE *fp = fopen(filename, "rb");
    char temp[64];
    int little_endian;
    long file_length;
    int i;

    map->tables = NULL;

    if (fp == NULL)
    {
       goto cleanup;           /* file not found - exit w/o error message  */
    }


    fseek(fp, 0, SEEK_END);    /* determine if the file has the right   */
    file_length = ftell(fp);   /* size (12 + n*256) and the number n of */
                               /* lookup tables                         */
    if ((file_length - 12) % (256 + 32))
    {
       goto error;
    }

    map->n_tables = (int) ((file_length - 12 ) / (256 + 32));
    fseek(fp, 0, SEEK_SET);

    if (fread(temp, 12, 1, fp) != 1)   /* read the map file header        */
    {
       goto error;
    }

    little_endian = (temp[0] == 1);   /* is the file for intel or vax?    */
    map->charset_name[8] = 0;         /* determine the character set name */
    memmove(map->charset_name, temp + 4, 8);

                                  /* allocate room for the lookup tables  */
    map->tables = xcalloc(map->n_tables, sizeof(LOOKUPTABLE));

                                  /* read in the individual lookup tables */
    for (i = 0; i < map->n_tables; i++)
    {                             /* read in the table header             */
        if (fread(temp, 32, 1, fp) != 1)
        {
            goto error;
        }

        if (temp[0] || temp[1] || temp[2] || temp[3])     /* id must be 0 */
        {
            goto error;
        }

        if (little_endian)
        {                                       /* mod rev must be 0 or 1 */
            if ((temp[4] != 1 && temp[4] != 0 ) || (temp[5] != 0) )
            {
               goto error;
            }
            map->tables[i].level = (temp[7] << 8 ) + temp[6];
        }
        else
        {                                       /* mod rev must be 0 or 1 */
            if ((temp[5] != 1 && temp[5] != 0 ) || (temp[4] != 0) )
            {
               goto error;
            }
            map->tables[i].level = (temp[6] << 8 ) + temp[7];
        }

        map->tables[i].from_charset[8] = 0;
        map->tables[i].to_charset[8] = 0;
        memmove (map->tables[i].from_charset, temp+16, 8);
        memmove (map->tables[i].to_charset, temp+24, 8);
        if (stricmp(map->tables[i].to_charset,"ASCII") == 0)
        {
           toasc_encountered = 1;
        }

                                              /* read in the table itself */
        if (fread(map->tables[i].lookuptable, 256, 1, fp) != 1)
        {
            goto error;
        }
    }

    fclose (fp);
    return map;

error:
    fprintf (stderr,
             "\r\aError reading %s: Unrecognized or corrupt file format.\n",
             filename);
cleanup:
    if (fp != NULL)
    {
        fclose(fp);
    }
    if (map != NULL)
    {
        if (map->tables != NULL)
        {
            xfree(map->tables);
        }
        xfree(map);
    }
    return NULL;
}


void read_charset_maps(void)
{
    int i;
    char *fnr, *fnw;

    destroy_charset_maps();

                                /* fill in the maskout table */
    strcpy(maskout_table.from_charset, "ASCII");
    strcpy(maskout_table.to_charset, "ASCII");
    maskout_table.level = 2;
    for (i = 0; i < 128; i++)
    {
        maskout_table.lookuptable[i * 2] = '\001';
        maskout_table.lookuptable[i * 2 + 1] = '?';
    }

    fnr = shell_expand(xstrdup(READMAPSDAT));
    fnw = shell_expand(xstrdup(WRITMAPSDAT));
    
    readmaps=read_map(fnr);
    toasc_encountered=0;
    writemaps=read_map(fnw);

    if (readmaps == NULL || writemaps == NULL)
    {
        fprintf (stderr,
                 "\r\aWarning: Could not open %s. You should correct this",
                 readmaps == NULL ? READMAPSDAT : WRITMAPSDAT);
        fprintf (stderr,
                 "\n         before you try to use umlauts, accented "
                 "characters or IBM graphics.\n");
    }

    if (readmaps != NULL && writemaps != NULL)
    {
        if (!toasc_encountered)
        {
           fprintf (stderr,
                    "\r\aWarning: %s does not contain an entry for converting"
                    "\n         back to ASCII!", fnw);
        }
        if (strcmp(readmaps->charset_name, writemaps->charset_name) == 0 &&
            toasc_encountered)
        {
           printf ("\rIncorporating FSC 0054 charset engine. "
                   "Local charset is: %s\n", readmaps->charset_name);
           xfree(fnr);
           xfree(fnw);
           return;
        }
        else
        {
           fprintf (stderr, "\rError: readmaps.dat and writmaps.dat "
                    "do not correspond in primary charset name.\n");
        }
    }
    destroy_charset_maps();
    xfree(fnr);
    xfree(fnw);
    return;
}

void destroy_charset_maps(void)
{
    if (readmaps!=NULL)
    {
        xfree(readmaps->tables);
        xfree(readmaps);
    }
    if (writemaps!=NULL)
    {
        xfree(writemaps->tables);
        xfree(writemaps);
    }
    readmaps=writemaps=NULL;
}



/* Find a lookup table. Note: NULL pointer means no translation has to be
   (or can be) done. */

LOOKUPTABLE *get_readtable (const char *charset_name, int level)
{
    int i;

    if (readmaps == NULL)
    {
        return NULL;
    }
    if (!strcmp(readmaps->charset_name, charset_name))
    {
        return NULL;  /* no translation necessary */
    }
    for (i = 0; i < readmaps->n_tables; i++)  /* find an ideal table */
    {
        if (readmaps->tables[i].level == level &&
            !strcmp(readmaps->tables[i].from_charset, charset_name) &&
            !strcmp(readmaps->tables[i].to_charset, readmaps->charset_name) )
        {
            return readmaps->tables+i;
        }
    }
    for (i = 0; i < readmaps->n_tables; i++)  /* find a table that at least */
    {                                         /* translates to 7 bit ASCII  */
        if (readmaps->tables[i].level == level &&
            !strcmp(readmaps->tables[i].from_charset, charset_name) &&
            !strcmp(readmaps->tables[i].to_charset, "ASCII") )
        {
            return readmaps->tables+i;
        }
    }
    return &maskout_table;  /* can't help - mask out all characters >= 128 */
}

LOOKUPTABLE * get_writetable(const char *charset_name, int *allowed)
{
    int i;

 
    if (writemaps == NULL)
    {
        *allowed=0;
        return NULL;
    }
    if (charset_name != NULL)
    {
        if (!strcmp(writemaps->charset_name, charset_name))
        {
            *allowed=1;
            return NULL;  /* no translation necessary */
        }
        for (i = 0; i < writemaps->n_tables; i++)  /* find an ideal table */
        {
            if (!strcmp(writemaps->tables[i].to_charset, charset_name) &&
                !strcmp(writemaps->tables[i].from_charset,
                        writemaps->charset_name) )
            {
                *allowed=1;
                return writemaps->tables+i;
            }
        }
    }
    for (i = 0; i < writemaps->n_tables; i++) /* find a to ASCII table */
    {
        if (!strcmp(writemaps->tables[i].to_charset, "ASCII")&&
            !strcmp(writemaps->tables[i].from_charset,
                    writemaps->charset_name))
        {
            *allowed = 0; /* don't write CHRS kludge if we translate
                             to ASCII anyway */
            return writemaps->tables+i;
        }
    }
    *allowed=0;
    return NULL;  /* can't help */
}

/* this routine filters out control codes that could break vt100 */

void strip_control_chars (char *text)
{
#if defined(UNIX) || defined(SASC)

    unsigned char c;
    size_t dstidx, len = strlen(text);

    for (dstidx = 0; dstidx < len; dstidx++)
    {
        c = *(unsigned char*)(text + dstidx);
        if ( (c < 32 && c != '\n' && c != '\r' && c != '\001') || 
            (c >= 128 && c < 160) )
        {
            text[dstidx] = '?';
        }
    }
#endif
}

char *translate_text (const char *text, LOOKUPTABLE *table)
{
    size_t orglength, maxlength;
    size_t srcidx = 0, dstidx = 0;
    char * translated;
    unsigned char tblidx;

    if (text == NULL)
    {
        return NULL;
    }

    translated = xmalloc((maxlength = orglength = strlen(text)) +1);
                       /* at first, we assume 1:1 translation */


    if (table == NULL) /* no translation necessary or possible */
    {
       if (maxlength != 0)
       {
          memcpy (translated, text, maxlength);
       }
       translated[maxlength] = 0;
       return translated;
    }

    for (srcidx=dstidx=0; srcidx < orglength; srcidx++)
    {
        if (dstidx>=maxlength)
        {
            translated=realloc(translated, maxlength += 40);
        }

        switch (table->level)
        {
        case 1:
            if (text[srcidx] & 0x80)   /* can't help here */
            {
                translated[dstidx++] = text[srcidx];
                continue;
            }
            tblidx=(unsigned char)text[srcidx] * 2;
            break; /* case */
        case 2:
            if (!(text[srcidx] & 0x80))   /* nothing to do here */
            {
                translated[dstidx++] = text[srcidx];
                continue;
            }
            tblidx=((unsigned char)text[srcidx]-128) * 2;
            break; /* case */
        default:  /* other levels are not implemented */
            translated[dstidx++] = text[srcidx];
            continue; /* for */
        }

        if (table->lookuptable[tblidx] >=0 &&
            table->lookuptable[tblidx] <=1)
        {
            if (table->lookuptable[tblidx+1])
            {
               translated[dstidx++] = table->lookuptable[tblidx+1];
            }
            continue;
        }

        if  (! (table->lookuptable[tblidx] >=2  &&   /* not a reserved */
                table->lookuptable[tblidx] <=32) )   /* character      */
        {
            translated[dstidx++] = table->lookuptable[tblidx];
            if (dstidx>=maxlength)
            {
               translated=realloc(translated, maxlength += 40);
            }
            translated[dstidx++] = table->lookuptable[tblidx+1];
            continue;
        }
    }

    if (dstidx>=maxlength)
    {
        translated=realloc(translated, maxlength += 40);
    }
    translated[dstidx++]=0;

    return translated;
}




