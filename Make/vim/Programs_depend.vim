:
:set viminfo=
:set ul=0
:set ttyfast
:
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<c\>/\1$(OBJ_DIR)\2.o : $(OBJ_DIR)c_dir_made\1$(LD_OBJ_DIR)\2.o : $(LD_OBJ_DIR)ld_dir_made \1$(OBJ_DIR)\2.o	$(KJB_CP) \1$(OBJ_DIR)\2.o $@/e
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<C\>/\1$(CXX_OBJ_DIR)\2.o : $(CXX_OBJ_DIR)cxx_dir_made\1$(LD_OBJ_DIR)\2.o : $(LD_OBJ_DIR)ld_dir_made \1$(CXX_OBJ_DIR)\2.o	$(KJB_CP) \1$(CXX_OBJ_DIR)\2.o $@/e
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<cxx\>/\1$(CXX_OBJ_DIR)\2.o : $(CXX_OBJ_DIR)cxx_dir_made\1$(LD_OBJ_DIR)\2.o : $(LD_OBJ_DIR)ld_dir_made \1$(CXX_OBJ_DIR)\2.o	$(KJB_CP) \1$(CXX_OBJ_DIR)\2.o $@/e
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<cpp\>/\1$(CXX_OBJ_DIR)\2.o : $(CXX_OBJ_DIR)cxx_dir_made\1$(LD_OBJ_DIR)\2.o : $(LD_OBJ_DIR)ld_dir_made \1$(CXX_OBJ_DIR)\2.o	$(KJB_CP) \1$(CXX_OBJ_DIR)\2.o $@/e
:%s/^\([^ \/]*\/\)\([A-Za-z0-9_\-\.]*\)\.\<cc\>/\1$(CXX_OBJ_DIR)\2.o : $(CXX_OBJ_DIR)cxx_dir_made\1$(LD_OBJ_DIR)\2.o : $(LD_OBJ_DIR)ld_dir_made \1$(CXX_OBJ_DIR)\2.o	$(KJB_CP) \1$(CXX_OBJ_DIR)\2.o $@/e

:%s/^\([A-Za-z0-9_\-\.]*\)\.\<c\>/$(OBJ_DIR)\1.o : $(OBJ_DIR)c_dir_made$(LD_OBJ_DIR)\1.o : $(LD_OBJ_DIR)ld_dir_made $(OBJ_DIR)\1.o	$(KJB_CP) $(OBJ_DIR)\1.o $@/e
:%s/^\([A-Za-z0-9_\-\.]*\)\.\<C\>/$(CXX_OBJ_DIR)\1.o : $(CXX_OBJ_DIR)cxx_dir_made$(LD_OBJ_DIR)\1.o : $(LD_OBJ_DIR)ld_dir_made $(CXX_OBJ_DIR)\1.o	$(KJB_CP) $(CXX_OBJ_DIR)\1.o $@/e
:%s/^\([A-Za-z0-9_\-\.]*\)\.\<cxx\>/$(CXX_OBJ_DIR)\1.o : $(CXX_OBJ_DIR)cxx_dir_made$(LD_OBJ_DIR)\1.o : $(LD_OBJ_DIR)ld_dir_made $(CXX_OBJ_DIR)\1.o	$(KJB_CP) $(CXX_OBJ_DIR)\1.o $@/e
:%s/^\([A-Za-z0-9_\-\.]*\)\.\<cpp\>/$(CXX_OBJ_DIR)\1.o : $(CXX_OBJ_DIR)cxx_dir_made$(LD_OBJ_DIR)\1.o : $(LD_OBJ_DIR)ld_dir_made $(CXX_OBJ_DIR)\1.o	$(KJB_CP) $(CXX_OBJ_DIR)\1.o $@/e
:%s/^\([A-Za-z0-9_\-\.]*\)\.\<cc\>/$(CXX_OBJ_DIR)\1.o : $(CXX_OBJ_DIR)cxx_dir_made$(LD_OBJ_DIR)\1.o : $(LD_OBJ_DIR)ld_dir_made $(CXX_OBJ_DIR)\1.o	$(KJB_CP) $(CXX_OBJ_DIR)\1.o $@/e
:
:wq
