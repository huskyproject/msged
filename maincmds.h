/*
 *  MAINCMDS.H
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 */

#ifndef __MAINCMDS_H__
#define __MAINCMDS_H__

static void left(void);
static void right(void);
static void go_last(void);
static void link_to(void);
static void link_from(void);
static void view(void);
static void go_root(void);
static void go_back(void);
static void go_dos(void);
static void rundos(void);
static void search(void);
static void hdrsearch(void);
static void spmail(void);
static void pmail(void);
static void gotomsg0(void);
static void quit(void);
static void first(void);
static void astart(void);
static void slast(void);
static void next_area(void);
static void prev_area(void);
static void scan_areas(int);
static void al_scan_areas(int);
static void go_next(void);
static void edithdr(void);
static void nada(void);

void makefreq(void);
void uudecode(void);
void hex_dump(void);
void newmsg(void);
void reply(void);
void quote(void);
void reply_oarea(void);
void replyextra(void);
void followup(void);
void change(void);
void Go_Dwn(void);
void Go_Up(void);

static void delete(void);
static void move(void);
static void outtxt(void);
static void chngaddr(void);
static void chngname(void);
static void chngnodel(void);
static void do_help(void);
static void set(void);
static void list(void);
static void rotate(void);

void (*mainckeys[256]) (void) =
{
    NULL, NULL, NULL, NULL, uudecode, NULL, makefreq, NULL,      /* 0 */
    edithdr, go_next, NULL, NULL, NULL, right, chngnodel, NULL,  /* 8 */
    NULL, NULL, NULL, NULL, NULL, chngname, NULL, chngaddr,      /* 10 */
    NULL, NULL, NULL, quit, NULL, NULL, NULL, NULL,              /* 18 */
    NULL, rundos, NULL, scan_unscanned_areas, NULL, NULL, NULL, NULL,  /* 20 */
    NULL, NULL, scan_all_areas, next_area, NULL, prev_area, NULL, NULL,/* 28 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 30 */
    NULL, NULL, NULL, NULL, rotate, NULL, rotate, NULL,       /* 38 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 40 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 48 */
    spmail, NULL, NULL, NULL, NULL, NULL, NULL, NULL,         /* 50 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 58 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 60 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 68 */
    spmail, NULL, NULL, NULL, NULL, NULL, NULL, NULL,         /* 70 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 78 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 80 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 88 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 90 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 98 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* A0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* A8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* B0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* B8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* C0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* C8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* D0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* D8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* E0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* E8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* F0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL            /* F8 */
};

void (*mainakeys[256]) (void) =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 8 */
    quote, outtxt, newmsg, reply, sel_chs, replyextra, followup, NULL, /* 10 */
    go_dos, pmail, NULL, NULL, NULL, NULL, quit, set,         /* 18 */
    delete, search, gotomsg0, do_help, NULL, NULL, list, NULL,  /* 20 */
    NULL, NULL, NULL, NULL, hdrsearch, quit, change, view,      /* 28 */
    hex_dump, reply_oarea, move, NULL, NULL, NULL, NULL, NULL,  /* 30 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 38 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, first,          /* 40 */
    NULL, NULL, NULL, left, NULL, right, NULL, slast,         /* 48 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 50 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 58 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 60 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 68 */
    NULL, NULL, NULL, link_from, link_to, NULL, NULL, NULL,   /* 70 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 78 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 80 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 88 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 90 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 98 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* A0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* A8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* B0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* B8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* C0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* C8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* D0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* D8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* E0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* E8 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* F0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL            /* F8 */
};

const struct _command maincmds[] =
{
    {"next_area", next_area},
    {"previous", left},
    {"last", go_last},
    {"link_to", link_to},
    {"link_from", link_from},
    {"view", view},
    {"home", go_root},
    {"back", go_back},
    {"shell", go_dos},
    {"search", search},
    {"delete", delete},
    {"newmsg", newmsg},
    {"reply", reply},
    {"quote", quote},
    {"move", move},
    {"export", outtxt},
    {"sel_chs", sel_chs},
    {"dos", rundos},
    {"config", set},
    {"list", list},
    {"change", change},
    {"null", nada},
    {"exit", quit},
    {"quit", quit},
    {"prev_area", prev_area},
    {"scan", scan_all_areas},
    {"scan_unscanned", scan_unscanned_areas},
    {"next", right},
    {"chngaddr", chngaddr},
    {"repoth", reply_oarea},
    {"followup", followup},
    {"u-next", go_next},
    {"first", first},
    {"slast", slast},
    {"astart", astart},
    {"chnodel", chngnodel},
    {"name", chngname},
    {"repext", replyextra},
    {"edithdr", edithdr},
    {"pmail", pmail},
    {"cur-pmail", spmail},
    {"hdrsearch", hdrsearch},
    {"down", Go_Dwn},
    {"up", Go_Up},
    {"help", do_help},
    {NULL, NULL}
};

#endif
