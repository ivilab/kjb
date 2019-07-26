:
:set viminfo=
:set ul=0
:set ttyfast
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<c\>/$(USER_BIN_DIR)\1/e
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<C\>/$(USER_BIN_DIR)\1/e
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<cxx\>/$(USER_BIN_DIR)\1/e
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<cpp\>/$(USER_BIN_DIR)\1/e
:
:" Get rid of suffix and add in MACHINE "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<cc\>/$(USER_BIN_DIR)\1/e
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
:1s/.*/USER_PROGRAMS = \\&/e
:
:wq
