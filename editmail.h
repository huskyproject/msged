/*
 *  EDITMAIL.H
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 */

#ifndef __EDITMAIL_H__
#define __EDITMAIL_H__

static void backspace(void);
static void rotate(void);
static void delete_character(void);
static void delword(void);
static void go_left(void);
static void go_right(void);
static void go_word_right(void);
static void go_word_left(void);
static void newline(void);
static void go_up(void);
static void go_down(void);
static void go_pgup(void);
static void go_pgdown(void);
static void delete_line(void);
static void go_eol(void);
static void cut(void);
static void toggle_quote(void);
static void paste(void);
static void anchor(void);
static void quit(void);
static void die(void);
static void imptxt(void);
static void outtext(void);
static void shellos(void);
static void go_bol(void);
static void toggle_ins(void);
static void tabit(void);
static void go_tos(void);
static void go_bos(void);
static void go_bom(void);
static void go_tom(void);
static void nada(void);
static void emacskill(void);
static void killeol(void);
static void undelete(void);
static void bytecount(void);
static void do_help(void);
static void doscmd(void);
static void unblock(void);
static void editheader(void);
static void setup(void);
static void zap_quotes(void);

void e_uudecode(void);

/* table of normal keystrokes (<ctrl> + normal keypresses) */

void (*editckeys[256]) (void) =
{
    NULL, NULL, NULL, NULL, e_uudecode, NULL, NULL, NULL,     /* 0 */
    backspace, tabit, NULL, emacskill, NULL, newline, NULL, NULL,  /* 8 */
    NULL, NULL, NULL, NULL, delword, undelete, NULL, NULL,    /* 10 */
    NULL, delete_line, NULL, die, NULL, NULL, NULL, NULL,     /* 18 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 20 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 28 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 30 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 38 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 40 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 48 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 50 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 58 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 60 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 68 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 70 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, backspace,      /* 78 */
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

/* table of extended keystrokes (<alt> keypresses) */

void (*editakeys[256]) (void) =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 0 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,      /* 8 */
    toggle_quote, outtext, editheader, rotate, setup, NULL, unblock, imptxt,  /* 10 */
    shellos, paste, NULL, NULL, NULL, NULL, anchor, quit,     /* 18 */
    delete_line, NULL, NULL, do_help, NULL, killeol, zap_quotes, NULL,  /* 20 */
    NULL, NULL, NULL, NULL, NULL, NULL, cut, NULL,            /* 28 */
    bytecount, NULL, NULL, NULL, NULL, NULL, NULL, NULL,      /* 30 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 38 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, go_bol,         /* 40 */
    go_up, go_pgup, NULL, go_left, NULL, go_right, NULL, go_eol,  /* 48 */
    go_down, go_pgdown, toggle_ins, delete_character, NULL, NULL, NULL, NULL,  /* 50 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 58 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 60 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,           /* 68 */
    NULL, NULL, NULL, go_word_left, go_word_right, go_bom, go_bos, go_tom,  /* 70 */
    doscmd, NULL, NULL, NULL, NULL, NULL, NULL, NULL,         /* 78 */
    NULL, NULL, NULL, NULL, go_tos, NULL, NULL, NULL,         /* 80 */
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

/* table linking the functions with their config names */

const struct _command editcmds[] =
{
    {"backspace", backspace},
    {"deleol", killeol},
    {"left", go_left},
    {"right", go_right},
    {"wordright", go_word_right},
    {"wordleft", go_word_left},
    {"newline", newline},
    {"up", go_up},
    {"down", go_down},
    {"pgup", go_pgup},
    {"pgdn", go_pgdown},
    {"emacskill", emacskill},
    {"delline", delete_line},
    {"goeol", go_eol},
    {"cut", cut},
    {"anchor", anchor},
    {"paste", paste},
    {"quit", quit},
    {"abort", die},
    {"import", imptxt},
    {"export", outtext},
    {"shell", shellos},
    {"gobol", go_bol},
    {"insert", toggle_ins},
    {"undel", undelete},
    {"tab", tabit},
    {"null", nada},
    {"top", go_tos},
    {"bottom", go_bos},
    {"first", go_tom},
    {"last", go_bom},
    {"del", delete_character},
    {"killword", delword},
    {"toggleq", toggle_quote},
    {"bytecount", bytecount},
    {"oscmd", doscmd},
    {"unblock", unblock},
    {"edithdr", editheader},
    {"setup", setup},
    {"zap", zap_quotes},
    {NULL, NULL}
};

#endif
