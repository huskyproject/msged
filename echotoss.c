/*
 *  ECHOTOSS.C
 *
 *  Written on 28-Jun-97 by Andrew Clarke and released to the public domain.
 *
 *  Code to append to Msged ECHOTOSS.LOG file.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "echotoss.h"
#include "strextra.h"

void echotoss_add(AREA * a)
{
    FILE * fp;
    char str[80];

    if(a == NULL || a->tag == NULL)
    {
        return;
    }

    if((a->local == 1) || (a->netmail == 1))
    {
        return;
    }

    fp = fopen(ST->echotoss, "r");

    if(fp != NULL)
    {
        while(fgets(str, sizeof str, fp) != NULL)
        {
            if(*str != '\0')
            {
                *(str + strlen(str) - 1) = '\0';
            }

            if(stricmp(str, a->tag) == 0)
            {
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    fp = fopen(ST->echotoss, "a");

    if(fp != NULL)
    {
        fprintf(fp, "%s\n", a->tag);
        fclose(fp);
    }
} /* echotoss_add */
