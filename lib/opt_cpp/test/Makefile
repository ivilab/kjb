#
# The strategy here to execute the script "build" which sets up a sub-shell with
# the appropriate environment, and then do a make with Makefile-2. We list
# possible targets here, but for most versions of make, a line below should
# be able to catch all cases. 
#
# IMPORTANT: None of the explicilty mentioned targets on the next non-comment
# line can be actual files or directories. For example, we cannot be attempting
# to make the directory "doc". Since the purpose of this file is to call the
# build script, the dependencies for "doc" will be hidden. If you suspect that
# this is the case for a target XXX, try "./build XXX". 

# This Makefile is almost invariably deals with only one target so we don't have
# to worry about ensuring that it builds serially. Note that we control the
# number of make threads in the build script based on the number of cpu's. 

# Some targets, such as confess are implemented in build-2. 
#
all dir_made init_clean confess depend_very_clean depend_clean obj_clean clean code depend depend_again doc doc_dir_made lint proto regress_clean bin work misc: 
	$(ECHO_MAKE_CMD)./build $@

# We cannot have the real dependency for Makefile because we do not know where
# we are in the source tree yet. Further, we cannot force the build because some
# versions of make put Makefiles being processed on the dependency list.  Thus
# whatever the build was, make would first try to make "Makefile.".  However,
# not having Makefile depend on something has the confusing effect that a "make
# Makefile" will report Makefile is up to date, even if it is not. 
#
Makefile : 
	@$(KJB_ECHO) "Dummy rule for Makefile." 


build : FORCE
	$(ECHO_MAKE_CMD)./build $@



#
# Need to specify anything that might have an implict rule, in case we forget to
# use the "-r" option. 
#
%.o : FORCE
	$(ECHO_MAKE_CMD)./build $@

%.h : FORCE
	$(ECHO_MAKE_CMD)./build $@

%.c : FORCE
	$(ECHO_MAKE_CMD)./build $@

%.cpp : FORCE
	$(ECHO_MAKE_CMD)./build $@

%.cxx : FORCE
	$(ECHO_MAKE_CMD)./build $@

%.C : FORCE
	$(ECHO_MAKE_CMD)./build $@

%.w : FORCE
	$(ECHO_MAKE_CMD)./build $@


.INIT : 
	@$(KJB_ECHO) "Starting in Makefile." 


.DONE :
	@$(KJB_ECHO) "Done in Makefile." 


#
# The catch all line. Any target not handled above should be handled here. This
# works fine with gmake. 
#
% : FORCE
	$(ECHO_MAKE_CMD)./build $@


FORCE :
	

