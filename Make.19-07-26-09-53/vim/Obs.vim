:
:set viminfo=
:set ul=0
:set ttyfast
:
:" How it was.
:"%s/\(.*\)\.[ocC][px]* *$/    \1.o \\/ge
:
:" Inspired by the .o in how it was before, but I am not sure why we need it.
:" Make *.o ==> *.o "
:"%s/\([A-Za-z0-9_\-]*\)\.o *$/    \1.o \\/ge
:
:" Starting here. 
:
:" Make *.c ==> *.$(OBJ_DIR) "
:%s/\([A-Za-z0-9_\-\.]*\)\.c *$/$(OBJ_DIR)\1.o \\/ge
:
:" Make *.C ==> *.$(CXX_OBJ_DIR) "
:%s/\([A-Za-z0-9_\-\.]*\)\.C *$/$(CXX_OBJ_DIR)\1.o \\/ge
:
:" Make *.cxx ==> *.$(CXX_OBJ_DIR) "
:%s/\([A-Za-z0-9_\-\.]*\)\.cxx *$/$(CXX_OBJ_DIR)\1.o \\/ge
:
:" Make *.cpp ==> *.$(CXX_OBJ_DIR) "
:%s/\([A-Za-z0-9_\-\.]*\)\.cpp *$/$(CXX_OBJ_DIR)\1.o \\/ge
:
:" Make *.cc ==> *.$(CXX_OBJ_DIR) "
:%s/\([A-Za-z0-9_\-\.]*\)\.cc *$/$(CXX_OBJ_DIR)\1.o \\/ge
:
:" Trim last backslash and add two blank lines." 
:$s/ *\\ *//e
:
:" Shift it over.
:%s/^/    /ge
:
:1s/.*/OBS = \\&/
:
:wq
