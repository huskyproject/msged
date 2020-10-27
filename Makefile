# Makefile for the Husky build environment

# include Husky-Makefile-Config
ifeq ($(DEBIAN), 1)
include debian/huskymak.cfg
else ifdef RPM_BUILD_ROOT
# RPM build requires all files to be in the source directory
include huskymak.cfg
else
include ../huskymak.cfg
endif

ifeq ($(DEBUG), 1)
  CFLAGS=$(WARNFLAGS) $(DEBCFLAGS) -I$(INCDIR)
  LFLAGS=$(DEBLFLAGS)
else
  CFLAGS=$(WARNFLAGS) $(OPTCFLAGS) -I$(INCDIR)
  LFLAGS=$(OPTLFLAGS)
endif

ifndef MSGEDCFG
  MSGEDCFG=\"$(CFGDIR)/msged.cfg\"
endif

# adapt to new huskymak.cfg

ifeq ("$(OBJ)", "")
  OBJ=$(_OBJ)
endif
ifeq ("$(EXE)", "")
  EXE=$(_EXE)
endif

CDEFS=-D$(OSTYPE) -DUSE_MSGAPI -DUSE_FIDOCONFIG -DUNAME=\"$(UNAME)\" \
      $(ADDCDEFS) -DREADMAPSDAT=\"$(CFGDIR)/msged/readmaps.dat\" \
      -DWRITMAPSDAT=\"$(CFGDIR)/msged/writmaps.dat\" \
      -DDEFAULT_CONFIG_FILE=$(MSGEDCFG)

ifeq ($(SHORTNAME), 1)
  LIBS= -L$(LIBDIR) -lfidoconf -lsmapi -lhusky
else
  LIBS= -L$(LIBDIR) -lfidoconfig -lsmapi -lhusky
endif

TARGET=	msged$(EXE)

ifeq ($(OSTYPE), UNIX)
  osobjs=	ansi$(OBJ) \
		readtc$(OBJ)
  ifneq ("$(TERMCAP)", "")
    OSLIBS=-l$(TERMCAP)
  endif
endif
ifeq ($(OSTYPE), OS2)
  osobjs=	os2scr$(OBJ) \
		malloc16$(OBJ)
endif
ifeq ($(OSTYPE), WINNT)
  osobjs=	winntscr$(OBJ)
endif
ifeq ($(OSTYPE), Cygwin)
  osobjs= ansi$(OBJ) readtc$(OBJ)
    OSLIBS=-ltermcap
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
	group$(OBJ)    \
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
	timezone$(OBJ) \
	userlist$(OBJ) \
	vsev$(OBJ)     \
	vsevops$(OBJ)  \
	win$(OBJ)      \
	wrap$(OBJ)


ifeq ($(OSTYPE), UNIX)
   all: $(TARGET) testcons do-maps msghelp.dat
else
   all: $(TARGET) do-maps msghelp.dat

endif

do-maps:
	(cd maps && $(MAKE) -f makefile.husky)
	(cd doc && cd manual && $(MAKE) -f makefile.husky)


%$(OBJ): %.c
	$(CC) $(CFLAGS) $(CDEFS) -c $*.c

$(TARGET): $(objs) $(osobjs)
	$(CC) $(LFLAGS) -o $(TARGET) $(objs) $(osobjs) $(LIBS) $(OSLIBS)

ifeq ($(OSTYPE), UNIX)
testcons: testcons$(OBJ)
	$(CC) $(LFLAGS) -o testcons$(EXE) testcons$(OBJ) $(LIBS) $(OSLIBS)
endif

msghelp.dat: msghelp.src
	.$(DIRSEP)$(TARGET) -hc msghelp.src msghelp.dat

clean:
	-$(RM) $(RMOPT) *$(OBJ)
	-$(RM) $(RMOPT) *~
	(cd maps && $(MAKE) -f makefile.husky clean)
	(cd doc && cd manual && $(MAKE) -f makefile.husky clean)

distclean: clean
	-$(RM) $(RMOPT) $(TARGET)
	-$(RM) $(RMOPT) msghelp.dat
	-$(RM) $(RMOPT) testcons$(EXE)
	(cd maps && $(MAKE) -f makefile.husky distclean)
	(cd doc && cd manual && $(MAKE) -f makefile.husky distclean)

ifeq ($(OSTYPE), UNIX)

ifdef RPM_BUILD_ROOT
install: $(TARGET) msghelp.dat
	-$(MKDIR) $(MKDIROPT) $(DESTDIR)$(BINDIR)
	$(INSTALL) $(IBOPT) $(TARGET) $(DESTDIR)$(BINDIR)
	-$(MKDIR) $(MKDIROPT) $(DESTDIR)$(CFGDIR)
	-$(MKDIR) $(MKDIROPT) $(DESTDIR)$(CFGDIR)/msged
	$(INSTALL) $(IIOPT) msghelp.dat $(DESTDIR)$(CFGDIR)/msged
	(cd maps && $(MAKE) -f makefile.husky install)
	(cd doc && $(MAKE) -f makefile.husky install)
	(cd doc && cd manual && $(MAKE) -f makefile.husky install)
else
install: $(TARGET) msghelp.dat testcons$(EXE)
	-$(MKDIR) $(MKDIROPT) $(DESTDIR)$(BINDIR)
	$(INSTALL) $(IBOPT) $(TARGET) $(DESTDIR)$(BINDIR)
	-$(MKDIR) $(MKDIROPT) $(DESTDIR)$(CFGDIR)
	$(INSTALL) $(IIOPT) msghelp.dat $(DESTDIR)$(CFGDIR)
	(cd maps && $(MAKE) -f makefile.husky install)
	(cd doc && cd manual && $(MAKE) -f makefile.husky install)
	$(INSTALL) $(IBOPT) testcons$(EXE) $(DESTDIR)$(BINDIR)
endif

else

install: $(TARGET) msghelp.dat
	-$(MKDIR) $(MKDIROPT) $(DESTDIR)$(BINDIR)
	$(INSTALL) $(IBOPT) $(TARGET) $(DESTDIR)$(BINDIR)
	-$(MKDIR) $(MKDIROPT) $(DESTDIR)$(CFGDIR)
	-$(MKDIR) $(MKDIROPT) $(DESTDIR)$(CFGDIR)/msged
	$(INSTALL) $(IIOPT) msghelp.dat $(DESTDIR)$(CFGDIR)/msged
	(cd maps && $(MAKE) -f makefile.husky install)
	(cd doc && cd manual && $(MAKE) -f makefile.husky install)

endif

uninstall:
	-$(RM) $(RMOPT) $(DESTDIR)$(BINDIR)$(DIRSEP)$(TARGET)
	-$(RM) $(RMOPT) $(DESTDIR)$(BINDIR)$(DIRSEP)testcons$(EXE) $(BINDIR)
	-$(RM) $(RMOPT) $(DESTDIR)$(CFGDIR)$(DIRSEP)msghelp.dat
	(cd maps && $(MAKE) -f makefile.husky uninstall)
	(cd doc && cd manual && $(MAKE) -f makefile.husky uninstall)

