/*
 *  FREQ.C
 *
 *  Written on 14-Jan-98 by Kim Lykkegaard.
 *  Modified by Tobias Ernst.
 *  Released to the public domain.
 *
 *  Makes netmail filerequest from a msgbody.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if defined(__MSC__) || defined(OS216)
#include <sys/types.h>
#include <sys/timeb.h>
#include <direct.h>
#endif

#if defined(MSDOS) && defined(__TURBOC__)
#include <dir.h>
#endif

#ifdef __WATCOMC__
#if defined(MSDOS) || defined(__NT__)
#include <direct.h>
#endif
#endif

#ifdef PACIFIC
#include <sys.h>
int bdos(int func, unsigned reg_dx, unsigned char reg_al);
#endif

#if defined(MSDOS) || (defined(__NT__) && !defined(__MINGW32__))
#include <dos.h>
#endif

#ifdef OS2
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#if defined(UNIX) || defined(__DJGPP__)
#include <unistd.h>
#endif

#include "addr.h"
#include "nedit.h"
#include "msged.h"
#include "winsys.h"
#include "menu.h"
#include "main.h"
#include "memextra.h"
#include "strextra.h"
#include "screen.h"
#include "config.h"
#include "keys.h"
#include "version.h"
#include "readmail.h"
#include "dialogs.h"
#include "help.h"
#include "makemsgn.h"
#include "flags.h"
#include "mctype.h"

/*
 *  Define record for the FileRequest list.
 */

typedef struct _freq
{
    int status;   /* is this file selected or not */
    char *file;    /* file name */
    char *desc;    /* Description for the file */
}
FREQ;


static char *get_desc(char *txt)
{
    char *cp = txt;

    do
    { 
        for(; m_isspace(*cp) && *cp; cp++);    /* skip white spaces  */

        if ( (!m_isdigit(*cp)) &&              /* not a file size    */
             (*cp != '[' || *cp != '(') &&   /* not a file counter */
             (stricmp(cp,"KB"))              /* not a "KB" unit    */
           )
        {
            break;
        }

        for(; (!m_isspace(*cp)) && *cp; cp++); /* skip this word     */
        
    } while (*cp);

    return xstrdup(cp);
}


void makefreq(void)
{
    EVT  event;
    msg *m = NULL, *n = NULL;
    LINE *line = NULL, *curr = NULL, *l2;
    FREQ *freq = NULL;
    ADDRESS from;
    char file[256], txt[236];
    char *desc;
    int k = 0;                   /* UMSGID msgnum */
    int o = 0;                   /* old UMSGID msgnullm */
    unsigned long length;        /* length in bytes of the message */
    unsigned long now = 0L;     /* UMSGID msgnum */
    int i, l=0, y, oy, pos, old_pos, Msg;
    int max = maxy - 9, maxn = maxy - 9, redraw, starty, done = 0;
    int num_freq = 0, gotdot = 0;
    WND *hCurr, *hWnd = NULL, *hWnd2 = NULL;
    
    now = sec_time();
    
    n = readmsg(CurArea.current);
    if ((m = readmsg(CurArea.current)) != NULL)
    {
        line = m->text;
        while (line->next != NULL)
        {
            l = 0;
            strcpy(txt, line->text);
            txt[20] = '\0';
            if ((strchr(txt, '.')) && ((!strstr(txt, "---"))
                                       && (!strstr(txt, "..."))))
            {
                while(txt[0] == ' ')
                {
                    memmove(txt, &txt[1], strlen(txt));
                    l++;
                    if (strlen(txt)== 0) /* this can't happen */
                    {
                        break;
                    }
                }

                for(i=0; i<strlen(txt); i++)
                {
                    file[i]=txt[i];
                    file[i+1]='\0';
                    if ((txt[i] == '.') && (!gotdot))
                    {
                        gotdot=1;
                    }
                    if ((txt[i]==' ') && (gotdot))
                    {
                        file[i]='\0';
                        l=l+i+1;
                        break;
                    }
                }
                strcpy(txt, &line->text[l]);
                while(txt[0]==' ')
                {
                    memmove(txt, &txt[1], strlen(txt));
                    l++;
                    if (strlen(txt)== 0)
                    {
                        txt[0]='\0';
                        break;
                    }
                }
                i=strlen(txt)-1;
                while((txt[i]==' ') || (txt[i]=='\n') || (txt[i]=='\r'))
                {
                    txt[i]='\0';
                    i--;
                    if (i<=0)
                    {
                        break;
                    }
                }

                desc = get_desc(txt);

                if ((!strchr(file, ' ')) && /* If space in file */
                    /* name - drop it   */
                    (file[strlen(file)-1] != '.') && /* If it ends on .  */
                    /* it isn't a file  */
                    (strchr(file, '.')))
                {
                    freq = xrealloc(freq, (++num_freq) * sizeof(struct _freq));
                    freq[num_freq - 1].file = xstrdup(file);
                    freq[num_freq - 1].desc = desc;
                    freq[num_freq - 1].status = 0;
                }
            }
            line = line->next;
        }
        freq = xrealloc(freq, (++num_freq) * sizeof(struct _freq));
        freq[num_freq - 1].file = xstrdup("FILES");
        sprintf(txt, "Filelist from %s", m->isfrom);
        freq[num_freq - 1].desc = xstrdup(txt);
        freq[num_freq - 1].status = 0;

        hCurr = WndTop();

        if (num_freq < maxn)
        {
            maxn=num_freq;
        }

        y = 0; oy = 0; pos=0; old_pos=0; redraw=1; starty=0;

        hWnd2 = WndOpen(0, 6, 79, maxn - starty + 7, SBDR,
                        cm[LS_BTXT], cm[LS_NTXT]);
        sprintf(txt, " File Request from %s, %s ", m->isfrom,
                show_address(&m->from));
        WndTitle(txt, cm[LS_NTXT]); 

        while(!done)
        {
            if(redraw)
            {
                if (redraw > 1) /* the frame gets larger */
                {
                    if (hWnd2 != NULL)
                    {
                        WndClose(hWnd2);
                    }
                    hWnd2 = WndOpen(0, 6, 79, maxn - starty + 7, SBDR,
                                    cm[LS_BTXT], cm[LS_NTXT]);
                    sprintf(txt, " File Request from %s, %s ", m->isfrom,
                            show_address(&m->from));
                    WndTitle(txt, cm[LS_NTXT]);
                }
                                   
                if (hWnd != NULL)
                {
                    WndClose(hWnd);
                }
                hWnd = WndOpen(0, 6, 79, maxn - starty + 7,
                               SBDR | NOSAVE,
                               cm[LS_BTXT], cm[LS_NTXT]);
                sprintf(txt, " File Request from %s, %s ", m->isfrom,
                        show_address(&m->from));
                WndTitle(txt, cm[LS_NTXT]); 
                WndClear(0, 0, 76, maxn - starty - 1, cm[LS_BTXT]);

                for (i = starty; i < maxn; i++)
                {
                    l=strlen(freq[i].file)+1;
                    if (l<15)
                    {
                        l=15;
                    }
                    if (strlen(freq[i].desc) > 60)
                    {
                        freq[i].desc[60]='\0';
                    }
                    WndGotoXY(1, i-starty);
                    if (freq[i].status != 0)
                    {
                        WndPutc('+', cm[LS_NTXT]);
                    }
                    WndWriteStr(2, i-starty, cm[LS_NTXT], freq[i].file);
                    WndWriteStr(2+l,i-starty, cm[LS_NTXT], freq[i].desc);
                }
                redraw=0;
            }
            if (oy != y)
            {
                WndWriteStr(2, oy, cm[LS_NTXT], freq[old_pos].file);
            }
            WndWriteStr(2, y, cm[LS_STXT], freq[pos].file);
            oy=y;
            old_pos=pos;
            
            Msg = MnuGetMsg(&event, hWnd->wid);
            
            switch (event.msgtype)
            {
            case WND_WM_CHAR:
                switch (Msg)
                {
                case Key_Up:
                    if (y>0)
                    {
                        y--;
                        pos--;
                    }
                    else
                    {
                        if ((starty>0) && (y==0))
                        {
                            starty--;
                            maxn--;
                            pos--;
                            redraw=1;
                            y=0;
                            oy=0;
                        }
                    }
                    break;
                    
                case Key_Dwn:
                    if (y<maxn-1-starty)
                    {
                        y++;
                        pos++;
                    }
                    else
                    {
                        if (maxn<num_freq)
                        {
                            starty++;
                            maxn++;
                            pos++;
                            redraw = 1;
                        }
                    }
                    break;
                    
                case Key_PgUp:
                    if (starty>0)
                    {
                        starty=starty-max;
                        maxn=maxn-max;
                        pos=pos-max;
                        if (starty<0)
                        {
                            starty=0;
                            maxn=num_freq;
                            if (maxn>max)
                            {
                                maxn=max;
                            }
                            pos=0;
                        }
                        y=0;
                        oy=0;
                        redraw=1;
                    }
                    break;
                    
                case Key_PgDn:
                    if (maxn<num_freq)
                    {
                        maxn=maxn+max;
                        starty=starty+max;
                        if (maxn>num_freq)
                        {
                            maxn=num_freq;
                            starty=maxn-max;
                        }
                        pos=starty;
                        oy=0;
                        y=0;
                        redraw=1;
                    }
                    break;
                    
                case Key_Home:
                    oy = 0;
                    y = 0;
                    pos = 0;
                    starty = 0;
                    maxn = maxy-9;
                    if (num_freq < maxn)
                    {
                        maxn=num_freq;
                    }
                    redraw = 1;
                    break;
                    
                case Key_A_I:
                case Key_Ins:
                    file[0]='\0';
                    gotdot =
                        GetString(" Add a File ",
                                  "Enter the filename to add to the list:",
                                  file, 40);
                    file[31]='\0';
                    strupr(file);
                    
                    if (gotdot)
                    {
                        
                        freq = xrealloc(freq,
                                        (++num_freq) * sizeof(struct _freq));
                        freq[num_freq - 1].file = xstrdup(file);
                        freq[num_freq - 1].desc = xstrdup("No description.");
                        freq[num_freq - 1].status = 1;
                        if (maxn < max)
                        {
                            maxn++;
                        }
                    }
                    else    /* user did not at a new file */
                    {
                        break;
                    }
                    redraw=1;
                case Key_End:
                    if (maxn<num_freq)
                    {
                        maxn=num_freq;
                        starty=maxn-max;
                        pos=num_freq-1;
                        y=maxn-starty-1;
                        oy=maxn-starty-1;
                    }
                    else
                    {
                        oy=maxn-starty-1;
                        y=maxn-starty-1;
                        pos=maxn-1;
                    }
                    redraw++;
                    break;
                    
                case Key_Spc:
                    freq[pos].status = !freq[pos].status;
                    WndGotoXY(1, y);
                    if (freq[pos].status != 0)
                    {
                        WndPutc('+', cm[LS_NTXT]);
                    }
                    else
                    {
                        WndPutc(' ', cm[LS_NTXT]);
                    }
                    if (y<maxn-starty-1)
                    {
                        y++;
                        pos++;
                    }
                    else
                    {
                        if (maxn<num_freq-1)
                        {
                            starty++;
                            maxn++;
                            pos++;
                            redraw=1;
                        }
                    }
                    break;
                case Key_A_X:
                case Key_Esc:
                    done=1;
                    break;
                case Key_F1:
                case Key_A_H:
                    if (ST->helpfile != NULL)
                    {
                        DoHelp(4);
                    }
                    break;
                case Key_Ent:
                    done=2;
                    break;
                }
                break;
            default:
                break;
            }
            
        }
        
        if (hWnd2 != NULL)
        {
            WndClose(hWnd2);
        }
        if (hWnd != NULL)
        {
            WndClose(hWnd);
        }
        WndCurr(hCurr);
        
        if ((done==1) || ((done==2) && (num_freq<1)))
        {
            for(i=0; i<num_freq; i++)
            {
                if (freq[i].file==NULL)
                {
                    break;
                }
                xfree(freq[i].file);
                if (freq[i].desc==NULL)
                {
                    break;
                }
                xfree(freq[i].desc);
            }
            return;
        }
        
        o=SW->area;
        
        gotdot=0;           /* reuse not nice, but save a int */
        if (ST->freqarea != NULL)
        {
            for (k = 0; k < SW->areas; k++)
            {
                if (!stricmp(ST->freqarea, arealist[k].tag)) /* FIXME */
                {
                    gotdot=1;
                    break;
                }
            }
        }
        
        if (gotdot)
        {
            set_area(k);        /* k er en int for area nr. */
            m->msgnum = MsgnToUid(CurArea.messages);
                
            from=m->from;
                
            xfree(m->text);
            m->text=NULL;
                
            if (SW->msgids)
            {
	        sprintf(txt, "\01MSGID: %s %08lx\r", show_address(&from), now);
	        now++;
                curr = InsertAfter(curr, txt);
            }
                
            if (curr != NULL)
            {
                curr->next = m->text;
                if (m->text != NULL)
                {
                    m->text->prev = curr;
                }
                    
                while (curr->prev != NULL)
                {
                    curr = curr->prev;
                }
                    
                m->text = curr;
            }
                
            l2 = m->text;
            length=0L;
            while (l2)
            {
                length += strlen(l2->text);
                l2 = l2->next;
            }
                
            txt[0]='\0';
            for(i=0; i<num_freq; i++)
            {
                if (freq[i].status != 0)
                {
                    strcat(txt, freq[i].file);
                    strcat(txt, " ");
                }
                if (strlen(txt) > 55) /* check subj isn't too big */
                {
                    txt[strlen(txt)-1]='\0'; /* kill last space */
                    m->msgnum++;
                        
                        
                    m->isto = xstrdup(n->isfrom);
                    xfree(m->isfrom);
                    m->isfrom = NULL;
                    m->isfrom = xstrdup(ST->username);
                    m->to = n->from;
                    if (n->from.domain)
                    {
                        m->to.domain = xstrdup(CurArea.addr.domain);
                    }
                    m->from = CurArea.addr;
                    if (CurArea.addr.domain)
                    {
                        m->from.domain = xstrdup(CurArea.addr.domain);
                    }
                    m->timestamp = time(NULL);
                    m->replyto = 0;
                    m->new = 1;
                    m->cost = 0;
                    m->times_read = 0;
                    m->scanned = 0;
                    m->soteot = 0;
                    m->time_arvd = 0;
                    clear_attributes(&m->attrib);
                    if (ST->freqflags != NULL)
                    {
                        parseflags(ST->freqflags, m);
                    }
                    else
                    {
                        m->attrib.killsent = 1;
                        m->attrib.direct = 1;
                    }
                    m->attrib.freq = 1;
                    m->subj=xstrdup(txt);
                        
                        
                    CurArea.last++;
                    writemsg(m);
                    txt[0]='\0';
                }
            }
            if (txt[0] != '\0')
            {
                txt[strlen(txt)-1]='\0'; /* kill last space */
            }
                
            m->isto = xstrdup(n->isfrom);
            xfree(m->isfrom);
            m->isfrom = NULL;
            m->isfrom = xstrdup(ST->username);
            m->to = n->from;
            if (n->from.domain)
            {
                m->to.domain = xstrdup(n->from.domain);
            }
            m->from = CurArea.addr;
            if (CurArea.addr.domain)
            {
                m->from.domain = xstrdup(CurArea.addr.domain);
            }
            m->timestamp = time(NULL);
            m->replyto = 0;
            m->new = 1;
            m->cost = 0;
            m->times_read = 0;
            m->scanned = 0;
            m->soteot = 0;
            m->time_arvd = 0;
            clear_attributes(&m->attrib);
            if (ST->freqflags != NULL)
            {
                parseflags(ST->freqflags, m);
            }
            else
            {
                m->attrib.killsent = 1;
                m->attrib.direct = 1;
            }
            m->attrib.freq = 1;
            m->subj=xstrdup(txt);
                
            if (strlen(txt)!=0)
            {
                CurArea.last++;
                m->msgnum++;
                writemsg(m);
            }
        }
        else         /* end gotdot (found a area to make req. in) */
        {
            if (ST->freqarea == NULL)
            {
                ChoiceBox("", "No file request area defined. "
                          "Use the FreqArea keyword.", "  Ok  ", NULL, NULL);
            }
            else
            {
                sprintf (txt, "FreqArea %s does not exist!", ST->freqarea);
                ChoiceBox("", txt, "  Ok  ", NULL, NULL);
            }
        }
    }
    
    dispose(m);
    dispose(n);
    
    
    for(i=0; i<num_freq; i++)
    {
        if (freq[i].file==NULL)
        {
            break;
        }
        xfree(freq[i].file);
        if (freq[i].desc==NULL)
        {
            break;
        }
        xfree(freq[i].desc);
    }
    
    set_area(o);
}
