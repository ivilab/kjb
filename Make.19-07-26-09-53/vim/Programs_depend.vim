:
:set viminfo=
:set ul=0
:set ttyfast
:
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<c\>/\1$(OBJ_DIR)\2.o : $(OBJ_DIR)c_dir_made
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<C\>/\1$(CXX_OBJ_DIR)\2.o : $(CXX_OBJ_DIR)cxx_dir_made
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<cxx\>/\1$(CXX_OBJ_DIR)\2.o : $(CXX_OBJ_DIR)cxx_dir_made
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<cpp\>/\1$(CXX_OBJ_DIR)\2.o : $(CXX_OBJ_DIR)cxx_dir_made
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<cc\>/\1$(CXX_OBJ_DIR)\2.o : $(CXX_OBJ_DIR)cxx_dir_made

:%s/^\([A-Za-z0-9_\-\.]*\)\.\<c\>/$(OBJ_DIR)\1.o : $(OBJ_DIR)c_dir_made
:%s/^\([A-Za-z0-9_\-\.]*\)\.\<C\>/$(CXX_OBJ_DIR)\1.o : $(CXX_OBJ_DIR)cxx_dir_made
:%s/^\([A-Za-z0-9_\-\.]*\)\.\<cxx\>/$(CXX_OBJ_DIR)\1.o : $(CXX_OBJ_DIR)cxx_dir_made
:%s/^\([A-Za-z0-9_\-\.]*\)\.\<cpp\>/$(CXX_OBJ_DIR)\1.o : $(CXX_OBJ_DIR)cxx_dir_made
:%s/^\([A-Za-z0-9_\-\.]*\)\.\<cc\>/$(CXX_OBJ_DIR)\1.o : $(CXX_OBJ_DIR)cxx_dir_made
:
:wq