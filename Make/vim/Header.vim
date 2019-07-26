:
:
:set viminfo=
:set ul=0
:set ttyfast
:
:
:%s/_incl\.o/_incl.h/ge
:
:" A hair risky to use gen, as there is always the possibility that a source
:" ends in "gen.c". However, if we want this file to apply universily ... 
:%s/gen\.o *:/gen.h:/ge
:
:
:" Can't have <letter>/<leter>_gen.h depend on itself.
:"
:%s/^\([a-z]\)\/\1_gen\.h\( *:.*\) \1\/\1_gen\.h/\1\/\1_gen.h\2/ge
:%s/^\([a-z]\)\/\1_gen\.h\( *:.*\) .*\/\1\/\1_gen\.h/\1\/\1_gen.h\2/ge
:%s/^.*\/\([a-z]\)\/\1_gen\.h\( *:.*\) \1\/\1_gen\.h/\1\/\1_gen.h\2/ge
:%s/^.*\/\([a-z]\)\/\1_gen\.h\( *:.*\) .*\/\1\/\1_gen\.h/\1\/\1_gen.h\2/ge
:
:
:" Can't have gen.h depend on itself.
:"
:%s/^\(.*\)\/gen\.h\( *:.*\) [^ ]\+\/gen\.h/\1\/gen.h\2/ge
:%s/^\(.*\)\/gen\.h\( *:.*\) gen\.h/\1\/gen.h\2/ge
:%s/^gen\.h\( *:.*\) [^ ]\+\/gen.h/gen.h\1/ge
:%s/^gen\.h\( *:.*\) gen\.h/gen.h\1/ge
:
:
:wq
