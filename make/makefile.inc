# $Id$
#
# Common makefiles part (include-file) for msged

# Use defines before including this file:
# _OBJ  object files suffix begins with point (".o", ".obj", ... )
# _LIB  library files suffix begins with point (".a", ".lib", ... )
# _DLL  dinamic linking library files suffix begins with point (".dll", ".so", ...)
# LIBPREFIX  library filename prefix ("lib", "", ...)
# LIBSUFFIX  library filename suffix (compiler ID, three characters, somewhere empty)
# DLLPREFIX  dinamic linking library prefix (usually empty)
# DLLSUFFIX  dinamic linking library suffix (compiler ID or empty)
# O, B, L, H prefixes to use with names of objects, binaries, libraries and
# headers respectively

# Module name
LIBNAME = msged

# Version
VERMAJOR= 1
VERMINOR= 9
VERPATCH= 0
VERH    = $(VERMAJOR).$(VERMINOR)
VER     = $(VERH).$(VERPATCH)

# Sources directory
_SRC_DIR= ../
# Headers directory
H_DIR   = ../

# Programm name
msged  = $(B)msged$(_EXE)

# Object files (please sort list to easy check by human)
OBJS = $(O)addr$(_OBJ) $(O)areas$(_OBJ) $(O)bmg$(_OBJ) $(O)charset$(_OBJ) \
       $(O)config$(_OBJ) $(O)control$(_OBJ) $(O)date$(_OBJ) \
       $(O)dialogs$(_OBJ) $(O)dirute$(_OBJ) $(O)dlgbox$(_OBJ) \
       $(O)dlist$(_OBJ) $(O)echotoss$(_OBJ) $(O)environ$(_OBJ) \
       $(O)fconf$(_OBJ) $(O)fecfg145$(_OBJ) $(O)fido$(_OBJ) \
       $(O)filedlg$(_OBJ) $(O)flags$(_OBJ) $(O)freq$(_OBJ) \
       $(O)gestr120$(_OBJ) $(O)getopts$(_OBJ) $(O)group$(_OBJ) \
       $(O)help$(_OBJ) $(O)helpcmp$(_OBJ) $(O)helpinfo$(_OBJ) \
       $(O)init$(_OBJ) $(O)keycode$(_OBJ) $(O)list$(_OBJ) \
       $(O)maintmsg$(_OBJ) $(O)makemsgn$(_OBJ) $(O)memextra$(_OBJ) \
       $(O)menu$(_OBJ) $(O)misc$(_OBJ) $(O)mnu$(_OBJ) $(O)msg$(_OBJ) \
       $(O)msged$(_OBJ) $(O)mxbt$(_OBJ) $(O)normalc$(_OBJ) $(O)nshow$(_OBJ) \
       $(O)quick$(_OBJ) $(O)quote$(_OBJ) $(O)readmail$(_OBJ) \
       $(O)screen$(_OBJ) $(O)strextra$(_OBJ) $(O)system$(_OBJ) \
       $(O)template$(_OBJ) $(O)textfile$(_OBJ) $(O)timezone$(_OBJ) \
       $(O)userlist$(_OBJ) $(O)vsev$(_OBJ) $(O)vsevops$(_OBJ) $(O)win$(_OBJ) \
       $(O)winntscr$(_OBJ) $(O)wrap$(_OBJ)

# Object files: programs
PRGOBJS = $(OBJS)

# Header files
HEADERS = $(H)cvsdate.h $(H)version.h


# Binary file(s) to build from sources
PROGRAMS= $(msged)

# List of libraries required to build binary file(s)
LIBS    =
