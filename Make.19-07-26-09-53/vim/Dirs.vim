:""
:set viminfo=
:set ul=0
:set ttyfast
:""
:""
:" Remove all comment lines"
:"g/^#/d"
:"
:"
:" Make it so that all lines have a blank at the end so we don't have to match 
:" the end of lines. "
:%s/ *$/ /e
:"
:" Make it so that all colons are surrounded by blanks to also simplify matching
:%s/ *: */ : /e
:""
:
:" Hyphens in lib dirs need to become double underscores in the environment symbol. 
:%s/-/-xxx-hyphen-yyy-/ge
:
:" Do <junk>/lib/<dir>/<other_stuff>.<suffix>
:%s/ [^ ]*\/lib\/\([A-Za-z0-9_\-]*\)\/\([a-z_A-Z0-9\-\.]*\.\<[Ccohit]p*x*c*\>\)/ $(\U\1\e_DIR)\2/ge
:%s/^[^ ]*\/lib\/\([A-Za-z0-9_\-]*\)\/\([a-z_A-Z0-9\-\.]*\.\<[Ccohit]p*x*c*\>\)/$(\U\1\e_DIR)\2/ge
:
:" Do lib/<dir>/<other_stuff>.<suffix>
:%s/ lib\/\([A-Za-z0-9_\-]*\)\/\([a-z_A-Z0-9\-\.]*\.\<[Ccohit]p*x*c*\>\)/ $(\U\1\e_DIR)\2/ge
:%s/^lib\/\([A-Za-z0-9_\-]*\)\/\([a-z_A-Z0-9\-\.]*\.\<[Ccohit]p*x*c*\>\)/$(\U\1\e_DIR)\2/ge
:
:
:" Do <dir>/<other_stuff>.<suffix> (Note that allowing the periond in the <dir> was already 
:" there when we added allowing them in file names. 
:%s/ \([A-Za-z0-9_\-]*\)\/\([a-z_A-Z0-9\-\.]*\.\<[Ccohit]p*x*c*\>\)/ $(\U\1\e_DIR)\2/ge
:%s/^\([A-Za-z0-9_\-]*\)\/\([a-z_A-Z0-9\-\.]*\.\<[Ccohit]p*x*c*\>\)/$(\U\1\e_DIR)\2/ge
:
:" We have reduced our reliance on this file, so we can be a bit more agressive to deal with the 
:" case that we are building libKJB.a from the KJB library directory (i.e., lib/), but only need
:" to do object lists, and thus we have one item per line as ../<stuff>/<other_stuff>.h"
:%s/ \.\.\/\([A-Za-z1-9_\-]*\)\/\([A-Za-z0-9_\-\.]*\.[Ccohit]p*x*c*\) *$/ $(\U\1\e_DIR)\2 /ge
:%s/^\.\.\/\([A-Za-z1-9_\-]*\)\/\([A-Za-z0-9_\-\.]*\.[Ccohit]p*x*c*\) *$/$(\U\1\e_DIR)\2 /ge
:
:" Kill remaining full paths"
:%s/ \/[^ ]*//ge
:
:" Fix hyphens. The translated hypens become triple underscores. 
:%s/-xxx-hyphen-yyy-/-/ge
:%s/-XXX-HYPHEN-YYY-/___/ge
:
:" Now trim all blanks"
:%s/ *$//ge
:""
:wq
