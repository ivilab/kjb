#
# The strategy here to execute the script "build" which sets up a sub-shell with
# the appropriate environment, and then do a make with Makefile-2.
#

all depend doc lint clean proto : 
	./build $@

#
# Need to specify anything that might have an implict rule, in case we forget to
# use the "-r" option. 
#
%.o : FORCE
	./build $@

%.h : FORCE
	./build $@


Makefile : 
	$(KJB_MAKE_NO_OP)


.INIT : 
	$(KJB_MAKE_NO_OP)


.DONE :
	$(KJB_MAKE_NO_OP)


% : FORCE
	./build $@


FORCE :
	

