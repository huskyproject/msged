/*
 *  Msged support routines for Pacific C
 *
 *  Written by Paul Edwards and Matthew Parker and released
 *  to the public domain.
 */

#include <stddef.h>
#include <ctype.h>
#include <time.h>
#include <unixio.h>
#include <dos.h>

long int strtol(const char * nptr, char ** endptr, int base)
{
    long x        = 0;
    int undecided = 0;

    if(base == 0)
    {
        undecided = 1;
    }

    while(1)
    {
        if(isdigit(*nptr))
        {
            if(base == 0)
            {
                if(*nptr == '0')
                {
                    base = 8;
                }
                else
                {
                    base      = 10;
                    undecided = 0;
                }
            }

            x = x * base + (*nptr - '0');
            nptr++;
        }
        else if(isalpha(*nptr))
        {
            if((*nptr == 'X') || (*nptr == 'x'))
            {
                if((base == 0) || ((base == 8) && undecided))
                {
                    base      = 16;
                    undecided = 0;
                }
                else
                {
                    break;
                }
            }
            else
            {
                x = x * base + (toupper((unsigned char)*nptr) - 'A') + 10;
                nptr++;
            }
        }
        else
        {
            break;
        }
    }

    if(endptr != NULL)
    {
        *endptr = (char *)nptr;
    }

    return x;
} /* strtol */

int sopen(char * filename, unsigned int access, int flags, ...)
{
    int fp;

    fp = open(filename, access);

    if(fp == -1 && access == 1)
    {
        creat(filename, 0666);
        fp = open(filename, access);
    }

    return fp;
}

int bdos(int func, unsigned reg_dx, unsigned char reg_al)
{
    union REGS r;

    r.h.ah = func;
    r.h.al = reg_al;
    r.x.dx = reg_dx;
    int86(0x21, &r, &r);
    return r.x.ax;
}

/*
 *  Scalar date routines, public domain by Ray Gardner.
 *
 *  These will work over the range 1-01-01 thru 14699-12-31.  The
 *  functions written by Ray are isleap, months_to_days, years_to_days,
 *  ymd_to_scalar, scalar_to_ymd.  Modified slightly by Paul Edwards.
 */
static int isleap(unsigned yr)
{
    return yr % 400 == 0 || (yr % 4 == 0 && yr % 100 != 0);
}

static unsigned months_to_days(unsigned month)
{
    return (month * 3057 - 3007) / 100;
}

static long years_to_days(unsigned yr)
{
    return yr * 365L + yr / 4 - yr / 100 + yr / 400;
}

static long ymd_to_scalar(unsigned yr, unsigned mo, unsigned day)
{
    long scalar;

    scalar = day + months_to_days(mo);

    if(mo > 2)
    {
        /* adjust if past February */
        scalar -= isleap(yr) ? 1 : 2;
    }

    yr--;
    scalar += years_to_days(yr);
    return scalar;
}

time_t mktime(struct tm * timeptr)
{
    time_t tt;

    if(timeptr->tm_year < 70 || timeptr->tm_year > 120)
    {
        tt = (time_t)-1;
    }
    else
    {
        tt =
            ymd_to_scalar(timeptr->tm_year + 1900, timeptr->tm_mon,
                          timeptr->tm_mday) - ymd_to_scalar(
                1970,
                1,
                1);
        tt = tt * 24 + timeptr->tm_hour;
        tt = tt * 60 + timeptr->tm_min;
        tt = tt * 60 + timeptr->tm_sec;
    }

    return tt;
}
