
TARGETS = \
    Add_incl_comment.ex \
    Add_incl_line.ex \
    Depend.ex \
    Dirs.ex \
    Flt.ex \
    Header.ex \
    Help.ex \
    Lint_depend.ex \
    Lint_obs.ex \
    Lint_programs.ex \
    Misc.ex \
    Obs.ex \
    Obs_depend.ex \
    Programs.ex \
    Programs_depend.ex \
    Tags.ex \
    User.ex \
    Work.ex \
    Writeup.ex \
    Version_obs.ex

TARGETS_VIM_7 = \
    Add_incl_comment.ex7 \
    Add_incl_line.ex7 \
    Depend.ex7 \
    Dirs.ex7 \
    Flt.ex7 \
    Header.ex7 \
    Help.ex7 \
    Lint_depend.ex7 \
    Lint_obs.ex7 \
    Lint_programs.ex7 \
    Misc.ex7 \
    Obs.ex7 \
    Obs_depend.ex7 \
    Programs.ex7 \
    Programs_depend.ex7 \
    Tags.ex7 \
    User.ex7 \
    Writeup.ex7 \
    Work.ex7 \
    Version_obs.ex7

all : $(TARGETS) $(TARGETS_VIM_7)

%.ex : %.vim
	./strip_ex_files $< $@

%.ex7 : %.vim
	./strip_ex7_files $< $@


