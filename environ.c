/*
 * ENVIRON.C
 *
 * Written 1998 by Tobias Ernst and release to the Public Domain
 *
 * Environment variable (%MAILBOXDRIVE% -> E: and so on) expander.
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "memextra.h"

#undef isalnum
char *env_expand(char *line)
{ char *cpTemp;
  char var[82]; char *cpvar, *cpsrc, *cpdest;
  int isVar=0, l=0, invar=0;
  size_t maxlen=2*strlen(line);

start:
  cpvar=var; cpsrc=line;
  cpTemp=xmalloc(maxlen+1);
  cpdest=cpTemp;

  while (*cpsrc)
  { if (*cpsrc=='%')
    {
      if (isVar)
      { *cpvar=0; isVar=0;
#ifndef UNIX
        /* On all systems except unix, environment variables are stored in
           uppercase alwys, but the user can access them via lowercase as
           well. As getenv is case-sensitive, we uppercase the variable
           name before passing it to getenv. */
        for (cpvar=var;*cpvar;cpvar++)
          *cpvar=toupper(*cpvar);
#endif
        if (!strlen(var))
        { *(cpdest++)='%'; if ((++l)>=maxlen) goto error; }
        else
            {
          if ((cpvar=getenv(var))!=NULL)
            for (;*cpvar;*(cpdest++)=*(cpvar++))
              if ((++l)>=maxlen) goto error;
            }
        cpvar=var;
      }
      else
      {
        isVar=1; invar = 0;
      }
    }
    else
    {
      if (isVar)
      { int novar = 0;
        
        *(cpvar++) = (*cpsrc); invar++;
        if (invar > 80)
        {
            novar = 1;
        }
        if ( ! ((*cpsrc >= 'A' && *cpsrc <= 'Z') ||
                (*cpsrc >= 'a' && *cpsrc <= 'z') ||
                (*cpsrc >= '0' && *cpsrc <= '9') ||
                (*cpsrc == '_')))
        {
            novar = 1;
        }
        if (novar)
        { isVar=0; *cpvar=0;
          *(cpdest++)='%';
          if ((++l)>=maxlen) goto error;
          for (cpvar=var;*cpvar;*(cpdest++)=*(cpvar++))
            if ((++l)>=maxlen) goto error;
          cpvar=var;
        } else;
      }
      else
      { *cpdest++=*cpsrc;
        if ((++l)>=maxlen) goto error;
      }
    }

    cpsrc++;
  }
  if (isVar)
  { isVar=0; *cpvar=0;
    *(cpdest++)='%';
    if ((++l)>=maxlen) goto error;
    for (cpvar=var;*cpvar;*(cpdest++)=*(cpvar++))
      if ((++l)>=maxlen) goto error;
    cpvar=var;
  }
  *cpdest=0;

  return cpTemp;

error: /* our estimate for the new buffer was too small */
  xfree(cpTemp);
  maxlen=maxlen+80;
  goto start;
}

