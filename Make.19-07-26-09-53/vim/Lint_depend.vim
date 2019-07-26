:
:" This file is very similar to Depend.vim, except with .ln instead of .o. 
:" However, we keep this separate for future considerations. "
:
:set viminfo=
:set ul=0
:set ttyfast
:
:%s/\.o/.ln/ge
:
:
:" Add OBJ_DIR "
:%s/\<[A-Za-z0-9._-]*\.ln\>/$(OBJ_DIR)&/e
:
:
:" Get rid of comments from makedepend "
:%s/^#.*//ge
:" Don't remove blank lines because we could empty the buffer which leads "
:" to error exit in Vim 7! 
:"g/^ *$/d
:
:
:" Format nicely "
:%s/ \([^ :]\)/ \\            \1/ge
:%s/^\([^ ]\)/\1/ge
:
:
:" Remove null dependcies"
:s/^.*: *$//ge
:
::" Convert strings in targets like /lib/c/ to $(C_DIR)/
:" First for target strings, then for dependency strings.
:" Target strings should start in the first column, and dependency strings have spaces.  
:"    Target
:%s/^.*\/lib\/\([A-Za-z0-9_\-\.]*\)\/\(\$(OBJ_DIR)[A-Za-z0-9_\-]*.ln\)/$(\U\1\e_DIR)\2/e
:%s/^lib\/\([A-Za-z0-9_\-\.]*\)\/\(\$(OBJ_DIR)[A-Za-z0-9_\-]*.ln\)/$(\U\1\e_DIR)\2/e
:"    Dependencies 
:%s/ [^ ]*\/lib\/\([A-Za-z0-9_\-\.]*\)\/\([A-Za-z0-9_\-]*\.[hit]p*\)/$(\U\1\e_DIR)\2/e
:%s/ lib\/\([A-Za-z0-9_\-\.]*\)\/\([A-Za-z0-9_\-]*\.[hit]p*\)/$(\U\1\e_DIR)\2/e
:
:1s/.*/&/e
:
:wq
