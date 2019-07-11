:
:set viminfo=
:set ul=0
:set ttyfast
:
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<c\>/$(MISC_BIN_DIR)\1/e
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<C\>/$(MISC_BIN_DIR)\1/e
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<cxx\>/$(MISC_BIN_DIR)\1/e
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<cpp\>/$(MISC_BIN_DIR)\1/e
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<cc\>/$(MISC_BIN_DIR)\1/e
:
:
:" Shift. "
:%s/^/        /e
:
:" Add backslashes "
:%s/\([^ ][^ ]*\) */\1 \\/e
:
:" Get rid of junk "
:%s/ *$//ge
:
:" Trim all blanks (should be done, but just in case). "
:%s/ *$//ge
:
:" Get rid of slash on last item, and add two blank lines. 
:$s/ *\\ *//e
:"
:1s/.*/MISC_PROGRAMS = \\&/e
:
:wq
