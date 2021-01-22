/*
 *  MSGED.H
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Main header file for all Msged source files.
 */

#ifndef __MSGED_H__
#define __MSGED_H__

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef PATHLEN
#undef PATHLEN
#endif

#define NO 0
#define YES 1
#define FAIL 0
#define OK 1
#define HIDE 2
#define RIGHT 0
#define LEFT 1
#define SAVE 1
#define ABORT -1

#define GDOMAINS 0x01           /* gate domains */
#define GZONES 0x02             /* gate zones */
#define BOTH 0x04               /* gate both */
#define GASK 0x08               /* ask if gate should be used */

#define DSCTAGASIS 0x01         /* what to import from the areafile  */
#define DSCTAGUPPER 0x02        /* and how to do it                  */
#define DSCTAGLOWER 0x04
#define DSCDESCASIS 0x08
#define DSCDESCUPPER 0x10
#define DSCDESCLOWER 0x20

#define FIDO 0x00               /* message base types */
#define QUICK 0x01
#define SQUISH 0x02
#define JAM 0x03
/* #define AREASBBS   0x00 */   /* area file types */
#define FASTECHO 0x01
/* #define SQUISH     0x02 */
#define GECHO 0x03
#define FIDOCONFIG 0x04

#define USENET 0x01
#define FIDONET 0x02

#define PATHLEN 64
#define BLOCKLEN 255
/* A buffer of this size must be able to
   store a) the complete amount of kludges in
   the message, or alternatively b) a complete
   paragraph (i.E. a text block without CR's
   in it.  Especially in times of internet
   gateways, the amount of kludge lines can
   easily grow past 4K (think of myriads of
   @RFC-CC: kludges etc.). Msged will not
   crash if the buffer has an overflow, but
   the overflowing parts of the message will
   simply not be displayed. */
#ifdef DOS
#define BUFLEN 4096
#else
#define BUFLEN 16384
#endif

#define MAXUSERS 20

#define MT_QUO 0x0001           /* msg has a quote */
#define MT_REP 0x0002           /* msg is a reply (no quote) */
#define MT_ARC 0x0004           /* check for dest. area */
#define MT_FOL 0x0008           /* insert followup template */
#define MT_NEW 0x0010           /* msg is new (no to:) */
#define MT_FOR 0x0020           /* forwarded message */
#define MT_RED 0x0040           /* redirected message */

#define WR_HEADER 0x0001        /* write header only */
#define WR_ALL 0x0002           /* write all msg */
#define RD_HEADER 0x0004        /* read header only */
#define RD_ALL 0x0008           /* read all msg */
#define RD_HEADER_BRIEF 0x0010  /* read header without replies */

#define ERR_OPEN_MSG -1         /* error opening msg */
#define ERR_CLOSE_MSG -2        /* error closing message */
#define ERR_OPEN_AREA -3        /* error opening area */
#define ERR_CLOSE_AREA -4       /* error closing area */
#define ERR_NO_AREA -5          /* no area currently open! */
/* file permissions */
#if defined (UNIX)              /* file permissions for creation */
#define S_IMODE_NETMAIL S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
#define S_IMODE_ECHOMAIL S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
#define S_IMODE_LASTREAD S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
#define S_IMODE ((CurArea.netmail || CurArea.local) ? S_IMODE_NETMAIL : S_IMODE_ECHOMAIL)
#else /* not UNIX */
#define S_IMODE S_IREAD | S_IWRITE
#define S_IMODE_LASTREAD S_IREAD | S_IWRITE
#endif
/* useful time-saving macros */

#define ST string_vars
#define SW switch_vars
#define thisnode alias[0]
#define CurArea arealist[SW->area]
/* structures and typedefs */
/* Attributes used & recognized by Msged */
struct _attributes
{
    unsigned int priv      : 1;  /* private message flag */
    unsigned int crash     : 1;  /* crash mail */
    unsigned int rcvd      : 1;  /* received by addressee */
    unsigned int sent      : 1;  /* message sent */
    unsigned int attach    : 1;  /* file attached */
    unsigned int forward   : 1;  /* message in transit */
    unsigned int orphan    : 1;  /* unknown destination */
    unsigned int killsent  : 1;  /* kill after sending */
    unsigned int local     : 1;  /* local message */
    unsigned int hold      : 1;  /* hold for pickup */
    unsigned int direct    : 1;  /* do no gating on this msg */
    unsigned int freq      : 1;  /* file request */
    unsigned int rreq      : 1;  /* return receipt requested */
    unsigned int rcpt      : 1;  /* return receipt */
    unsigned int areq      : 1;  /* audit trail request */
    unsigned int ureq      : 1;  /* update file request */
    /* extended attributes only found in @FLAGS kludge line */
    unsigned int kfs       : 1;  /* kill file when sent */
    unsigned int as        : 1;  /* archive when sent */
    unsigned int immediate : 1;  /* immediate delivery */
    unsigned int tfs       : 1;  /* truncate file when sent */
    unsigned int lock      : 1;  /* do not process this mail */
    unsigned int cfm       : 1;  /* confirm receipt request */
    unsigned int zon       : 1;  /* send mail via zone gate */
    unsigned int hub       : 1;  /* hub routing */
};

/* Structure defining an alias for header and cc: entry */
typedef struct _alias
{
    int                attr;    /* was an attribute specified? */
    char *             alias;   /* the alias */
    char *             name;    /* the real name */
    char *             subj;    /* the subject */
    struct _attributes attrib;  /* the attributes (if used) */
    FIDO_ADDRESS       addr;    /* the address (if used) */
} ALIAS;
/* Structure defining a Version7 nodelist */
typedef struct _domain_list
{
    char * name;                /* full domain name eg. fidonet.org */
    char * base_name;           /* base name of nodelist ie. nodex */
    char * sysop;               /* sysop lookup filename eg. sysop.ndx */
} D_LIST;
/*
 *  Defines a user plus the corresponding useroffset and lastread
 *  values.  The current values are copied to the global equivalents
 *  (to avoid extra work).
 */
typedef struct _user
{
    char *        name;
    char *        lastread;
    char *        robotname;
    unsigned long offset;
    int           offs_defined;
} USER;
/* The internal area structure. Contains all info relevant for an area. */
typedef struct _area
{
    int          status;        /* status of the area, 0 = closed, 1 = open */
    FIDO_ADDRESS addr;          /* the address to use in this message */
    char *       description;   /* what the user calls the area */
    char *       tag;           /* what confmail calls it! */
    char *       path;          /* where the area is on disk */
    unsigned int local     : 1;  /* local message area */
    unsigned int netmail   : 1;  /* netmail message area */
    unsigned int echomail  : 1;  /* echomail area */
    unsigned int news      : 1;  /* usenet news area */
    unsigned int uucp      : 1;  /* usenet mail area */
    unsigned int new : 1;        /* a message has been entered */
    unsigned int priv      : 1;  /* default private */
    unsigned int hold      : 1;  /* default hold */
    unsigned int direct    : 1;  /* default direct */
    unsigned int crash     : 1;  /* default crash */
    unsigned int killsent  : 1;  /* default crash */
    unsigned int eightbits : 1;  /* use charset kludging */
    unsigned int scanned   : 1;  /* area has been scanned */
    unsigned int recodedsc : 1;  /* recode areatag from IBMPC to local chrs */
    int          group;         /* Group handle */
    int          areanumber;    /* Number of this area. Normally corresponds
                                   to the array index. Needed to make qsort
                                   stable when sorting the areas */
    int template;               /* index into template array */
    int           username;     /* index into username array */
    unsigned int  msgtype;      /* the message type */
    int           board;        /* if a quickbbs area, which board */
    unsigned long first;        /* first message in the area? */
    unsigned long last;         /* last message in the area */
    unsigned long current;      /* current message in the area */
    unsigned long messages;     /* how many messages in the area */
    unsigned long lastread;     /* the highest message read */
} AREA;
/*
 *  The internal message structure.  Built to reflect both Squish
 *  (32-bit) and other messagebase structures.
 */
typedef struct _msg
{
    unsigned long msgnum;        /* message number (UMSGID) */
    char *        reply;         /* id of message this is a reply to */
    char *        msgid;         /* this messages msgid */
    char *        isfrom;        /* who from */
    char *        isto;          /* who to */
    char *        subj;          /* message subject */
    char *        replyarea;     /* AREA: kludge */
    char *        charset_name;  /* CHRS: kludge - first string */
    int           charset_level; /* CHRS: kludge - FTSC 0054 level number */
    unsigned int new : 1;         /* new message? */
    unsigned int       change     : 1;/* message been changed? */
    unsigned int       scanned    : 1;/* msg been scanned? */
    unsigned int       newrcvd    : 1;/* msg just received? */
    unsigned int       soteot     : 1;/* msg sot/eot protected? */
    unsigned int       invkludges : 1; /* invalidate kludges? */
    unsigned int       rawcopy    : 1;/* copy msg text/ctrlinfo raw? */
    unsigned int       has_timezone : 1; /* if timezone contains valid information */
    time_t             timestamp;  /* creation date, */
    time_t             time_arvd;  /* time message arrived */
    int                timezone;   /* offset from utc in minutes, east = + */
    unsigned long      replyto;    /* thread to previous msg */
    unsigned long      replies[10]; /* thread to next msg(s) */
    struct _attributes attrib;  /* message attribute */
    int                times_read; /* times msg been read */
    int                cost;       /* cost of message */
    FIDO_ADDRESS       to;         /* destination address of message */
    FIDO_ADDRESS       from;       /* origin address of message */
    LINE *             text;       /* the message buffer */
} msg;
/* A command/label assignment structure (for the keyboard) */
struct _command
{
    char * label;
    void   (* action)(void);
};

/* Handle for msgbase access functions */
typedef struct _msghandle
{
    msg * (*MsgReadHeader)(unsigned long n, int type);
    char * (*MsgReadText)(unsigned long n);
    int (* MsgWriteHeader)(msg * m, int type);
    int (* MsgWriteText)(char * text, unsigned long n, unsigned long mlen);
    int (* MsgDelete)(unsigned long n);
    int (* AreaSetLast)(AREA * a);
    long (* MsgAreaOpen)(AREA * a);
    int (* MsgAreaClose)(void);
    int (* MsgClose)(void);
    unsigned long (* UidToMsgn)(unsigned long n);
    unsigned long (* MsgnToUid)(unsigned long n);
    int (* MsgLockArea)(void);
    int (* MsgUnlockArea)(void);
} msghandle;
/* These are the system strings, for access across the system */
struct _sv
{
    char * username;            /* who is you */
    char * quotestr;            /* how to prefix a quote */
    char * fidolist;            /* nodelist user list */
    char * userlist;            /* personal user list */
    char * origin;              /* origin line */
    char * outfile;             /* default export filename */
    char * infile;              /* default import filename */
    char * home;                /* home directory */
    char * lastread;            /* name of the lastread file */
    char * cfgfile;             /* name of the config file */
    char * echotoss;            /* echotoss log file name */
    char * template;            /* template file */
    char * nodepath;            /* path to current nodelist */
    char * nodebase;            /* base (current) nodelist name */
    char * sysop;               /* name of sysop */
    char * swap_path;           /* swap path */
    char * helpfile;            /* helpfile name */
    char * uucpgate;            /* the UUCP gate name to send to */
    char * comspec;             /* file spec of command processor */
    char * editorName;          /* name of external editor */
    char * quickbbs;            /* where a quickbbs file is */
    char * fecfgpath;           /* path of fastecho configuration file */
    char * output_charset;      /* charset to use for output if the "8" */
                                /* flag is set */
    char * sort_criteria;       /* sort criteria for areas */
    char * special_characters;   /* allowed ASCII codes >= 128 */
    char * freqarea;            /* area tag for area to make f.req.in */
    char * input_charset;       /* charset  assumed when reading mail
                                   without CHRS kludge. */
    char * enforced_charset;    /* input charset that is always enforced */
    char * uucpreplyto;         /* a replyto: string for e-mail */
    char * freqflags;           /* flags to use for file requests */
    char * printer;             /* printer port or command name */
    char * readmap;             /* READMAPS.DAT filename */
    char * writemap;            /* WRITMAPS.DAT filename */
};

/* These are the system switches, for access across the system */
struct _swv
{
    unsigned long useroffset;   /* offset into lastread file */
    int           area;            /* current area number */
    int           areas;           /* how many message areas */
    int           maxareas;        /* how much room in arealist array */
    int           aliascount;      /* how many AKAs aliases do you have? */
    int           otheraliases;    /* how many aliases for other nodes? */
    int           maxotheraliases; /* consume memory in Zimbabwe ;-) */
    int           numtemplates;    /* how many templates in use? */
    int           domains;         /* how many domains listed */
    int           rm;              /* the current right margin */
    int           qm;              /* the current quote margin */
    int           orgrm;           /* the right margin as desired by the user */
    int           orgqm;           /* the quote margin as desired by the user */
    int           pointnet;        /* private net number of point */
    int           tabsize;         /* how many spaces for a tab */
    int           nodelists;       /* number of nodelists */
    int           rquote;          /* dfns for the reply funcs; quote */
    int           rotharea;        /* reply other area */
    int           rfollow;         /* reply followup */
    int           rextra;          /* reply extra - user definable */
    int           gate;            /* zone/domain gate messages? */
    int override;                  /* override the area origin line */
    int redraw;                    /* redraw screen header? */
    int msgids;                    /* add msgid lines */
    int opusdate;                  /* put in the opus time stamp */
    int shownotes;                 /* show hidden lines */
    int showseenbys;               /* show seenby lines */
    int showorigins;               /* show origin lines */
    int showtearlines;             /* show tear lines */
    int confirmations;             /* confirm deletes, aborts? */
    int datearrived;               /* show the date msg arrived? */
    int showaddr;                  /* shows curr address on screen */
    int use_lastr;                 /* whether to use lastread/current */
    int qquote;                    /* quote quotes? */
    int savecc;                    /* save cc original copy? */
    int rawcc;                     /* save the raw cc msg? */
    int chopquote;                 /* chop off the end of quoted msgs? */
    int hardquote;                 /* don't reformat quoted text? */
    int showcr;                    /* show CRs? */
    int showeol;                   /* show eol? */
    int showrealmsgn;              /* show real msg#s in <alt><l> */
    int usemouse;                  /* use the mouse? */
    int tabexpand;                 /* expand tabs? */
    int editcronly;                /* show CRs in editor only */
    int usepid;                    /* use a PID instead of a tear line */
    int soteot;                    /* use SOT/EOT */
    int showtime;                  /* show time in main message area? */
    int importfn;                  /* show the filename headers on import */
    int dmore;                     /* display msg numbers at top of screen */
    int statbar;                   /* show statbar */
    int showsystem;                /* show system name? */
    int extformat;                 /* format externally-edited messages? */
    int arealistexactmatch;        /* exact match area names in area list? */
    int echoflags;                 /* use FLAGS control line in echomail? */
    int netmailvia;                /* use Via control line in netmail? */
    int domainorigin;              /* write domain information to origin line? */
    int rightnextunreadarea;    /* key right to next unread area? */
    int usetearlines;              /* add tear lines in echomail? */
    int useoriginlines;            /* add origin lines in echomail? */
    int edittearlines;             /* add tearlines to template message */
    int editoriginlines;           /* add originlines to template message */
    int lowercase;                 /* Convert filenames to lower case */
    int adaptivecase;              /* Case insenstive behaviour on Unix */
    int receiveallnames;           /* Switch rvd on for mail to all configured */
                                   /* user names, or only for the active one? */
    int receivealladdr;            /* similar thing for FTN addresses */
    int squish_lock;               /* Lock message base for better speed */
    int carthy;                    /* behaviour of delete_line at last line */
    int direct_list;               /* jump directly to message listing mode */
    int areadesc;                  /* how to build areadesc from areafile */
    int areadefinesuser;           /* the selected area defines the username */
    int groupareas;                /* # of areas in current group selection */
    int group;                     /* currently selected group, 0 = none */
    int groupseparators;           /* do we want to draw horizontal bars? */
    int grouparea;                 /* SW->area = grouparealist[SW->grouparea] */
    int areafilegroups;            /* want to read group info from areafile? */
    int domainmsgid;               /* show msgid's with domain string? */
    int tzutc;                     /* generate TZUTC kludge line? */
    int xxltearline;               /* allow tearline > 35 characters? */
};


#ifndef INCL_MAIN

extern struct _swv * switch_vars;  /* array of switches */
extern struct _sv * string_vars;   /* array of strings */
extern msghandle msgdo[];         /* msgbase functions */
extern FIDO_ADDRESS * domain_list; /* list of ^aDOMAIN lookups */
extern FIDO_ADDRESS * alias;      /* list of AKA's */
extern FIDO_ADDRESS uucp_gate;    /* closest UUCP gate */
extern D_LIST * node_lists;       /* list of v7 nodelists */
extern AREA * arealist;           /* list of areas for system */
extern ALIAS * aliaslist;         /* list of alias lookups */
extern USER user_list[MAXUSERS];  /* list of system users */
extern char ** templates;         /* list of templates */
extern char ** origins;           /* list of alternative origins */
extern int n_origins;             /* number of alternative origins */
extern msg * message;             /* current message */
extern unsigned int * macros[41];  /* macros */
extern int maxy;                  /* max y pos, 1 origin */
extern int maxx;                  /* max x pos, 1 origin */
extern int maxx_force;
extern int maxy_force;
extern int cur_start;             /* these are ega/vga default */
extern int cur_end;               /* cursor sizes */
extern int rot13;                 /* rot13 this message? */
extern int stripSoft;             /* strip soft-crs? */
extern int softcrxlat;            /* soft cr replacement if not zero */
extern char * msgbuf;             /* message buffer for reading, size BUFLEN */
extern int msgederr;              /* global error number */
extern int cmd_dbginfo;           /* show debugging info at startup? */
extern int areas_type;            /* areas read from fastecho */
extern int scan;                  /* scan areas for new mail at startup? */
extern int * grouparealist;

#endif // ifndef INCL_MAIN
/* various and sundry macros */

#define release(s) {if(s != NULL)free(s); s = NULL;}

#define MsgReadHeader(n, type) (msgdo[CurArea.msgtype].MsgReadHeader(n, type))
#define MsgReadText(n) (msgdo[CurArea.msgtype].MsgReadText(n))
#define MsgWriteHeader(m, type) (msgdo[CurArea.msgtype].MsgWriteHeader(m, type))
#define MsgWriteText(text, n, mlen) (msgdo[CurArea.msgtype].MsgWriteText(text, n, mlen))
#define MsgDelete(n) (msgdo[CurArea.msgtype].MsgDelete(n))
#define AreaSetLast(a) (msgdo[CurArea.msgtype].AreaSetLast(a))
#define MsgAreaOpen(a) (msgdo[CurArea.msgtype].MsgAreaOpen(a))
#define MsgAreaClose() (msgdo[CurArea.msgtype].MsgAreaClose())
#define MsgClose() (msgdo[CurArea.msgtype].MsgClose())
#define UidToMsgn(n) (msgdo[CurArea.msgtype].UidToMsgn(n))
#define MsgnToUid(n) (msgdo[CurArea.msgtype].MsgnToUid(n))
#define MsgLockArea() (msgdo[CurArea.msgtype].MsgLockArea())
#define MsgUnlockArea() (msgdo[CurArea.msgtype].MsgUnlockArea())

#define DOROT13(c) ((rot13 == 0) ? (c) : handle_rot((c)))
/* prototypes */
msg * KillMsg(msg * m);
int confirm(char * option);
void arealist_area_scan(int);
void e_assignkey(unsigned int key, char * label);
void r_assignkey(unsigned int key, char * label);
void area_scan(int);
void dolist(void);
void dispose(msg * message);
void set_nongrouped_area(int newarea); /* argument is absolute area number */
void set_area(int newgrouparea);       /* argument is a group area number */
void cleanup(char * msg, ...);
void SetupArea(void);
int dv_running(void);
void shell_to_dos(void);
void scan_all_areas(void);
void scan_unscanned_areas(void);

#endif // ifndef __MSGED_H__
