#
# This Makefile is almost invariably deals with only one target so we don't have
# to worry about ensuring that it builds serially. Note that we control the
# number of make threads in the build script based on the number of cpu's. 
#
# The strategy here to execute the script "build" which sets up a sub-shell with
# the appropriate environment, and then do a make with Makefile-2. We list
# possible targets here, but for most versions of make, a line below should
# be able to catch all cases. 
#
# None of the explicilty mentioned targets that get sent to the build
# script are meant to be actual files or directories. To protect against a name
# clash, we declare the targets handled here as .PHONY.
#

.PHONY : all
.PHONY : dir_made
.PHONY : init_clean
.PHONY : confess
.PHONY : depend_very_clean
.PHONY : depend_clean
.PHONY : obj_clean
.PHONY : clean
.PHONY : code
.PHONY : depend
.PHONY : depend_again
.PHONY : doc
.PHONY : doc_dir_made
.PHONY : lint
.PHONY : proto
.PHONY : bin
.PHONY : work
.PHONY : misc
.PHONY : test
.PHONY : test_clean
.PHONY : test_very_clean
.PHONY : check_clean
.PHONY : regress
.PHONY : regress_clean
.PHONY : test_svn
.PHONY : shared
.PHONY : dynamic
.PHONY : static

# Some targets, such as confess are implemented in build-2. 
#
# Each target in the following lists needs to be declared .PHONY above. So, if
# you add one, you need to add a .PHONY line above. 
#
all dir_made init_clean confess depend_very_clean depend_clean obj_clean clean code depend depend_again doc doc_dir_made lint proto bin work misc :
	$(ECHO_MAKE_CMD)./build $@

test test_clean test_very_clean check_clean regress regress_clean test_svn :
	$(ECHO_MAKE_CMD)./build $@

static dynamic shared :
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
# works fine with gmake. The only small risk is that the target should really be
# declared .PHONY, which itself protects against the possiblity that we have a
# directory with the same name as the target. 
#
% : FORCE
	$(ECHO_MAKE_CMD)./build $@


FORCE :
	

