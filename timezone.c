/*
 *  TIMEZONE.C
 *
 *  Written on 11-Apr-01 by Tobias Ernst and released to the public domain.
 *
 *  Based on a suggestion by Jesper Soerensen
 *
 *  Routines for portably querying the local timezone as well as for
 *  converting dates between different time zones.
 */

#include <time.h>
#include <stdlib.h>             /* NULL */
#include <huskylib/compiler.h>
#include "timezone.h"
/* guess our time zone */
int tz_my_offset(void)
{
    time_t now;
    struct tm * tm;
    int gm_minutes;
    long gm_days;
    int local_minutes;
    long local_days;

    tzset();
    now           = time(NULL);
    tm            = localtime(&now);
    local_minutes = tm->tm_hour * 60 + tm->tm_min;
    local_days    = (long)tm->tm_year * 366L + (long)tm->tm_yday;
    tm            = gmtime(&now);
    gm_minutes    = tm->tm_hour * 60 + tm->tm_min;
    gm_days       = (long)tm->tm_year * 366L + (long)tm->tm_yday;

    if(gm_days < local_days)
    {
        local_minutes += 1440;
    }
    else if(gm_days > local_days)
    {
        gm_minutes += 1440;
    }

    return local_minutes - gm_minutes;
} /* tz_my_offset */

#if 0
/* adds offset minutes to this date */
void adjust_date(struct _stamp * pdate, int offset)
{
    int minutes = pdate->time.mm * 60 + pdate->time.hh;

    minutes += offset;

    while(minutes >= 24 * 60)
    {
        day++; /* not yet implemented */
        minutes -= 24 * 60;
    }

    while(minutes < 0)
    {
        day--; /* not yet implemented */
        minutes += 24 * 60;
    }
    pdate->time.mm = minutes % 60;
    pdate->time.hh = minutes / 60;
}

#endif
