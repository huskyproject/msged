#include <stdio.h>
#include <termios.h>

/* This program initialises the console in the same way as done by Msged, and
   then reads keystrokes and reports the codes byte by byte. */

static struct termios oldtios;

int main(void)
{
   struct termios tios;
   int i, nq=0;

   tcgetattr(0, &tios);
   oldtios = tios;
   tios.c_lflag &= ~ICANON;
   tios.c_lflag &= ~ECHO;
   tios.c_lflag &= ~ISIG;
   tios.c_cc[VMIN] = 0;
   tios.c_cc[VTIME] = 0;
   tcsetattr(0, TCSANOW, &tios);
   setbuf(stdin, NULL);

   printf ("%c[1J%c[1;1H\
This program reads the keyboard in the same way as Msged does internally. It\n\
will show you the codes that correspond to your keystrokes exactly as the\n\
terminal driver reports them to Msged.\n\
\n\
These key codes can NOT be used as arguments for ReadKey or EditKey. The\n\
arguments for ReadKey and EditKey are PC'ish BIOS keyboard codes. Msged has\n\
internal tables to translate from the UNIX terminal driver keycode\n\
sequences to a PC'ish BIOS keyboard code.\n\
\n\
This tool is used to debug Msged's keycode translation tables. If you\n\
experience a problem of the sort that you press a certain key, but Msged does\n\
not react to it like expected, then you can use this tool to find out the raw\n\
keycode sequence and then contact the author of Msged to request support for\n\
this particular keycode to be added.\n\
\n\
Please note that a single keystroke may result in more than one keycode.\n\
\n\
===> Press the \"q\" key three times in sequence to end this program. <====\n\
",27,27);

   do
   {
       i = getchar();
       if ( i != EOF )
       { 
           printf ("%3dd %2xh ",i,i);
           if (i>=32 && i <=127)
               printf ("\'%c\'", (char) i);
           printf ("\n");
           if (i == 'q' || i == 'Q')
               nq++;
           else
               nq=0;
       }
       else
       {
           clearerr(stdin);
       }
   } while (nq<3); 
   
   tcsetattr(0, 0, &oldtios);
   fflush(stdout);
   return 0;
}

