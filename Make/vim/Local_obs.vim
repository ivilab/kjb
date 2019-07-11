:set viminfo=
:set ul=0
:set ttyfast
:s/^..*$/OBS_ARCHIVE = $(LD_OBJ_DIR)LOCAL.a/e
:v/OBS_ARCHIVE/d
:wq
