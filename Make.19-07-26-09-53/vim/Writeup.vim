:
:set viminfo=
:set ul=0
:set ttyfast
:
:"Add a blank line, so if there are no files, we do not get an error status 
:"due to 0 lines after we delete the fake_out_glob line.
:1s/.*/&/e
:"
:" The code below assumes that the line is here, and that it is replaced by a
:" blank line."
:g/fake_out_glob/d
:
:" Change *.w ==> *.w.made, which forms input to produce *.made
:%s/\.\<w\>/&.made/ge
:
:" Shift. "
:%s/^/        /e
:
:
:" Add backslashes "
:%s/\([^ ][^ ]*\) */\1 \\/e
:
:
:" Get rid of comments from makedepend "
:%s/^#.*//ge
:
:
:" Add in MAKE_DOC_DIR"
:%s/[A-Za-z0-9_\-\.]/$(MAKE_DOC_DIR)&/e
:
:" Trim all blanks (should be done, but just in case). "
:%s/ *$//ge
:
:
:" Get rid of slash on last item, and add two blank lines. 
:$s/ *\\ *//e
:"
:1s/.*/MAN_FROM_WRITEUP_FILES = \\/e
:
:"Add a blank line at the end.
:$s/.*/&/e
:
:wq
