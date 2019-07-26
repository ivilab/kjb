:
:set viminfo=
:set ul=0
:set ttyfast
:
:" NOT sure if we want cpp and c files for lint? "
:" Make *.c ==> *.o "
:" Make *.cpp ==> *.o "
:%s/\(.*\)\.cp* *$/    \1.ln \\/ge
:
:" Trim last backslash. " 
:$s/ *\\ *//ge
:
:" Add in OBJ_DIR"
:%s/[A-Za-z0-9._-\.]*\.ln/$(OBJ_DIR)&/e
:
:1s/.*/LINT_OBS = \\&/
:
:wq
