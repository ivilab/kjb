:
:set viminfo=
:set ul=0
:set ttyfast
:
:" Make *.c ==> *.o "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<c\> *$/    $(OBJ_DIR)\1.o \\/ge
:" Make *.C ==> *.o "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<C\> *$/    $(CXX_OBJ_DIR)\1.o \\/ge
:" Make *.cpp ==> *.o "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<cpp\> *$/    $(CXX_OBJ_DIR)\1.o \\/ge
:" Make *.cxx ==> *.o "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<cxx\> *$/    $(CXX_OBJ_DIR)\1.o \\/ge
:" Make *.cc ==> *.o "
:%s/\([A-Za-z0-9_\-\.]*\)\.\<cc\> *$/    $(CXX_OBJ_DIR)\1.o \\/ge
:
:" Trim last backslash. " 
:$s/ *\\ *//ge
:
:1s/.*/VERSION_OBS = \\&/
:
:wq
