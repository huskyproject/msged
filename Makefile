# Makefile for the Husky build environment

# include Husky-Makefile-Config
include ../huskymak.cfg

ifeq ($(DEBUG), 1)
  CFLAGS=$(WARNFLAGS) $(DEBCFLAGS) -I$(INCDIR)
  LFLAGS=$(DEBLFLAGS)
else
  CFLAGS=$(WARNFLAGS) $(OPTCFLAGS) -I$(INCDIR)
  LFLAGS=$(OPTLFLAGS)
endif

CDEFS=-D$(OSTYPE) -DUSE_MSGAPI -DUSE_FIDOCONFIG -DUNAME=\"$(UNAME)\" \
      $(ADDCDEFS) -DREADMAPSDAT=\"$(CFGDIR)/readmaps.dat\" \
      -DWRITMAPSDAT=\"$(CFGDIR)/writmaps.dat\" \
      -DDEFAULT_CONFIG_FILE=$MSGEDCFG

ifeq ($(SHORTNAME), 1)
  LIBS= -L$(LIBDIR) -lfidoconf -lsmapi
else
  LIBS= -L$(LIBDIR) -lfidoconfig -lsmapi
endif

TARGET=	msged$(EXE)

ifeq ($(OSTYPE), UNIX)
  osobjs=	ansi$(OBJ) \
  		readtc$(OBJ)
  OSLIBS=-l$(TERMCAP)
endif
ifeq ($(OSTYPE), OS2)
  osobjs=	os2scr$(OBJ) \
  		malloc16$(OBJ)
endif
ifeq ($(OSTYPE), WINNT)
  osobjs=	winntscr$(OBJ)
endif


objs=   addr$(OBJ)     \
	areas$(OBJ)    \
	bmg$(OBJ)      \
	charset$(OBJ)  \
	config$(OBJ)   \
	control$(OBJ)  \
	curses$(OBJ)   \
	date$(OBJ)     \
	dialogs$(OBJ)  \
	dirute$(OBJ)   \
	dlgbox$(OBJ)   \
	dlist$(OBJ)    \
	echotoss$(OBJ) \
	environ$(OBJ)  \
	fconf$(OBJ)    \
	fecfg145$(OBJ) \
	fido$(OBJ)     \
	filedlg$(OBJ)  \
	flags$(OBJ)    \
	freq$(OBJ)     \
	gestr120$(OBJ) \
	getopts$(OBJ)  \
	help$(OBJ)     \
	helpcmp$(OBJ)  \
	helpinfo$(OBJ) \
	init$(OBJ)     \
	keycode$(OBJ)  \
	list$(OBJ)     \
	maintmsg$(OBJ) \
	makemsgn$(OBJ) \
	memextra$(OBJ) \
	menu$(OBJ)     \
	misc$(OBJ)     \
	mnu$(OBJ)      \
	msg$(OBJ)      \
	msged$(OBJ)    \
	mxbt$(OBJ)     \
	normalc$(OBJ)  \
	nshow$(OBJ)    \
	quick$(OBJ)    \
	quote$(OBJ)    \
	readmail$(OBJ) \
	screen$(OBJ)   \
	strextra$(OBJ) \
	system$(OBJ)   \
	template$(OBJ) \
	textfile$(OBJ) \
	userlist$(OBJ) \
	vsev$(OBJ)     \
	vsevops$(OBJ)  \
	win$(OBJ)      \
	wrap$(OBJ)

all: $(TARGET) msghelp.dat

%$(OBJ): %.c
	$(CC) $(CFLAGS) $(CDEFS) -c $*.c

$(TARGET): $(objs) $(osobjs)
	$(CC) $(LFLAGS) -o $(TARGET) $(objs) $(osobjs) $(LIBS) $(OSLIBS)

msghelp.dat: msghelp.src
	.$(DIRSEP)$(TARGET) -hc msghelp.src msghelp.dat

clean:
	-$(RM) *$(OBJ)
	-$(RM) *~
	(cd maps && $(MAKE) -f makefile.husky clean)
ifdef INFODIR
	(cd doc && cd manual && $(MAKE) -f makefile.husky clean)
endif


distclean: clean
	-$(RM) $(TARGET)
	-$(RM) msghelp.dat
	(cd maps && $(MAKE) -f makefile.husky distclean)
	(cd doc && cd manual && $(MAKE) -f makefile.husky distclean)


install: $(TARGET) msghelp.dat
	-$(MKDIR) $(MKDIROPT) $(CFGDIR)
	-$(MKDIR) $(MKDIROPT) $(BINDIR)
	$(INSTALL) $(IBOPT) $(TARGET) $(BINDIR)
	$(INSTALL) $(IIOPT) msghelp.dat $(CFGDIR)
	(cd maps && $(MAKE) -f makefile.husky install)
	(cd doc && cd manual && $(MAKE) -f makefile.husky install)


