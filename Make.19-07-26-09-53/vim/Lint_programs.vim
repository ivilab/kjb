:
:set viminfo=
:set ul=0
:set ttyfast
:
:" Make *.c ==> *.h "
:%s/\.\<cpp\>/.ln/ge
:%s/\.\<cc\>/.ln/ge
:%s/\.\<cxx\>/.ln/ge
:%s/\.\<c\>/.ln/ge
:
:
:" Shift. "
:%s/^/        /
:
:
:"  Add backslashes "
:%s/\([^ ][^ ]*\) */\1 \\/
:
:
:" Get rid of junk "
:%s/ *$//ge
:
:
:" Add in OBJ_DIR "
:%s/[A-Za-z0-9_\-\.]*\.ln/$(OBJ_DIR)&/e
:
:" Trim all blanks (should be done, but just in case). "
:%s/ *$//ge
:
:
:1s/.*/LINT_OBS = \\&/e
:
:
:wq
