/*
 *  MAKEMAPS.C
 *
 *  Written 1998 by Tobias Ernst. Released to the Public Domain.
 *
 *  This program generates the READMAPS.DAT and WRITMAPS.DAT files required
 *  by MsgEd to do proper FSC-0054 character set translation.
 *
 *  Warning: This is spaghetti code. You normally don't have to use the program
 *  anyway (I do supply ready-made binary map files), and you surely don't have
 *  to mess around with this source code. :-)
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <huskylib/huskylib.h>

extern int unlink(const char *);

typedef struct _lookuptable
{
    unsigned long  id;
    unsigned short modrev;
    unsigned short level;
    char           from_charset[9];
    char           to_charset[9];
    char           lookuptable[256];
} LOOKUPTABLE;

typedef struct _readwritemap
{
    char           charset_name[9];
    int            n_tables;
    LOOKUPTABLE   *tables;
} READWRITEMAPS;


/* global variables */

READWRITEMAPS readmaps, writmaps;


/*
 * converts a string to upper case
 *
 */

char* upcase(char *p)
{
    char *o = p;
    for (;*p;p++)
    {
        *p = toupper(*p);
    }
    return o;
}


/*
 * malloc and realloc routines with built-in check for memory exhaustion.
 *
 */

void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        fprintf (stderr, "OUT OF MEMORY\n");
        abort();
    }
    return ptr;
}

char *xstrdup(const char *string)
{
    char *ptr = strdup(string);
    if (ptr == NULL)
    {
        fprintf (stderr, "OUT OF MEMORY\n");
        abort();
    }
    return ptr;
}

void *xrealloc(void *oldptr, size_t size)
{
    void *ptr = realloc(oldptr, size);
    if (ptr == NULL)
    {
        fprintf (stderr, "OUT OF MEMORY\n");
        abort();
    }
    return ptr;
}


/*
 * initialise the global variables
 *
 */

void init(char *charset_name)
{
    size_t len = strlen(charset_name);

    if (len>8)
    {
        len = 8;
    }

    memset(&readmaps, '\0', sizeof(readmaps));
    memset(&writmaps, '\0', sizeof(writmaps));

    memmove(readmaps.charset_name, charset_name, len);
    memmove(writmaps.charset_name, charset_name, len);
    upcase(readmaps.charset_name);
    upcase(writmaps.charset_name);
}


/*
 * print a short help text
 *
 */

void usage(void)
{
    printf ("Usage:\n\n");
    printf ("        MAKEMAPS <charset-name> <chs-file> ...\n\n");
    printf ("Where:\n\n");
    printf ("        <charset-name> is the name of the charset this\n");
    printf ("                       mapping file is for.\n");
    printf ("        <chs-file>     is the name of a source file in\n");
    printf ("                       .chs format defining a translation\n");
    printf ("                       table. This parameter can and\n");
    printf ("                       should be repeated.\n\n");
    printf ("Example:\n\n");
    printf ("         MAKEMAPS IBMPC IBM_ISO.CHS ISO_IBM.CHS IBM_ASC.CHS ASC_IBM.CHS\n\n");
    printf ("On a UNIX-like shell with shell expansions, you can simply use:\n\n");
    printf ("         ./makemaps LATIN-1 *.CHS *.chs\n");
}


/*
 * Extract the first and second word from a line.
 *
 */

void parse_line (char *string, char **word1, char **word2)
{
    int word = 0;
    char *dup = xstrdup(string);
    char *cp;
    int noword = 1;

    *word1 = NULL;
    *word2 = NULL;

    if (*string == ';')
    {
        return;
    }

    for (cp = dup; *cp; cp++)
    {
        if (isspace(*cp))
        {
            *cp = '\0';
            noword = 1;
            if (word == 2)
            {
                break;
            }
        }
        else
        {
            if (noword)
            {
                noword = 0;
                if (!word)
                {
                    *word1 = cp;
                    word = 1;
                }
                else
                {
                    *word2 = cp;
                    word = 2;
                }
            }
        }
    }
    if (*word1 != NULL)
    {
        *word1 = xstrdup(*word1);
    }
    if (*word2 != NULL)
    {
        *word2 = xstrdup(*word2);
    }
    nfree(dup);
}


/*
 * parse_char parses a "character description" and returns the corresponding
 * unsigned character. A character description must be either
 *
 *     - a single character or
 *     - an escape sequence introduced with a backslash. The following escape
 *        sequences are supported:
 *        \\    -> a single backslash
 *        \dnnn -> decimal notation of an ascii code
 *        \xnn  -> hexadecimal notation of an ascii code
 *
 * examples:
 *
 *     0
 *     \d48
 *     \x30
 *
 * all refer to the same character.
 *
 * If parse_char returns 256, it means that an error occured. The filename
 * parameter is passed to parse_char for error logging purposes only.
 *
 */

unsigned int parse_char(char *cp, char *filename)
{
    unsigned char *ucp = (unsigned char *)cp;
    unsigned int rv = 0;

    if (*ucp == '\\')
    {
        switch (*(++ucp))
        {
        case 'd':
            for (++ucp; *ucp; ++ucp)
            {
                if (!isdigit(*ucp))
                {
                    printf ("%s: Error: Cannot parse character \"%s\".\n",
                            filename, cp);
                    return 256;
                }
                rv = rv * 10 + (*ucp - '0');
            }
            break;
        case 'x':
            for (++ucp; *ucp; ++ucp)
            {
                if (!isxdigit(*ucp))
                {
                    printf ("%s: Error: Cannot parse character \"%s\".\n",
                            filename, cp);
                    return 256;
                }
                if (isdigit(*ucp))
                {
                    rv = rv * 16 + (*ucp - '0');
                }
                else
                {
                    rv = rv * 16 + (toupper(*ucp) - 'A' + 10);
                }
            }
            break;
        case '\\':
            return '\\';
            break;
        case '0':
            return 0;
            break;
        default:
            printf ("%s: Error: Cannot parse character \"%s\".\n",
                    filename, cp);
            return 256;
        }
        if (rv >= 256)
        {
            printf ("%s: Error: Cannot parse character \"%s\".\n",
                    filename, cp);
            return 256;
        }
        else
        {
            return rv;
        }
    }
    else
    {
        return *ucp;
    }
}


/*
 * The process() function reads in a .CHS file, parses it, and creates a
 * corresponding LOOKUPTABLE entry in the tables array of either the readmaps
 * or the writmaps variable.
 *
 */

int process(char *filename)
{
    FILE *f = fopen(filename, "r");
    int pos;
    char buf[256];
    char *word1, *word2;
    LOOKUPTABLE ltable;
    READWRITEMAPS *maps = NULL;
    unsigned int tmp;

    if (f == NULL)
    {
        printf ("%-15s: Not found.\n", filename);
        return 0;
    }

    pos = 0;

    while ((!feof(f)) && (pos <= 133))
    {
        if (fgets(buf, sizeof(buf) - 1, f) != NULL)
        {
            parse_line(buf, &word1, &word2);
            if (word1 == NULL)
            {
                continue;
            }
            switch (pos)
            {
            case 0:
                ltable.id = atoi(word1);
                break;
            case 1:
                ltable.modrev = atoi(word1);
                break;
            case 2:
                ltable.level = atoi(word1);
                if (ltable.level < 1 || ltable.level > 2)
                {
                    printf ("%s: ERROR: Charset level not 1 or 2.\n", filename);
                    fclose(f);
                    return 0;
                }
                break;
            case 3:
                if (strlen(word1) > 8)
                {
                    printf ("%s: ERROR: Charset name longer "
                            "than 8 characters.\n", filename);
                    fclose(f);
                    return 0;
                }
                strcpy(ltable.from_charset, word1);
                upcase(ltable.from_charset);
                break;
            case 4:
                if (strlen(word1) > 8)
                {
                    printf ("%s: ERROR: Charset name longer "
                            "than 8 characters.\n", filename);
                    fclose(f);
                    return 0;
                }
                strcpy(ltable.to_charset, word1);
                upcase(ltable.to_charset);
                if (!strcmp(ltable.from_charset, writmaps.charset_name))
                {
                    maps = &writmaps;
                    printf ("%s: Using for WRITMAPS.DAT.\n", filename);
                }
		else
                if ((!strcmp(ltable.to_charset, readmaps.charset_name)) ||
                   (!strcmp(ltable.to_charset, "ASCII")))
                {
                    maps = &readmaps; /* read */
                    printf ("%s: Using for READMAPS.DAT.\n", filename);
                }
                else
                {
                    printf ("%s: Skipping (no matching charset).\n", filename);
                    fclose(f);
                    return 1;
                }
                break;
            case 133:
                if (strcmp(upcase(word1), "END"))
                {
                    printf ("%s: ERROR: Expected END statement not found.\n",
                            filename);
                    fclose(f);
                    return 0;
                }
                break;
            default:
                if ((tmp = parse_char(word1, filename)) >= 256)
                {
                    fclose(f);
                    return 0;
                }
                if (word2 == NULL)
                {
                    printf ("%s: ERROR: Syntax error.\n", filename);
                    fclose(f);
                    return(0);
                }
                ltable.lookuptable[2*(pos-5)] = tmp;
                if ((tmp = parse_char(word2, filename)) >= 256)
                {
                    fclose(f);
                    return 0;
                }
                ltable.lookuptable[(2*(pos-5)) + 1] = tmp;
                break;
            }
            pos++;
            nfree(word1);
            if (word2 != NULL)
            {
                nfree(word2);
            }
        }
    }
    if (maps->tables == NULL)
    {
        maps->tables = malloc(sizeof(LOOKUPTABLE) * (maps->n_tables + 1));
    }
    else
    {
        maps->tables = realloc(maps->tables,
                                sizeof(LOOKUPTABLE) * (maps->n_tables + 1));
    }
    memmove(maps->tables + maps->n_tables, &ltable, sizeof(LOOKUPTABLE));
    maps->n_tables++;
    fclose(f);
    return 1;
}


/*
 * save_table writes a given lookup table to disk.
 *
 */

int save_table(FILE *f, LOOKUPTABLE *pltable)
{
    unsigned char raw[288];
    unsigned int i;

    memset(raw, '\0', sizeof(raw));
    raw[4] = 1; /* module revision */
    raw[6] = pltable->level & 0xFF;
    raw[7] = (pltable->level >> 8) & 0xFF;
    memmove(raw + 16, pltable->from_charset, strlen(pltable->from_charset));
    memmove(raw + 24, pltable->to_charset, strlen(pltable->to_charset));

    for (i=0; i<256; i++)
    {
        raw[i+32] = pltable->lookuptable[i];
    }

    return (fwrite(raw, sizeof(raw), 1, f) == 1);
}


/*
 * sort_read is a qsort helper function for sorting the readmaps,
 * sort_write is a qsort helper function for sorting the writmaps.
 *
 */

int sort_read(const void *p1, const void *p2)
{
    const LOOKUPTABLE *pl1 = p1, *pl2 = p2;
    int i;
    if (pl1->level < pl2->level)
    {
        return -1;
    }
    if (pl1->level > pl2->level)
    {
        return 1;
    }
    if ((i = strcmp(pl1->from_charset, pl2->from_charset)) != 0)
    {
        return i;
    }
    if (!strcmp(pl1->from_charset, "ASCII"))
    {
        return 1;
    }
    else if (!strcmp(pl2->from_charset, "ASCII"))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int sort_writ(const void *p1, const void *p2)
{
    const LOOKUPTABLE *pl1 = p1, *pl2 = p2;

    if (pl1->level < pl2->level)
    {
        return -1;
    }
    if (pl1->level > pl2->level)
    {
        return 1;
    }
    return (strcmp(pl1->to_charset, pl2->to_charset));
}


/*
 * save writes the whole thing to disk
 *
 */

int save(void)
{
    FILE *fr = fopen("readmaps.dat","wb");
    FILE *fw = fopen("writmaps.dat","wb");
    unsigned char header[12];
    int i;

    if (fr == NULL)
    {
        fprintf (stderr, "Cannot write readmaps.dat.\n");
        goto erro;
    }
    if (fw == NULL)
    {
        fprintf (stderr, "Cannot write writmaps.dat.\n");
        goto erro;
    }

    memset(header, '\0', sizeof(header));
    header[0] = 1;  /* We always write a little endian style file. MsgEd TE
                       will be able to read this even on a big endian machine,
                       and we are able to create it even on a big endian
                       machine, so there is no problem. */
    memmove(header+4, readmaps.charset_name, strlen(readmaps.charset_name));
    if (fwrite(header, sizeof(header), 1, fr) != 1)
    {
        goto erro;
    }
    memset(header + 4, '\0', sizeof(header) - 4);
    memmove(header+4, writmaps.charset_name, strlen(writmaps.charset_name));
    if (fwrite(header, sizeof(header), 1, fw) != 1)
    {
        goto erro;
    }

    qsort(readmaps.tables, readmaps.n_tables, sizeof(LOOKUPTABLE), sort_read);
    for (i=0; i<readmaps.n_tables; i++)
    {
        if (!save_table(fr, readmaps.tables + i))
        {
            goto erro;
        }
    }

    qsort(writmaps.tables, writmaps.n_tables, sizeof(LOOKUPTABLE), sort_writ);
    for (i=0; i<writmaps.n_tables; i++)
    {
        if (!save_table(fw, writmaps.tables + i))
        {
            goto erro;
        }
    }

    fclose(fw);
    fclose(fr);
    return 1;

erro:
    fprintf (stderr, "File I/O error.\n");
    if (fw == NULL || fr == NULL)
    {
        if (fw != NULL)
        {
            fclose(fw);
        }
        if (fr != NULL)
        {
            fclose(fr);
        }
    }
    unlink("readmaps.dat");
    unlink("writmaps.dat");
    return 0;
}

int main(int argc, char **argv)
{
    int i;

    if (argc < 3)
    {
        usage(); return 8;
    }
    if (strlen(argv[1]) > 8)
    {
        fprintf (stderr,
                 "Error: Character set name is longer than 8 characters.\n");
        return 8;
    }

    init(argv[1]);
    for (i = 2; i < argc; i++)
    {
        if (!process(argv[i]))
        {
            return 8;
        }
    }

    i = save();

    if (readmaps.tables != NULL)
    {
        nfree(readmaps.tables);
    }
    if (writmaps.tables != NULL)
    {
        nfree(writmaps.tables);
    }
    return (!i) ? 8 : 0;
}




