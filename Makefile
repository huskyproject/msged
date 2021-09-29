# msged/Makefile
#
# This file is part of msged, part of the Husky fidonet software project
# Use with GNU make v.3.82 or later
# Requires: husky enviroment
#

# Version
msged_g1:=$(GREP) -Po 'define\s+VERNUM\s+\"\K\d+\.\d+'
msged_g2:=$(GREP) -Po 'define\s+VERPATCH\s+\"\K\.\d+'
msged_g3:=$(GREP) -Po 'char\s+cvs_date\[\]\s*=\s*"\K\d+-\d+-\d+'
msged_VERNUM   := $(shell $(msged_g1) $(msged_ROOTDIR)$(msged_H_DIR)version.h)
msged_VERPATCH := $(shell $(msged_g2) $(msged_ROOTDIR)$(msged_H_DIR)version.h)
msged_cvsdate  := $(shell $(msged_g3) $(msged_ROOTDIR)cvsdate.h)
msged_reldate  := $(subst -,,$(msged_cvsdate))
 
msged_VER      := $(msged_VERNUM).$(msged_reldate)

msged_LIBS     := $(fidoconf_TARGET_BLD) $(smapi_TARGET_BLD) $(huskylib_TARGET_BLD)
msged_LIBS_DST := $(fidoconf_TARGET_DST) $(smapi_TARGET_DST) $(huskylib_TARGET_DST)

ifndef MSGEDCFG
  MSGEDCFG=\"$(CFGDIR)/msged.cfg\"
endif

msged_DOCDIR_DST = $(DOCDIR_DST)msged-$(msged_VER)$(DIRSEP)

msged_CDEFS := $(CDEFS) -I$(fidoconf_ROOTDIR) \
                        -I$(smapi_ROOTDIR) \
                        -I$(huskylib_ROOTDIR) \
                        -I$(msged_ROOTDIR)$(msged_H_DIR)

msged_CDEFS+=-DUSE_MSGAPI -DUSE_FIDOCONFIG -DUNAME=\"$(UNAME)\" \
             -DREADMAPSDAT=\"$(CFGDIR)$(DIRSEP)msged$(DIRSEP)readmaps.dat\" \
             -DWRITMAPSDAT=\"$(CFGDIR)$(DIRSEP)msged$(DIRSEP)writmaps.dat\" \
             -DDEFAULT_CONFIG_FILE=$(MSGEDCFG)

msged_SRC := $(wildcard $(msged_SRCDIR)*$(_C))


ifeq ($(OSTYPE), UNIX)
    ifneq ("$(TERMCAP)", "")
        msged_OSLIBS=-l$(TERMCAP)
    endif
    # remove what belongs to OS2
    msged_SRC := $(msged_SRC:os2scr$(_C)=)
    msged_SRC := $(msged_SRC:malloc16$(_C)=)
    # remove what belongs to WINNT
    msged_SRC := $(msged_SRC:winntscr$(_C)=)
endif
ifeq ($(OSTYPE), OS2)
    # remove what belongs to UNIX
    msged_SRC := $(msged_SRC:ansi$(_C)=)
    msged_SRC := $(msged_SRC:readtc$(_C)=)
    # remove what belongs to  WINNT
    msged_SRC := $(msged_SRC:winntscr$(_C)=)
endif
ifeq ($(OSTYPE), WINNT)
    # remove what belongs to UNIX
    msged_SRC := $(msged_SRC:ansi$(_C)=)
    msged_SRC := $(msged_SRC:readtc$(_C)=)
    # remove what belongs to OS2
    msged_SRC := $(msged_SRC:os2scr$(_C)=)
    msged_SRC := $(msged_SRC:malloc16$(_C)=)
endif
ifeq ($(OSTYPE), Cygwin)
    ifneq ("$(TERMCAP)", )
        msged_OSLIBS=-l$(TERMCAP)
    endif
    # remove what belongs to OS2
    msged_SRC := $(msged_SRC:os2scr$(_C)=)
    msged_SRC := $(msged_SRC:malloc16$(_C)=)
    # remove what belongs to WINNT
    msged_SRC := $(msged_SRC:winntscr$(_C)=)
endif
ifneq ($(OSTYPE), MSDOS)
    msged_SRC := $(msged_SRC:dosasm$(_C)=)
    msged_SRC := $(msged_SRC:dosmisc$(_C)=)
endif
# The source files to exclude
msged_excl := ibmscrn$(_C) mouse4$(_C) pacific$(_C) rfind1st$(_C) sasc$(_C) \
              vio$(_C)
msged_excl := $(addprefix $(msged_SRCDIR),$(msged_excl))
msged_SRC := $(filter-out $(msged_excl),$(msged_SRC))

msged_DEPS := $(addprefix $(msged_DEPDIR),$(notdir $(msged_SRC:$(_C)=$(_DEP))))

# Exclude the source of a separate application
msged_SRC := $(msged_SRC:testcons$(_C)=)
msged_OBJS := $(addprefix $(msged_OBJDIR),$(notdir $(msged_SRC:$(_C)=$(_OBJ))))

msged_TARGET     = msged$(_EXE)
msged_TARGET_BLD = $(msged_BUILDDIR)$(msged_TARGET)
msged_TARGET_DST = $(BINDIR_DST)$(msged_TARGET)


.PHONY: msged_all msged_install msged_clean msged_distclean msged_uninstall \
        build_maps install_maps clean_maps distclean_maps uninstall_maps \
        msged_depend msged_doc msged_doc_install msged_doc_uninstall \
        msged_rmdir_DEP msged_rm_DEPS msged_clean_OBJ msged_main_distclean \
        uninstall_msghelp uninstall_msged_DOCDIR_DST

ifeq ($(DYNLIBS), 1)
    ifeq ($(OSTYPE), UNIX)
        msged_all: $(msged_TARGET_BLD) $(msged_BUILDDIR)testcons$(_EXE) msged_doc ;
    else
        msged_all: $(msged_TARGET_BLD) msged_doc ;
    endif
else
    ifeq ($(OSTYPE), UNIX)
        msged_all: $(msged_BUILDDIR)msghelp.dat $(msged_BUILDDIR)testcons$(_EXE) \
                   build_maps msged_doc ;
    else
        msged_all: $(msged_BUILDDIR)msghelp.dat build_maps msged_doc ;
    endif
endif

ifneq ($(MAKECMDGOALS), depend)
    include $(msged_MAPDIR)makefile.husky
    include $(msged_DOCDIR)makefile.husky
ifneq ($(MAKECMDGOALS), distclean)
ifneq ($(MAKECMDGOALS), uninstall)
    include $(msged_DEPS)
endif
endif
endif

ifneq ($(DYNLIBS), 1)
$(msged_BUILDDIR)msghelp.dat: $(msged_SRCDIR)msghelp.src $(msged_TARGET_BLD)
	$(msged_TARGET_BLD) -hc $(msged_SRCDIR)msghelp.src $@
endif

# Build application
$(msged_TARGET_BLD): $(msged_OBJS) $(msged_LIBS) | do_not_run_make_as_root
	$(CC) $(LFLAGS) $(EXENAMEFLAG) $@ $^ $(msged_OSLIBS)

# Compile .c files
$(msged_OBJS): $(msged_OBJDIR)%$(_OBJ): $(msged_SRCDIR)%$(_C) | $(msged_OBJDIR)
	$(CC) $(CFLAGS) $(msged_CDEFS) $(OBJNAMEFLAG) $(msged_OBJDIR)$*$(_OBJ) $(msged_SRCDIR)$*$(_C)

$(msged_OBJDIR): | $(msged_BUILDDIR) do_not_run_make_as_root
	[ -d $@ ] || $(MKDIR) $(MKDIROPT) $@

# testcons
ifeq ($(OSTYPE), UNIX)
$(msged_BUILDDIR)testcons$(_EXE): $(msged_OBJDIR)testcons$(_OBJ) \
    $(msged_LIBS) | do_not_run_make_as_root
	$(CC) $(LFLAGS) $(EXENAMEFLAG) $@ $^ $(msged_OSLIBS)

$(msged_OBJDIR)testcons$(_OBJ): $(msged_SRCDIR)testcons$(_C) | $(msged_OBJDIR)
	$(CC) $(CFLAGS) $(msged_CDEFS) $(OBJNAMEFLAG) $@ $<
endif


# Install
msged_install: $(msged_TARGET_DST) $(msged_DOCDIR_DST)msghelp.dat install_maps \
               msged_doc_install ;

$(msged_TARGET_DST): $(msged_TARGET_BLD) | $(DESTDIR)$(BINDIR)
	$(INSTALL) $(IBOPT) $< $(DESTDIR)$(BINDIR); \
	$(TOUCH) "$@"

ifeq ($(DYNLIBS), 1)
    $(msged_DOCDIR_DST)msghelp.dat: $(msged_SRCDIR)msghelp.src \
        $(msged_TARGET_BLD) $(msged_LIBS_DST) | $(msged_DOCDIR_DST)
			$(msged_TARGET_BLD) -hc $(msged_SRCDIR)msghelp.src $@; \
			$(TOUCH) "$@"
else
    $(msged_DOCDIR_DST)msghelp.dat: $(msged_BUILDDIR)msghelp.dat | \
        $(msged_DOCDIR_DST)
		$(INSTALL) $(IMOPT) $< $(msged_DOCDIR_DST); \
		$(TOUCH) "$@"
endif


# Clean
msged_clean: msged_clean_OBJ
	-[ -d "$(msged_OBJDIR)" ] && $(RMDIR) $(msged_OBJDIR) || true

msged_clean_OBJ: clean_maps msged_doc_clean
	-$(RM) $(RMOPT) $(msged_OBJDIR)*


# Distclean
msged_distclean: distclean_maps msged_doc_distclean msged_main_distclean msged_rmdir_DEP
	-[ -d "$(msged_BUILDDIR)" ] && $(RMDIR) $(msged_BUILDDIR) || true

msged_rmdir_DEP: msged_rm_DEPS
	-[ -d "$(msged_DEPDIR)" ] && $(RMDIR) $(msged_DEPDIR) || true

msged_rm_DEPS:
	-$(RM) $(RMOPT) $(msged_DEPDIR)*

msged_main_distclean: msged_clean
	-$(RM) $(RMOPT) $(msged_TARGET_BLD)
	-$(RM) $(RMOPT) $(msged_BUILDDIR)testcons$(_EXE)
	-$(RM) $(RMOPT) $(msged_BUILDDIR)msghelp.dat


# Uninstall
msged_uninstall: uninstall_msged_DOCDIR_DST
	-$(RM) $(RMOPT) $(msged_TARGET_DST)

uninstall_msged_DOCDIR_DST: uninstall_msghelp uninstall_maps msged_doc_uninstall
	-[ -d "$(msged_DOCDIR_DST)" ] && $(RMDIR) $(msged_DOCDIR_DST) || true

uninstall_msghelp:
	-$(RM) $(RMOPT) $(msged_DOCDIR_DST)msghelp.dat


# Depend
msged_depend: $(msged_DEPS) ;

# Build a dependency makefile for every source file
$(msged_DEPS): $(msged_DEPDIR)%$(_DEP): $(msged_SRCDIR)%.c | $(msged_DEPDIR)
	@set -e; rm -f $@; \
	$(CC) -MM $(msged_CFLAGS) $(msged_CDEFS) $< > $@.$$$$; \
	sed 's,\($*\)$(_OBJ)[ :]*,$(msged_OBJDIR)\1$(_OBJ) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(msged_DEPDIR): | $(msged_BUILDDIR) do_not_run_depend_as_root
	[ -d $@ ] || $(MKDIR) $(MKDIROPT) $@

$(msged_BUILDDIR):
	[ -d $@ ] || $(MKDIR) $(MKDIROPT) $@
