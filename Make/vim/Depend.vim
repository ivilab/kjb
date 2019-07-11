:""
:set viminfo=
:set ul=0
:set ttyfast
:""
:
:"How it was
:" Add OBJ_DIR "
:"%s/\<[A-Za-z0-9._-]*\.o\>/$(OBJ_DIR)&/e
:
:
:" Get rid of comments from makedepend "
:%s/^#.*//ge
:" Don't remove blank lines because we could empty the buffer which leads
:" to error exit in Vim 7! 
:"g/^ *$/d
:
:" Add symbolic pieces to header files that will translate them to "
:" precompiled headers whe the symbols are defined to do so. 
:" We handle the case that the header file is preceded by a blank 
:" as arranged by Dirs.vim, or a canonocal subdir symbol. 
:%s/\.h$/.h /e
:%s/\$(\([A-Z][A-Z0-9_]*\)_DIR)\([a-zA-Z0-9][a-zA-Z0-9_]*\)\.h/$(PCH_\1_DIR)\2.h$(PCH_SUFFIX)/ge
:%s/ \([a-zA-Z0-9][a-zA-Z0-9_]*\)\.h/ $(PCH_DIR)\1.h$(PCH_SUFFIX)/ge
:%s/\.h$(PCH_SUFFIX)  *$/.h$(PCH_SUFFIX)/e
:
:" Format nicely "
:%s/ \([^ :]\)/ \\            \1/ge
:%s/^\([^ ]\)/\1/ge
:
:
:" Remove null dependcies
:s/^.*: *$//ge
:
:
:" Convert strings in targets like /lib/c/ to $(C_DIR)/
:" First for target strings (.h then .o), then for dependency strings.
:" Target strings should start in the first column, and dependency strings have spaces.  
:"    Target
:%s/^.*\/lib\/\([A-Za-z0-9_\-]*\)\/\([A-Za-z0-9_\-\.]*.h:\)/$(\U\1\e_DIR)\2/e
:%s/^lib\/\([A-Za-z0-9_\-]*\)\/\([A-Za-z0-9_\-\.]*.h:\)/$(\U\1\e_DIR)\2/e
:
:%s/^.*\/lib\/\([A-Za-z0-9_\-]*\)\/\(\$([A-Z_]*OBJ_DIR)[A-Za-z0-9_\-\.]*\.o:\)/$(\U\1\e_DIR)\2/e
:%s/^lib\/\([A-Za-z0-9_\-]*\)\/\(\$([A-Z_]*OBJ_DIR)[A-Za-z0-9_\-\.]*\.o:\)/$(\U\1\e_DIR)\2/e
:"    Dependencies 
:%s/ [^ ]*\/lib\/\([A-Za-z0-9_\-]*\)\/\([A-Za-z0-9_\-\.]*\.[hit]p*\)/$(\U\1\e_DIR)\2/e
:%s/ lib\/\([A-Za-z0-9_\-]*\)\/\([A-Za-z0-9_\-\.]*\.[hit]p*\)/$(\U\1\e_DIR)\2/e
:
:1s/.*/&/e
:
:wq
