case $CONFIG in
'')
	if test ! -f config.sh; then
		ln ../config.sh . || \
		ln ../../config.sh . || \
		ln ../../../config.sh . || \
		(echo "Can't find config.sh."; exit 1)
	fi 2>/dev/null
	. ./config.sh
	;;
esac
: This forces SH files to create target in same directory as SH file.
: This is so that make depend always knows where to find SH derivatives.
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac
echo "Extracting Makefile (with variable substitutions)"
: This section of the file will have variable substitutions done on it.
: Move anything that needs config subs from !NO!SUBS! section to !GROK!THIS!.
: Protect any dollar signs and backticks that you do not want interpreted
: by putting a backslash in front.  You may delete these comments.
$spitshell >Makefile <<!GROK!THIS!
# $Id: Makefile.SH,v 2.0.1.29 1995/07/05 08:43:38 greyham Exp $
#
# UNIX makefile for manual page generator
# Note: any changes made here will be lost next time Configure is run!.

CC=$cc $optimize
YACC=\$(KJB_YACC)
LEX=$lex $lexflags
SED=\$(KJB_SED)
CAT=$cat
RM=\$(KJB_RM)
MV=\$(KJB_MV)
CP=\$(KJB_CP)
ECHO=$echo
MKDEP=./mkdep

# where we get installed
bin=$binexp
privlib=$installprivlib

mansrc=$mansrc
manext=$manext
CFLAGS=$ccflags
WARNFLAGS=$warnflags
LDFLAGS=$ldflags
#
# Kobus. 
#
# This comment comes from Makefile.SH which builds the Makefile. 
#
# The Configure script produces full path output for the libraries which forces
# a Configure to build the program on a new machine. On average the following
# works. If it fails somewhere, we should patch it in init_compile somehow.
#
# LIBS=$libs $liblex $libyacc
#
LIBS=-lfl
!GROK!THIS!

: In the following dollars and backticks do not need the extra backslash.
$spitshell >>Makefile <<'!NO!SUBS!'

# As Larry said, "Grrrr"
SHELL=/bin/sh

OSOURCES =	config.h c2man.h semantic.h symbol.h strconcat.h \
		strappend.h manpage.h enum.h output.h lex.l grammar.y
DCSOURCES =	c2man.c semantic.c string.c symbol.c strconcat.c \
		strappend.c manpage.c enum.c nroff.c texinfo.c latex.c html.c \
		autodoc.c
SOURCES =	$(OSOURCES) $(DCSOURCES)
CSOURCES =	$(DCSOURCES) y.tab.c

OBJECTS = $(OBJ_DIR)c2man.o \
          $(OBJ_DIR)semantic.o \
          $(OBJ_DIR)string.o \
          $(OBJ_DIR)symbol.o \
          $(OBJ_DIR)y.tab.o \
          $(OBJ_DIR)strconcat.o \
          $(OBJ_DIR)strappend.o \
          $(OBJ_DIR)manpage.o \
          $(OBJ_DIR)enum.o \
          $(OBJ_DIR)nroff.o \
          $(OBJ_DIR)texinfo.o \
          $(OBJ_DIR)latex.o \
          $(OBJ_DIR)html.o \
          $(OBJ_DIR)autodoc.o

GENERATED =	$(OBJ_DIR)c2man example.inc ctype_ex.inc y.tab.c lex.yy.c y.output \
		fixexamp.sed flatten.sed

#
# Hack the directory making for now.
#
all: $(OBJ_DIR) $(OBJ_DIR)c2man c2man.1

#
# Hack the directory making for now.
#
bin: $(OBJ_DIR) $(MAKE_DIR)bin/$(MACHINE)/c2man

$(MAKE_DIR)bin/$(MACHINE)/c2man: $(OBJ_DIR)c2man
	$(KJB_MKDIR) $(MAKE_DIR)bin/$(MACHINE)
	$(KJB_CP) $< $@

install: all
	$(CP) $(OBJ_DIR)c2man $(bin)
	$(CP) c2man.1 $(mansrc)/c2man.$(manext)
	-mkdir $(privlib)
	chmod 755 $(privlib)
	-mkdir $(privlib)/eg
	chmod 755 $(privlib)/eg
	$(CP) eg/*.[chly] $(privlib)/eg
	chmod 755 $(privlib)/eg/*

uninstall:
	$(RM) -f $(bin)/c2man $(mansrc)/c2man.$(manext)
	$(RM) -f $(privlib)/eg/*
	rmdir $(privlib)/eg
	rmdir $(privlib)

$(OBJ_DIR)c2man: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(OBJ_DIR):
	$(KJB_MKDIR) $(OBJ_DIR)

c2man.1: c2man.man $(OBJ_DIR)c2man example.inc ctype_ex.inc flatten.sed
	$(SED) -f flatten.sed < c2man.man > c2man.1

$(OBJ_DIR)%.o: %.c
	$(CC) $(CFLAGS) $(WARNFLAGS) -c $< -o $@

y.tab.c: grammar.y
	@$(ECHO) Expect 54 shift/reduce conflicts.
	$(YACC) grammar.y

# don't compile y.tab.c with all warnings; yacc/bison are not up to it.
$(OBJ_DIR)y.tab.o: y.tab.c lex.yy.c
	$(CC) $(CFLAGS) -c y.tab.c -o $@

lex.yy.c: lex.l
	$(LEX) -n lex.l

example.inc: $(OBJ_DIR)c2man example.h fixexamp.sed
	$(OBJ_DIR)c2man -o- example.h | $(SED) -f fixexamp.sed >example.inc

ctype_ex.inc: $(OBJ_DIR)c2man ctype_ex.h fixexamp.sed
	$(OBJ_DIR)c2man -o- -g ctype_ex.h | $(SED) -f fixexamp.sed >ctype_ex.inc

fixexamp.sed: fixexamp.in
	$(SED) -e '/^#/d' fixexamp.in > fixexamp.sed

flatten.sed: flatten.SH config.sh
	sh flatten.SH

Makefile: Makefile.SH config.sh
	sh Makefile.SH

config.h: config_h.SH config.sh
	sh config_h.SH

TAGS: $(SOURCES)
	etags -t $(SOURCES)

clean:
	$(RM) -f $(OBJECTS) *.s *.bak *~ *.log $(GENERATED) core

distclean realclean: clean
	$(RM) -f Makefile config.sh mkdep c2man.kit?

lint:
	lint -b $(CFLAGS) $(CSOURCES)

print:
	cpr $(SOURCES) | lpr -J'c2man'

test: $(OBJ_DIR)c2man
	@echo "Running c2man over the examples..." 1>&2
	@for file in eg/*.[chly]; do $(OBJ_DIR)c2man -v -o- $$file; done
	@echo "Running c2man over its own source code..." 1>&2
	@for file in $(DCSOURCES); do $(OBJ_DIR)c2man -v -o- $$file; done
	@echo "Hmmm, test seemed to go OK." 1>&2

depend:
	$(SED) -e '1,/^# DO NOT/!d' < Makefile > Makefile.new
	$(MKDEP) $(DCSOURCES) | $(SED) 's/^\(.*\.o:\)/$$(OBJ_DIR)\1/' >> Makefile.new
	- test ! -f y.tab.c -o ! -f lex.yy.c || $(MKDEP) y.tab.c  | $(SED) 's/^\(.*\.o:\)/$$(OBJ_DIR)\1/' >> Makefile.new
#	$(SED) < Makefile.new 's/\(y\.\)*\([a-z][a-z2]*\.\<o\>\)/$$(OBJ_DIR)\1\2/g' > Makefile
	$(MV) -f Makefile.new Makefile

# y.tab.c dependancies updated manually since it won't exist yet when make
# depend is first run.
$(OBJ_DIR)y.tab.o: c2man.h
$(OBJ_DIR)y.tab.o: config.h
$(OBJ_DIR)y.tab.o: confmagic.h
$(OBJ_DIR)y.tab.o: enum.h
$(OBJ_DIR)y.tab.o: manpage.h
$(OBJ_DIR)y.tab.o: semantic.h
$(OBJ_DIR)y.tab.o: strappend.h
$(OBJ_DIR)y.tab.o: strconcat.h
$(OBJ_DIR)y.tab.o: symbol.h

# DO NOT DELETE THIS LINE! make depend DEPENDS ON IT!
c2man.o:
	@echo "You must run a make depend first."; exit 1
!NO!SUBS!
chmod 755 Makefile
$eunicefix Makefile
